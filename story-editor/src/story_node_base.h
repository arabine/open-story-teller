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
/*
    typedef ::Factory<StoryNodeBase> Factory;
    virtual ~StoryNodeBase() {}
    virtual int answer() const = 0;

    static Factory::Type create(Factory::Key const& name) {
        return m_factory.create(name);
    }
    template<class Derived>
    static void define(Factory::Key const& name) {
        bool new_key = m_factory.define(name,
            &Factory::template create_func<StoryNodeBase, Derived>);
        if (not new_key) {
            throw std::logic_error(std::string(__PRETTY_FUNCTION__) +
                                   ": name already registered");
        }
    }
    */

    void setNodeId(NodeId id) { m_nodeId = id; }
    NodeId getNodeId() { return m_nodeId; }

    NodeGeometryData  &geometryData() { return m_geometryData; }

private:
    NodeId m_nodeId;
    NodeGeometryData m_geometryData;
};


#endif // STORY_NODE_BASE_H
