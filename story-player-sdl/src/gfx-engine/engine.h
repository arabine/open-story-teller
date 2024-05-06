#ifndef GFX_ENGINE_H
#define GFX_ENGINE_H

// SDL
#include <SDL3/SDL.h>

// C++
#include <vector>
#include <iostream>
#include <list>
#include <deque>
#include <memory>
#include <map>
#include <algorithm>
#include <functional>
#include <variant>

// Gfx-Engine
#include "entity.h"
#include "scene.h"
#include "image.h"
#include "xlog.h"

namespace gfx 
{

typedef std::variant<std::string, double, std::int64_t, std::int32_t> Value;
/*
class Display
{
public:
    uint32_t GetScreenW();
    uint32_t GetScreenH();

    float GetRatioW() const { return mRatioW; }
    float GetRatioH() const { return mRatioH; }

    float Ratio() const { return mRatio; }

    uint32_t W(uint32_t w) const { return static_cast<uint32_t>(w * mRatioW); }
    uint32_t H(uint32_t h) const { return static_cast<uint32_t>(h * mRatioH); }

protected:
    SDL_Window *mWindow = nullptr;
    SDL_Renderer *mRenderer = nullptr;
    float mRatioW{1.0};
    float mRatioH{1.0};

    float mRatio{1.0};

    // Taille de base
    uint32_t mWidth = 648;
    uint32_t mHeight = 960;

    // Taille réelle
    uint32_t mScreenWidth{648};
    uint32_t mScreenHeight{960};
};

*/

class Engine : public Display
{
public:
    Engine()
    {
    }

    ~Engine()
    {
    }

    void Warmup();
    uint32_t Process();

 
    void PushBigFont() { ImGui::PushFont(mBigFont); }
    void PopBigFont() { ImGui::PopFont(); }

    void SetBackgroundColor(const SDL_Color &color)
    {
        mBackgroundColor = color;
    }

    void AddScene(std::shared_ptr<Scene> scene, uint32_t id);
    void SwitchSceneTo(uint32_t sceneId, const std::map<std::string, Value> &args = std::map<std::string, Value>());

    // Platform independant file loading
    static bool LoadFile(const char *filename, std::string &fileData);

    static bool HasTexture(const std::string &name);
    static SDL_Texture *GetTexture(const std::string &name);
    static void StoreTexture(const std::string &name, SDL_Texture *tex);
    static void DestroyTexture(const std::string &name);

private:
    struct Texture
    {
        SDL_Texture *tex{nullptr};
        uint32_t count{1};
    };

    // key: name
    static std::map<std::string, Texture> mTextures;

    // Key: id
    std::map<uint32_t, std::shared_ptr<Scene>> mScenes;
    bool mSceneActivated = false;
    uint32_t mCurrentSceneId = 0;

    std::map<std::string, Value> mArgs;
};

} // namespace gfx

