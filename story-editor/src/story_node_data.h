#pragma once


#include <QtNodes/NodeData>

using QtNodes::NodeData;
using QtNodes::NodeDataType;

/// The class can potentially incapsulate any user data which
/// need to be transferred within the Node Editor graph
class StoryNodeData : public NodeData
{
public:
    StoryNodeData() {}

    StoryNodeData(uint32_t id)
        : m_id(id)
    {}

    NodeDataType type() const override
    {
        //       id      name
        return {"id", "ID"};
    }

    uint32_t id() const { return m_id; }

private:
    uint32_t m_id{0};
};
