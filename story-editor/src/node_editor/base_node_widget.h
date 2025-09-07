#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_manager.h"
#include "base_node.h"
#include "gui.h"

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
    ImU32 color;
    /// @brief Socket shape ID
    int socket_shape;
    /// @brief Socket radius
    float socket_radius;
    /// @brief Socket radius when hovered
    float socket_hovered_radius;
    /// @brief Socket radius when connected
    float socket_connected_radius;
    /// @brief Socket outline thickness when empty
    float socket_thickness;
};

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


    virtual void DrawSocket(uint32_t index, bool isInput, ImVec2 pin_pos, bool isConnected)  {}


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

