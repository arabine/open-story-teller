
#include "engine.h"

using namespace gfx;

void Engine::AddScene(std::shared_ptr<Scene> scene, uint32_t id)
{
    mScenes[id] = scene;
}

void Engine::SwitchSceneTo(uint32_t sceneId, const std::map<std::string, Value> &args)
{
    mCurrentSceneId = sceneId;
    mSceneActivated = true;
    mArgs = args;
}

uint32_t Display::GetScreenW()
{
    return mScreenWidth;
}
uint32_t Display::GetScreenH()
{
    return mScreenHeight;
}


void Engine::Warmup()
{
    currentTick = SDL_GetPerformanceCounter();

    for (auto &s : mScenes)
    {
        s.second->OnCreate(mRenderer);
    }
}

uint32_t Engine::Process()
{
    uint32_t nextScene = 0;

    SDL_SetRenderDrawColor(mRenderer, mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, mBackgroundColor.a);
    SDL_RenderClear(mRenderer);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // without it you won't have keyboard input and other things
        ImGui_ImplSDL2_ProcessEvent(&event);
        // you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
        // before processing events

        switch (event.type)
        {
        case SDL_QUIT:
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                return 10000;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                mWidth = event.window.data1;
                mHeight = event.window.data2;
                // std::cout << "[INFO] Window size: "
                //           << windowWidth
                //           << "x"
                //           << windowHeight
                //           << std::endl;
                break;
            }
            break;
        }

        if (mScenes.count(mCurrentSceneId))
        {
            auto s = mScenes.at(mCurrentSceneId);
            s->ProcessEvent(event);
        }
    }

    // start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(mWindow);
    ImGui::NewFrame();

    if (mScenes.count(mCurrentSceneId))
    {
        auto s = mScenes.at(mCurrentSceneId);
        if (mSceneActivated)
        {
            mSceneActivated = false;
            s->OnActivate(mRenderer, mArgs);
        }

        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency());

        s->Update(deltaTime);
        s->Draw(mRenderer);

        nextScene = s->GetNextScene();
        if (nextScene > 0)
        {
            SwitchSceneTo(nextScene, s->GetArgs());
        }
    }
    else
    {
        // FIXME: log error
    }

    // rendering
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(mRenderer);

    return nextScene;
}

bool Engine::LoadFile(const char *filename, std::string &fileData)
{
    size_t size = 0;
    bool success = false;
    void *f = SDL_LoadFile(filename, &size);

    if ((f != nullptr) && (size > 0))
    {
        fileData.assign(reinterpret_cast<char *>(f), size);
        success = true;
    }
    return success;
}

std::map<std::string, Engine::Texture> Engine::mTextures;

bool Engine::HasTexture(const std::string &name)
{
    return mTextures.count(name) > 0;
}

SDL_Texture *Engine::GetTexture(const std::string &name)
{
    SDL_Texture *tex{nullptr};

    if (HasTexture(name))
    {
        tex = mTextures[name].tex;
    }

    return tex;
}

void Engine::StoreTexture(const std::string &name, SDL_Texture *tex)
{
    if (HasTexture(name))
    {
        mTextures[name].count++;
    }
    else
    {
        Texture t;
        t.tex = tex;
        mTextures[name] = t;
    }
}

void Engine::DestroyTexture(const std::string &name)
{
    if (HasTexture(name))
    {
        mTextures[name].count--;

        if (mTextures[name].count == 0)
        {
            SDL_DestroyTexture(mTextures[name].tex);
            mTextures.erase(name);
        }
    }
}
