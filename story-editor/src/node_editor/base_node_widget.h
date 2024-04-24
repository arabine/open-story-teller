#pragma once

#include <string>
#include <memory>
#include <random>
#include <string>

#include "json.hpp"
#include "i_story_manager.h"
#include "base_node.h"

#include <imgui_node_editor.h>
namespace ed = ax::NodeEditor;

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



struct Node;

struct Pin
{
    ed::PinId   ID;
    ::Node*     Node;
    std::string Name;
    PinType     Type;
    PinKind     Kind;

    Pin(int id, const char* name, PinType type):
        ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
    {
    }
};

struct Node
{
    ed::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    ImColor Color;
    NodeType Type;
    ImVec2 Size;

    std::string State;
    std::string SavedState;

    Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)):
        ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
    {
    }
};

struct Link
{
    ed::LinkId ID;

    ed::PinId StartPinID;
    ed::PinId EndPinID;

    ImColor Color;

    Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId):
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

/**
 * @brief Basically a wrapper class around ImGuiNodeEditor Node structure
 * 
 */
class BaseNodeWidget : public BaseNode
{
public:
    struct NodePosition
    {
        float x;
        float y;
    };

    BaseNodeWidget(const std::string &type, IStoryManager &proj);

    virtual void Initialize();

    virtual void Draw() = 0;
    virtual void DrawProperties() = 0;
    virtual std::string GenerateConstants() = 0;
    virtual std::string Build() = 0;
    virtual std::string GetEntryLabel() = 0;


    void FrameStart();
    void FrameEnd();
    void DrawPins();

    virtual float GetX() const;
    virtual float GetY() const;


    uint32_t Inputs() const { return m_node->Inputs.size(); }
    uint32_t Outputs() const { return m_node->Outputs.size(); }

    unsigned long GetInternalId() const { return m_node->ID.Get(); }


    static unsigned long GetNextId()
    {
        return s_nextId++;
    }

    static void InitId() {
        s_nextId = 1;
    }


    ed::PinId GetInputPinAt(int index)
    {
        ed::PinId id = 0;

        if (index < static_cast<int>(m_node->Inputs.size()))
        {
            id = m_node->Inputs[index].ID;
        }

        return id;
    }

    ed::PinId GetOutputPinAt(int index)
    {
        ed::PinId id = 0;

        if (index < static_cast<int>(m_node->Outputs.size()))
        {
            id = m_node->Outputs[index].ID;
        }

        return id;
    }

    bool HasInputPinId(ed::PinId &pinId, int &atIndex) const
    {
        bool found = false;
        atIndex = 0;
        for (auto &i : m_node->Inputs)
        {
            if (i.ID == pinId)
            {
                found = true;
                break;
            }
            atIndex++;
        }
        return found;
    }

    bool HasOnputPinId(ed::PinId &pinId, int &atIndex) const
    {
        bool found = false;
        atIndex = 0;
        for (auto &i : m_node->Outputs)
        {
            if (i.ID == pinId)
            {
                found = true;
                break;
            }
            atIndex++;
        }
        return found;
    }


    void AddInput();
    void AddOutputs(int num = 1);
    void SetOutputs(uint32_t num);
    void DeleteOutput();

private:
    IStoryManager &m_story;

    std::unique_ptr<Node> m_node;

    bool m_firstFrame{true};

    static unsigned long s_nextId;

};

