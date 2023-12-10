#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node.h"
#include "gui.h"
#include <imgui_node_editor.h>


class MediaNode : public BaseNode
{
public:
    MediaNode(const std::string &title, StoryProject &proj);

    void Draw() override;

    virtual void FromJson(nlohmann::json &j) override;

private:
    StoryProject &m_project;
    Gui::Image  m_image;
    std::string m_soundName;
    std::string m_soundPath;

};
