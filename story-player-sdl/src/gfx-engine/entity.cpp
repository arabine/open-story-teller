#include "entity.h"
#include "xlog.h"

using namespace gfx;

Entity::Entity(Display &s)
    : mSystem(s)
{
    mRect.x = 0;
    mRect.y = 0;
    mRect.w = 0;
    mRect.h = 0;
}

Entity::~Entity()
{
    mTexture = nullptr;
}

void Entity::OnCreate(SDL_Renderer *renderer)
{
    (void) renderer;
    mCreated = true;
}

void Entity::ProcessEvent(const SDL_Event &event, const Vector2 &origin)
{
    (void) event;
}

void Entity::Update(double deltaTime) { (void) deltaTime; }

void Entity::Draw(SDL_Renderer *renderer, int x_offset, int y_offset)
{
    if (!mCreated)
    {
        OnCreate(renderer);
    }

    if (IsVisible() && (mTexture != nullptr))
    {
        SDL_Rect r = GetRect();
        r.w *= GetScale().x;
        r.h *= GetScale().y;
        r.x = GetX() + x_offset;
        r.y = GetY() + y_offset;
        Render(renderer, r);
    }
}

void Entity::SetTexture(SDL_Texture *texture)
{
    mTexture = texture;
    int w = 0;
    int h = 0;
    // get the width and height of the texture
    if (SDL_QueryTexture(mTexture, NULL, NULL, &w, &h) == 0)
    {
        SetSize(w, h);
    }
}

void Entity::Render(SDL_Renderer *renderer, const SDL_Rect &pos)
{   
    SDL_SetTextureBlendMode(mTexture, mBlend);
    SDL_SetTextureColorMod(mTexture, mColorMod.r, mColorMod.g, mColorMod.b);
    SDL_RenderCopyEx(renderer, mTexture, NULL, &pos, GetAngle(), NULL, SDL_FLIP_NONE);
}
