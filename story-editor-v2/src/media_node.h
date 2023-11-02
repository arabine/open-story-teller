#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node.h"
#include "Gui.h"
#include <imgui_node_editor.h>


class MediaNode : public BaseNode
{
public:
    MediaNode(const std::string &title);

    void Draw() override;

private:
    Gui::Image  m_image;

};
