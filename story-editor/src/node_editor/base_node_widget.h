#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_manager.h"
#include "base_node.h"


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

private:
    IStoryManager &m_manager;
    std::shared_ptr<BaseNode> m_base;

    bool m_firstFrame{true};

    static unsigned long s_nextId;
};

