#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "gui.h"
#include <imgui_node_editor.h>


class MediaNode : public BaseNodeWidget
{
public:
    MediaNode(const std::string &title, IStoryManager &proj);

    void Draw() override;

    virtual void DrawProperties() override;
    virtual std::string Build() override;
    virtual std::string GetEntryLabel() override;
    virtual std::string GenerateConstants() override;

    virtual void Initialize() override;
    

private:
    IStoryManager &m_story;
    Gui::Image  m_image;
    std::string m_soundName;
    std::string m_soundPath;
    std::string m_buttonUniqueName;

    void SetImage(const std::string &f);
    void SetSound(const std::string &f);
    std::string ChoiceLabel() const;
    void StoreInternalData();
};
