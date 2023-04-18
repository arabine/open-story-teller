#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QPointF>
#include <QtCore/QSize>

#include <QtNodes/AbstractGraphModel>
#include <QtNodes/StyleCollection>

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>

using QtNodes::ConnectionStyle;
using QtNodes::NodeRole;
using QtNodes::StyleCollection;
using QtNodes::NodeDelegateModelRegistry;
using ConnectionId = QtNodes::ConnectionId;
using ConnectionPolicy = QtNodes::ConnectionPolicy;
using NodeFlag = QtNodes::NodeFlag;
using NodeId = QtNodes::NodeId;
using NodeRole = QtNodes::NodeRole;
using PortIndex = QtNodes::PortIndex;
using PortRole = QtNodes::PortRole;
using PortType = QtNodes::PortType;
using StyleCollection = QtNodes::StyleCollection;
using QtNodes::NodeDelegateModel;
using QtNodes::InvalidNodeId;

class PortAddRemoveWidget;

#include "story_node_base.h"

/*
template<class Interface, class KeyT=std::string>
struct Factory {
    typedef KeyT Key;
    typedef std::shared_ptr<Interface> Type;
    typedef Type (*Creator)();

    bool define(Key const& key, Creator v) {
        // Define key -> v relationship, return whether this is a new key.
        return _registry.insert(typename Registry::value_type(key, v)).second;
    }

    Type create(Key const& key) {
        typename Registry::const_iterator i = _registry.find(key);
        if (i == _registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else return i->second();
    }

    template<class Base, class Actual>
    static
    std::shared_ptr<Base> create_func() {
        return std::shared_ptr<Base>(new Actual());
    }

private:
    typedef std::map<Key, Creator> Registry;
    Registry _registry;
};
*/

/**
 * The class implements a bare minimum required to demonstrate a model-based
 * graph.
 */
class StoryGraphModel : public QtNodes::AbstractGraphModel
{
    Q_OBJECT
public:


public:
    StoryGraphModel();

    ~StoryGraphModel() override;

    std::set<QString> categories() { return m_categories; }
    void addModel(const QString &name, const QString &category) {
        m_modelNames[name] = category;
        m_categories.insert(category);
    }
    std::unordered_map<QString, QString> models() { return m_modelNames; }

    std::unordered_set<NodeId> allNodeIds() const override;

    std::unordered_set<ConnectionId> allConnectionIds(NodeId const nodeId) const override;

    std::unordered_set<ConnectionId> connections(NodeId nodeId,
                                                 PortType portType,
                                                 PortIndex portIndex) const override;

    bool connectionExists(ConnectionId const connectionId) const override;

    NodeId addNode(QString const nodeType = QString()) override;

    NodeId addNode(std::shared_ptr<StoryNodeBase> model);
    /**
   * Connection is possible when graph contains no connectivity data
   * in both directions `Out -> In` and `In -> Out`.
   */
    bool connectionPossible(ConnectionId const connectionId) const override;

    void addConnection(ConnectionId const connectionId) override;

    bool nodeExists(NodeId const nodeId) const override;

    QVariant nodeData(NodeId nodeId, NodeRole role) const override;

    bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

    QVariant portData(NodeId nodeId,
                      PortType portType,
                      PortIndex portIndex,
                      PortRole role) const override;

    bool setPortData(NodeId nodeId,
                     PortType portType,
                     PortIndex portIndex,
                     QVariant const &value,
                     PortRole role = PortRole::Data) override;

    bool deleteConnection(ConnectionId const connectionId) override;

    bool deleteNode(NodeId const nodeId) override;

    QJsonObject saveNode(NodeId const) const override;

    QJsonObject save() const;

    /// @brief Creates a new node based on the informatoin in `nodeJson`.
    /**
   * @param nodeJson conains a `NodeId`, node's position, internal node
   * information.
   */
    void loadNode(QJsonObject const &nodeJson) override;

    void load(QJsonObject const &jsonDocument);

    void addPort(NodeId nodeId, PortType portType, PortIndex portIndex);

    void removePort(NodeId nodeId, PortType portType, PortIndex first);

    NodeId newNodeId() override { return _nextNodeId++; }


    template<class Derived>
    void registerNode(const std::string& name) {
        m_registry.insert(typename Registry::value_type(name, Factory<Derived>::create_func));
    }

    std::shared_ptr<StoryNodeBase> createNode(const std::string& key) {
        typename Registry::const_iterator i = m_registry.find(key);
        if (i == m_registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else return i->second(*this);
    }

private:
    std::unordered_set<NodeId> _nodeIds;

    // Key: widget name, value: category
    std::unordered_map<QString, QString> m_modelNames;
    std::set<QString> m_categories;


    template<class NodeType>
    struct Factory {
        static std::shared_ptr<StoryNodeBase> create_func(StoryGraphModel &model) {
            return std::make_shared<NodeType>(model);
        }
    };

    typedef std::shared_ptr<StoryNodeBase> (*GenericCreator)(StoryGraphModel &model);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    /// [Important] This is a user defined data structure backing your model.
    /// In your case it could be anything else representing a graph, for example, a
    /// table. Or a collection of structs with pointers to each other. Or an
    /// abstract syntax tree, you name it.
    std::unordered_set<ConnectionId> _connectivity;

    std::unordered_map<NodeId, std::shared_ptr<StoryNodeBase>> _models;

    /// A convenience variable needed for generating unique node ids.
    unsigned int _nextNodeId{0};
};
