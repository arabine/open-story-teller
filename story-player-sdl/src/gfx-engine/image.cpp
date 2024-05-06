#include "image.h"
#include "xlog.h"
#include <SDL3_image/SDL_image.h>

using namespace gfx;


// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, Image &img)
{

    SDL_Surface *surface, *temp;

    surface = IMG_Load(filename);
    if (!surface) {
        SDL_Log("Couldn't load %s: %s\n", filename, SDL_GetError());
        return false;
    }

    /* Use the tonemap operator to convert to SDR output */
    const char *tonemap = NULL;
    SDL_SetStringProperty(SDL_GetSurfaceProperties(surface), SDL_PROP_SURFACE_TONEMAP_OPERATOR_STRING, tonemap);
    temp = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (!temp) {
        SDL_Log("Couldn't convert surface: %s\n", SDL_GetError());
        return false;
    }

    img.texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_DestroySurface(temp);
    if (!img.texture) {
        SDL_Log("Couldn't create texture: %s\n", SDL_GetError());
       return false;
    }

    SDL_QueryTexture(static_cast<SDL_Texture*>(img.texture), NULL, NULL, &img.w, &img.h);
    
    return true;
}


Image::Image(Display &system, const std::string &path, bool isSvg)
    : Entity(system)
    , mFileName(path)
    , mIsSvg(isSvg)
{
}

Image::~Image()
{
    GfxEngine::DestroyTexture(TextureName());
}

std::string Image::TextureName() const
{
    // On ajoute le scale car plusieurs texture avec le même fichier peuvent être créées
    // mais avec des scales différents
    // ça ne change rien pour les images RAW (png, jpg...)
    return mFileName + std::to_string(mSvgScale);
}

void Image::SetActive(bool active)
{
    mIsActive = active;
}


void Image::HandleOnClick(std::function<void(void)> callback)
{
    mCallback = callback;
}

void Image::OnClick()
{
    if (mCallback && mIsActive)
    {
        mCallback();
    }
}

bool Image::IsCursorOver(const SDL_Point &pos, const Vector2 &origin) const
{
    SDL_Rect rect = GetRect();
    rect.x += origin.x;
    rect.y += origin.y;
    return SDL_PointInRect(&pos, &rect);
}


void Image::ProcessEvent(const SDL_Event &event, const Vector2 &origin)
{
    SDL_Point mousePos;
    mousePos.x = event.button.x;
    mousePos.y = event.button.y;

    bool isOver = IsCursorOver(mousePos, origin) && mIsActive;

    if (event.type == SDL_MOUSEBUTTONUP)
    {
        if (isOver)
        {
            OnClick();
        }
    }
    if (event.type == SDL_MOUSEMOTION)
    {
        if (isOver)
        {
            if (!mHighlight)
            {
                // Entered
                mHighlight = true;
                SetBlendMode(SDL_BLENDMODE_ADD);
                SetColorMod({250, 250, 250, 255});
            }
        }
        else
        {
            if (mHighlight)
            {
                // Exited
                mHighlight = false;
                SetBlendMode(SDL_BLENDMODE_BLEND);
                SetColorMod({255, 255, 255, 255});
            }
        }
    }

}

void Image::OnCreate(SDL_Renderer *renderer)
{
    Entity::OnCreate(renderer);

    SDL_Texture *tex = nullptr;

    if (GfxEngine::HasTexture(mFileName))
    {
        tex = GfxEngine::GetTexture(mFileName);
    }
    else
    {
        if (mIsSvg)
        {
            if (GfxEngine::LoadFile(mFileName.c_str(), mSvg))
            {
                std::string svg = UpdateSvg(mSvg);

                float scale = mSvgScale * GetSystem().Ratio();
                tex = Image::RenderSVG(renderer, svg.data(), scale);
            }
            else
            {
                LOG_ERROR("[IMAGE] Cannot load SVG file");
            }
        }
        else
        {
            tex = LoadImage(renderer, mFileName.c_str());
        }

        // Texture created, store it into our texture library
        GfxEngine::StoreTexture(TextureName(), tex);
    }

    SetTexture(tex);
}

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

/* Load a SVG type image from an SDL datasource */
SDL_Texture *Image::LoadSVG(SDL_Renderer *renderer, const char *filename)
{
    std::string data;
    SDL_Texture * tex = nullptr;

    if (GfxEngine::LoadFile(filename, data))
    {
        tex = Image::RenderSVG(renderer, data.data());
    }

    if (tex == nullptr)
    {
        // errror, default texture so that the pointer is valid
        tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 20, 20);
    }

    return tex;
}

SDL_Texture *Image::RenderSVG(SDL_Renderer *renderer, const std::string &svgData, float scale)
{
    std::string data = svgData; // on est obligé de créer une copie car nsvgParse modifie le code SVG
    struct NSVGimage *image;
    struct NSVGrasterizer *rasterizer;
    SDL_Surface *surface = NULL;

    /* For now just use default units of pixels at 96 DPI */
    image = nsvgParse(data.data(), "px", 96.0f);

    if ( !image ) {
        //IMG_SetError("Couldn't parse SVG image");
        return NULL;
    }

    rasterizer = nsvgCreateRasterizer();
    if ( !rasterizer ) {
       // IMG_SetError("Couldn't create SVG rasterizer");
        nsvgDelete( image );
        return NULL;
    }

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   (int)(image->width * scale),
                                   (int)(image->height * scale),
                                   32,
                                   0x000000FF,
                                   0x0000FF00,
                                   0x00FF0000,
                                   0xFF000000);
    if ( !surface ) {
        nsvgDeleteRasterizer( rasterizer );
        nsvgDelete( image );
        return NULL;
    }

    nsvgRasterize(rasterizer, image, 0.0f, 0.0f, scale, (unsigned char *)surface->pixels, surface->w, surface->h, surface->pitch);
    nsvgDeleteRasterizer( rasterizer );
    nsvgDelete( image );

    return SDL_CreateTextureFromSurface(renderer, surface);
}

SDL_Texture *Image::LoadImage(SDL_Renderer *renderer, const char* filename)
{
    // Read data
    int32_t width, height, bytesPerPixel;
    void* data = stbi_load(filename, &width, &height, &bytesPerPixel, 0);

    // Calculate pitch
    int pitch;
    pitch = width * bytesPerPixel;
    pitch = (pitch + 3) & ~3;

    // Setup relevance bitmask
    int32_t Rmask, Gmask, Bmask, Amask;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    Rmask = 0x000000FF;
    Gmask = 0x0000FF00;
    Bmask = 0x00FF0000;
    Amask = (bytesPerPixel == 4) ? 0xFF000000 : 0;
#else
    int s = (bytesPerPixel == 4) ? 0 : 8;
    Rmask = 0xFF000000 >> s;
    Gmask = 0x00FF0000 >> s;
    Bmask = 0x0000FF00 >> s;
    Amask = 0x000000FF >> s;
#endif
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data, width, height, bytesPerPixel*8, pitch, Rmask, Gmask, Bmask, Amask);
    SDL_Texture* t = nullptr;
    if (surface)
    {
        t = SDL_CreateTextureFromSurface(renderer, surface);
    }
    else
    {
        t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 20, 20);
    }

    STBI_FREE(data);
    SDL_FreeSurface(surface);
    return t;
}

