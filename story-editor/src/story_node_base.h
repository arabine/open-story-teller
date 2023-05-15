#ifndef STORY_NODE_BASE_H
#define STORY_NODE_BASE_H


#include <iostream>

#include <QtCore/QObject>
#include <QtWidgets/QLabel>

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/Definitions>
#include "json.hpp"

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

    virtual nlohmann::json ToJson() const {
        nlohmann::json j;

        j["model-name"] = name().toStdString();
        return j;
    }

    virtual void FromJson(nlohmann::json &j) {
        // default implementation does nothing
    }

    virtual void setInternalData(const nlohmann::json &j) {
        // default impl
    }

    virtual std::string Build() {
        return "";
    }

    NodeGeometryData  &geometryData() { return m_geometryData; }


private:
    NodeId m_nodeId;
    NodeGeometryData m_geometryData;
};


#endif // STORY_NODE_BASE_H
