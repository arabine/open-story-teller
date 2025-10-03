#pragma once

#include "window_base.h"
#include "i_story_manager.h"
#include "gui.h"
#include <vector>
#include <string>
#include <mutex>

class EmulatorDock : public WindowBase
{
public:
    EmulatorDock(IStoryManager &proj);

    void Initialize();
    virtual void Draw() override;

    void ClearImage();
    void SetImage(const std::string &image);
    
    // NEW: Console VM pour afficher la sortie des prints
    void AddVmOutput(const std::string& text);
    void ClearVmOutput();

private:
    IStoryManager &m_story;
    Gui::Image m_image;
    std::string m_imageFileName;
    
    // NEW: Buffer pour la console VM
    struct VmOutputEntry {
        std::string text;
        uint32_t type; // 0 = normal, 1 = error
    };
    
    std::vector<VmOutputEntry> m_vmOutput;
    std::mutex m_vmOutputMutex;
    bool m_autoScrollVm = true;
    
    void DrawVmConsole();
};