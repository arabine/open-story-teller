#pragma once

#include <string>
#include <memory>
#include "json.hpp"
#include <random>
#include <string>

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



// Encaasulate the genaeration of a Version 4 UUID object
// A Version 4 UUID is a universally unique identifier that is generated using random numbers.
class UUID
{
public:

    UUID() { New(); }

    // Factory method for creating UUID object.
    void New()
    {
        std::random_device rd;
        std::mt19937 engine{rd()};
        std::uniform_int_distribution<int> dist{0, 256}; //Limits of the interval

        for (int index = 0; index < 16; ++index)
        {
            _data[index] = (unsigned char)dist(engine);
        }

        _data[6] = ((_data[6] & 0x0f) | 0x40); // Version 4
        _data[8] = ((_data[8] & 0x3f) | 0x80); // Variant is 10
    }

    // Returns UUID as formatted string
    std::string String()
    {
        // Formats to "0065e7d7-418c-4da4-b4d6-b54b6cf7466a"
        char buffer[256] = {0};
        std::snprintf(buffer, 255,
                      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                      _data[0], _data[1], _data[2], _data[3],
                      _data[4], _data[5],
                      _data[6], _data[7],
                      _data[8], _data[9],
                      _data[10], _data[11], _data[12], _data[13], _data[14], _data[15]);

        std::string uuid = buffer;

        return uuid;
    }


    unsigned char _data[16] = {0};
};


class BaseNode
{
public:
    struct NodePosition
    {
        int x;
        int y;
    };

    BaseNode(const std::string &title);


    virtual void Draw() = 0;

    void SetPosition(int x, int y);

    void FrameStart();
    void FrameEnd();

    void DrawPins();

    uint32_t Outputs() const { return m_outputs; }

    void SetOutputs(uint32_t outputs) { m_outputs = outputs; }

    void SetId(const std::string &id) { m_id = id; }
    std::string GetId() const { return m_id; }

    void seTitle(const std::string &title) { m_title = title; }
    std::string getTitle() const { return m_title; }

    virtual nlohmann::json ToJson() const {
        nlohmann::json j;

        j["type"] = m_type;
        return j;
    }

    static int GetNextId()
    {
        return s_nextId++;
    }

private:
    std::unique_ptr<Node> m_node;

    std::string m_title{"Base node"};
    std::string m_type;
    std::string m_id;
    NodePosition m_pos;

    uint32_t m_outputs{1};
    uint32_t m_inputs{1};

    static int s_nextId;




};

