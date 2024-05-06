#pragma once

#include <functional>

#include "display.h"
#include "entity.h"

namespace gfx
{

class Image : public Entity
{

public:
    Image(Display &system, const std::string &path, bool isSvg);

    virtual ~Image();

    virtual void OnClick();
    virtual std::string UpdateSvg(const std::string &svg) { return svg; } // FIXME: avoid string duplication in memory

    bool IsCursorOver(const SDL_Point &pos, const Vector2 &origin) const;

    std::string_view FileName() const
    {
        return std::string_view(mFileName);
    }

    void SetSvgScale(float scale) { mSvgScale = scale; }

    virtual void ProcessEvent(const SDL_Event &event, const Vector2 &origin) override;

    virtual void OnCreate(SDL_Renderer *renderer) override;

    void SetHighlight(bool enable) { mHighlight = enable; }
    bool IsHighlighted() const { return mHighlight; }
    size_t font_data_size = 0;

    static std::string LoadFile(const char *filename);
    static SDL_Texture *LoadSVG(SDL_Renderer *renderer, const char *filename);
    static SDL_Texture *RenderSVG(SDL_Renderer *renderer, const std::string &svgData, float scale = 1.0);
    static SDL_Texture *LoadImage(SDL_Renderer *renderer, const char *filename);

    void SetActive(bool active);
    void HandleOnClick(std::function<void ()> callback);
private:
    std::string mFileName;
    std::string mSvg;
    bool mHighlight{false};
    bool mIsSvg{false};
    float mSvgScale{1.0};
    std::function<void(void)> mCallback;
    bool mIsActive = false;
    std::string TextureName() const;
};
