#ifndef STORY_NODE_BASE_H
#define STORY_NODE_BASE_H


#include <iostream>

#include <QtCore/QObject>
#include <QtWidgets/QLabel>

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/Definitions>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::NodeId;

class StoryNodeBase : public NodeDelegateModel
{
public:
    struct NodeGeometryData
    {
        QSize size;
        QPointF pos;
    };

    ~StoryNodeBase() {
        std::cout << "Delete node: " << m_nodeId << std::endl;
    }

    void setNodeId(NodeId id) { m_nodeId = id; }
    NodeId getNodeId() { return m_nodeId; }

    virtual void setInternalData(const QVariant &value) {
        // default impl
    }

    NodeGeometryData  &geometryData() { return m_geometryData; }

private:
    NodeId m_nodeId;
    NodeGeometryData m_geometryData;
};


#endif // STORY_NODE_BASE_H
