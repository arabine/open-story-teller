#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_manager.h"
#include "base_node.h"
#include "gui.h"

namespace Nw
{

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
};

enum class PinKind
{
    Output,
    Input
};

enum class NodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini
};

struct PinStyle
{
    /// @brief Socket and link color
    ImU32 color{IM_COL32(255,255,255,255)};
    /// @brief Socket shape ID
    int socket_shape{5};
    /// @brief Socket radius
    float socket_radius{4.f};
    /// @brief Socket radius when hovered
    float socket_hovered_radius{4.4f};
    /// @brief Socket radius when connected
    float socket_connected_radius{4.2f};
    /// @brief Socket outline thickness when empty
    float socket_thickness{1.f};
    ImVec2 padding = ImVec2(3.f, 1.f);
    /// @brief Border and background corner rounding
    float bg_radius = 8.f;
    /// @brief Border thickness
    float border_thickness = 1.f;
    /// @brief Background color
    ImU32 bg_color = IM_COL32(23, 16, 16, 0);
    /// @brief Background color when hovered
    ImU32 bg_hover_color = IM_COL32(100, 100, 255, 70);
    /// @brief Border color
    ImU32 border_color = IM_COL32(255, 255, 255, 0);
};

struct Pin
{
    ImVec2 pos = ImVec2(0.f, 0.f);
    ImVec2 size;
    ImVec2 pinPoint = ImVec2(0.f, 0.f);
    bool isConnected{false};
    int index{0};
    PinKind pinKind{PinKind::Input};
    PinStyle style;
};

} // namespace Nw

/**
 * @brief Basically a wrapper class around ImGuiNodeEditor Node structure
 * 
 */
class BaseNodeWidget
{
public:
    BaseNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    virtual ~BaseNodeWidget();

    virtual void Initialize();

    virtual void Draw() = 0;
    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) = 0;


    virtual void DrawSocket(const Nw::Pin &pin);


    virtual bool HasSync() const { 
        bool hasSync = false;
        if (m_base)
        {
            // Si c'est un noeud executable, il y a une entrée de syncho (lein entre les fonctions, appels)
            if (m_base->IsExecutable())
            {
                hasSync = true;
            }
        }
        return hasSync;
     }

    uint32_t Inputs() const { return m_base->InputsCount(); }
    uint32_t Outputs() const { return m_base->OutputsCount(); }


    static unsigned long GetNextId()
    {
        return s_nextId++;
    }

    static void InitId() {
        s_nextId = 1;
    }

    std::shared_ptr<BaseNode> Base() { return m_base; }

    void SetTitle(const std::string& title) { m_title = title; }
    std::string GetTitle() const { return m_title; }

private:
    IStoryManager &m_manager;
    std::shared_ptr<BaseNode> m_base;
    std::string m_title;

    bool m_firstFrame{true};

    static unsigned long s_nextId;
};

