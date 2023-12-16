#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node.h"
#include "i_story_project.h"
#include "gui.h"
#include <imgui_node_editor.h>


class MediaNode : public BaseNode
{
public:
    MediaNode(const std::string &title, IStoryProject &proj);

    void Draw() override;

    virtual void FromJson(nlohmann::json &j) override;

    virtual void DrawProperties() override;

private:
    IStoryProject &m_project;
    Gui::Image  m_image;
    std::string m_soundName;
    std::string m_soundPath;

    std::string m_id;

    std::string m_buttonUniqueName;
};
