#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

class Display;

namespace gfx
{



struct Vector2f
{
    Vector2f()
    :x(0.0f), y(0.0f)
    {}

    Vector2f(float p_x, float p_y)
    :x(p_x), y(p_y)
    {}

    void print()
    {
        std::cout << x << ", " << y << std::endl;
    }

    float x, y;
};

struct Vector2
{
    Vector2()
        : x(0)
        , y(0)
    {}

    Vector2(int p_x, int p_y, bool e)
        : x(p_x)
        , y(p_y)
    {}

    void print()
    {
        std::cout << x << ", " << y << std::endl;
    }

    int x, y;
};


struct Rect : Vector2f
{
    int w;
    int h;

    Rect() {
        w = 0;
        h = 0;
    }
};


class Entity
{
public:
    Entity(Display &s);

    virtual ~Entity();

    virtual void OnCreate(SDL_Renderer *renderer);

    // Manage user interaction (mouse, keyboard...)
    virtual void ProcessEvent(const SDL_Event &event, const Vector2 &origin);

    // Update state
    virtual void Update(double deltaTime);

    // Draw to screen
    void Draw(SDL_Renderer *renderer, int x_offset = 0, int y_offset = 0);

    bool IsVisible() const { return mVisible; }
    void SetVisible(bool visible) { mVisible = visible; }

    void SetBlendMode(SDL_BlendMode newMode) { mBlend = newMode; }
    void SetColorMod(const SDL_Color &color) { mColorMod = color; }

    int GetX() const
    {
        return mRect.x;
    }

    int GetY() const
    {
        return mRect.y;
    }

    const SDL_Rect &GetRect() const
    {
        return mRect;
    }

    void SetSize(int w, int h)
    {
        mRect.w = w;
        mRect.h = h;
    }

    Vector2f &GetScale()
    {
        return mScale;
    }
    float GetAngle() const
    {
        return mAngle;
    }
    void SetPos(int x, int y)
    {
        mRect.x = x;
        mRect.y = y;
    }

    void SetScale(float x, float y)
    {
        mScale.x = x;
        mScale.y = y;
    }
    void SetAngle(float angle)
    {
        mAngle = angle;
    }

    void SetSceneIdOnwer(uint32_t sceneId) {
        mSceneIdOnwer = sceneId;
    }

    void SetId(uint32_t id) {
        mId = id;
    }

    uint32_t GetId() const {
        return mId;
    }

    void SetTexture(SDL_Texture *texture);

    void SetZ(uint32_t z) {
        mZ = z;
    }

    uint32_t GetZ() const { return mZ; }

    Display &GetSystem() { return mSystem; }

    void Rebuild() { mCreated = false; }

private:
    Display &mSystem; // keep it first please

    SDL_Texture *mTexture = nullptr;

    uint32_t mSceneIdOnwer = 0;
    uint32_t mId = 0;
    uint32_t mZ = 0; // pseudo Z value (order of drawing)

    bool mVisible{true};
    bool mCreated{false};
    SDL_Rect mRect;
    float mAngle = 0;
    Vector2f mScale = Vector2f(1, 1);

    SDL_BlendMode mBlend{SDL_BLENDMODE_BLEND};
    SDL_Color mColorMod{255, 255, 255, 255};

    void Render(SDL_Renderer *renderer, const SDL_Rect &pos);
};

} // namespace gfx

