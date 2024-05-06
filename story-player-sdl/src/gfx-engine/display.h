#pragma once

#include <string>
#include <cstdint>

namespace gfx
{

class Display
{
public:
    Display();

    struct Size {
        int w;
        int h;
    };

    bool Initialize(const std::string &title);
    bool PollEvent();
    void StartFrame();
    void EndFrame();
    void Destroy();
    void SetWindowTitle(const std::string &title);

    static Size GetWindowSize();


protected:
    float mRatioW{1.0};
    float mRatioH{1.0};

    float mRatio{1.0};

    // Taille de base
    uint32_t mWidth = 648;
    uint32_t mHeight = 960;

    // Taille réelle
    uint32_t mScreenWidth{648};
    uint32_t mScreenHeight{960};


private:
    std::string m_executablePath;

    const uint32_t mMinimumWidth = 648;
    const uint32_t mMinimumHeight = 960;

    

    uint64_t currentTick = 0;
    uint64_t lastTick = 0;
    double deltaTime = 0;


};

} // namespace Gfx

