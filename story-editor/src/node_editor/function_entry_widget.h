#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "function_entry_node.h"
#include "gui.h"


class FunctionEntryWidget : public BaseNodeWidget
{
public:
    FunctionEntryWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_functionEntryNode = std::dynamic_pointer_cast<FunctionEntryNode>(node);
        SetTitle("Function Entry");
    }

    void Draw() override {
        ImGui::SetNextItemWidth(100.f);
    }

    void DrawSocket(uint32_t index, bool isInput, ImVec2 pin_pos, bool isConnected) override
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float socket_size = 4;

        // Définir les points du polygone pour le symbole de synchronisation
        // C'est un polygone fermé à 5 points
        ImVec2 p1(pin_pos.x - socket_size * 0.5f, pin_pos.y - socket_size);
        ImVec2 p2(pin_pos.x + socket_size * 0.5f, pin_pos.y - socket_size);
        ImVec2 p3(pin_pos.x + socket_size * 0.5f, pin_pos.y + socket_size);
        ImVec2 p4(pin_pos.x - socket_size * 0.5f, pin_pos.y + socket_size);
        ImVec2 p5(pin_pos.x + socket_size * 1.5f, pin_pos.y);

        ImVec2 vertices[] = {p1, p2, p5, p3, p4}; // Ordre des sommets

        // Pour la détection de survol (hover) on peut toujours utiliser le rectangle englobant
        ImVec2 tl = pin_pos - ImVec2(socket_size * 1.5f, socket_size);
        ImVec2 br = pin_pos + ImVec2(socket_size * 1.5f, socket_size);

        if (isConnected)
        {
            draw_list->AddConvexPolyFilled(vertices, IM_ARRAYSIZE(vertices), IM_COL32(255,255,255,255));
        }
        else
        {
            if (ImGui::IsItemHovered() || ImGui::IsMouseHoveringRect(tl, br))
            {
                draw_list->AddPolyline(vertices, IM_ARRAYSIZE(vertices),IM_COL32(255,255,255,255), ImDrawFlags_Closed, 4.67f);
            }
            else
            {
                draw_list->AddPolyline(vertices, IM_ARRAYSIZE(vertices), IM_COL32(255,255,255,255), ImDrawFlags_Closed, 1.3f);
            }
        }
    }

    virtual bool HasSync() const override { 
        return false;
    }

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override {

    }
    virtual void Initialize() override {

    }

private:
    IStoryManager &m_manager;

    std::shared_ptr<FunctionEntryNode> m_functionEntryNode;
 
};
