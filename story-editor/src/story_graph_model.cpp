#include "story_graph_model.h"
#include "media_node_model.h"

#include <QtNodes/ConnectionIdUtils>
#include <QJsonArray>
#include <iterator>
#include <QDir>

StoryGraphModel::StoryGraphModel(StoryProject &project)
    : m_project(project)
{
    m_player = new QMediaPlayer;
    m_audioOutput = new QAudioOutput;
    m_player->setAudioOutput(m_audioOutput);
    
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [&](QMediaPlayer::PlaybackState newState) {
        if (newState == QMediaPlayer::PlaybackState::StoppedState) {
            m_player->stop();
            emit sigAudioStopped();
        }
    });
}

StoryGraphModel::~StoryGraphModel()
{
    //
}

std::unordered_set<NodeId> StoryGraphModel::allNodeIds() const
{
    return _nodeIds;
}

std::unordered_set<ConnectionId> StoryGraphModel::allConnectionIds(NodeId const nodeId) const
{
    std::unordered_set<ConnectionId> result;

    std::copy_if(_connectivity.begin(),
                 _connectivity.end(),
                 std::inserter(result, std::end(result)),
                 [&nodeId](ConnectionId const &cid) {
                     return cid.inNodeId == nodeId || cid.outNodeId == nodeId;
                 });

    return result;
}

std::unordered_set<ConnectionId> StoryGraphModel::connections(NodeId nodeId,
                                                                PortType portType,
                                                                PortIndex portIndex) const
{
    std::unordered_set<ConnectionId> result;

    std::copy_if(_connectivity.begin(),
                 _connectivity.end(),
                 std::inserter(result, std::end(result)),
                 [&portType, &portIndex, &nodeId](ConnectionId const &cid) {
                     return (getNodeId(portType, cid) == nodeId
                             && getPortIndex(portType, cid) == portIndex);
                 });

    return result;
}

bool StoryGraphModel::connectionExists(ConnectionId const connectionId) const
{
    return (_connectivity.find(connectionId) != _connectivity.end());
}

NodeId StoryGraphModel::addNode(QString const nodeType)
{
    NodeId id = InvalidNodeId;
    std::cout << "-------------------> add node: " << nodeType.toStdString() << std::endl;

    try {

        NodeId newId = newNodeId();

        // Create new node.
        _nodeIds.insert(newId);

        _models[newId] = createNode(nodeType.toStdString());
        _models[newId]->setNodeId(newId);

        Q_EMIT nodeCreated(newId);

        id = newId;

    } catch(std::invalid_argument e) {
        std::cout << e.what() << std::endl;
    }

    return id;
}


NodeId StoryGraphModel::addNode(std::shared_ptr<StoryNodeBase> model)
{
    NodeId newId = newNodeId();

    // Create new node.
    _nodeIds.insert(newId);

    _models[newId] = model;

    Q_EMIT nodeCreated(newId);

    return newId;
}



bool StoryGraphModel::connectionPossible(ConnectionId const connectionId) const
{
    return !connectionExists(connectionId);
}

void StoryGraphModel::addConnection(ConnectionId const connectionId)
{
    _connectivity.insert(connectionId);

    Q_EMIT connectionCreated(connectionId);
}

bool StoryGraphModel::nodeExists(NodeId const nodeId) const
{
    return (_nodeIds.find(nodeId) != _nodeIds.end());
}


QVariant StoryGraphModel::nodeData(NodeId nodeId, NodeRole role) const
{
    QVariant result;

    auto it = _models.find(nodeId);
    if (it == _models.end())
        return result;

    auto &model = it->second;

    switch (role) {
    case NodeRole::Type:
        result = QString("Default Node Type");
        break;

    case NodeRole::Position:
        result = model->geometryData().pos;
        break;

    case NodeRole::Size:
        result = model->geometryData().size;
        break;

    case NodeRole::CaptionVisible:
        result = true;
        break;

    case NodeRole::Caption:
        result = model->caption();
        break;

    case NodeRole::Style: {
        auto style = StyleCollection::nodeStyle();
        result = style.toJson().toVariantMap();
    } break;

    case NodeRole::InternalData:
        break;

    case NodeRole::InPortCount:
        result = model->nPorts(PortType::In);
        break;

    case NodeRole::OutPortCount:
        result = model->nPorts(PortType::Out);
        break;

    case NodeRole::Widget: {
        auto w = model->embeddedWidget();
        result = QVariant::fromValue(w);
        break;
    }
    case NodeRole::Id:
        result = model->getNodeId();
        break;
    }

    return result;
}

void StoryGraphModel::SetInternalData(NodeId nodeId, nlohmann::json &j)
{
    auto it = _models.find(nodeId);
    if (it == _models.end())
        return;

    auto &model = it->second;

    model->setInternalData(j);
}

bool StoryGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value)
{
    bool result = false;

    auto it = _models.find(nodeId);
    if (it == _models.end())
        return result;

    auto &model = it->second;

    switch (role) {
    case NodeRole::Type:
        break;
    case NodeRole::Position: {        
        model->geometryData().pos = value.value<QPointF>();

        Q_EMIT nodePositionUpdated(nodeId);

        result = true;
    } break;

    case NodeRole::Size: {
        model->geometryData().size = value.value<QSize>();
        result = true;
    } break;

    case NodeRole::CaptionVisible:
        break;

    case NodeRole::Caption:
        break;

    case NodeRole::Style:
        break;

    case NodeRole::InternalData:
        break;

    case NodeRole::InPortCount:
        break;

    case NodeRole::OutPortCount:
        break;

    case NodeRole::Widget:
        break;
    }

    return result;
}

QVariant StoryGraphModel::portData(NodeId nodeId,
                                     PortType portType,
                                     PortIndex portIndex,
                                     PortRole role) const
{
    switch (role) {
    case PortRole::Data:
        return QVariant();
        break;

    case PortRole::DataType:
        return QVariant();
        break;

    case PortRole::ConnectionPolicyRole:
        return QVariant::fromValue(ConnectionPolicy::One);
        break;

    case PortRole::CaptionVisible:
        return true;
        break;

    case PortRole::Caption:
        if (portType == PortType::In)
            return QString::fromUtf8("Port In");
        else
            return QString::fromUtf8("Port Out");

        break;
    }

    return QVariant();
}

bool StoryGraphModel::setPortData(
    NodeId nodeId, PortType portType, PortIndex portIndex, QVariant const &value, PortRole role)
{
    Q_UNUSED(nodeId);
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

bool StoryGraphModel::deleteConnection(ConnectionId const connectionId)
{
    bool disconnected = false;

    auto it = _connectivity.find(connectionId);

    if (it != _connectivity.end()) {
        disconnected = true;

        _connectivity.erase(it);
    };

    if (disconnected)
        Q_EMIT connectionDeleted(connectionId);

    return disconnected;
}

bool StoryGraphModel::deleteNode(NodeId const nodeId)
{
    // Delete connections to this node first.
    auto connectionIds = allConnectionIds(nodeId);
    for (auto &cId : connectionIds) {
        deleteConnection(cId);
    }

    _nodeIds.erase(nodeId);
    Q_EMIT nodeDeleted(nodeId);

    return true;
}

namespace QtNodes {
    void to_json(nlohmann::json& j, const ConnectionId& p) {
        j = nlohmann::json{
            {"outNodeId", static_cast<qint64>(p.outNodeId)},
            {"outPortIndex", static_cast<qint64>(p.outPortIndex)},
            {"inNodeId", static_cast<qint64>(p.inNodeId)},
            {"inPortIndex", static_cast<qint64>(p.inPortIndex)},
        };
    }

    void from_json(const nlohmann::json& j, ConnectionId& p) {
        j.at("outNodeId").get_to(p.outNodeId);
        j.at("outPortIndex").get_to(p.outPortIndex);
        j.at("inNodeId").get_to(p.inNodeId);
        j.at("inPortIndex").get_to(p.inPortIndex);
    }
} // namespace QtNodes


nlohmann::json StoryGraphModel::Save() const
{
    nlohmann::json j;
    nlohmann::json nodesJsonArray;
    for (auto const nodeId : allNodeIds()) {
        nodesJsonArray.push_back(SaveNode(nodeId));
    }
    j["nodes"] = nodesJsonArray;

    nlohmann::json connJsonArray;
    for (auto const &cid : _connectivity) {
        nlohmann::json o = cid;
        connJsonArray.push_back(o);
    }
    j["connections"] = connJsonArray;

    return j;
}

void StoryGraphModel::Load(const nlohmann::json &j)
{
    nlohmann::json nodesJsonArray = j["nodes"];

    for (auto& element : nodesJsonArray) {
        LoadNode(element);
    }

    std::cout << j.dump(4) << std::endl;

    nlohmann::json connectionJsonArray = j["connections"];

    for (auto& connection : connectionJsonArray) {
        ConnectionId connId = connection.get<QtNodes::ConnectionId>();
        // Restore the connection
        addConnection(connId);
    }
}

nlohmann::json StoryGraphModel::SaveNode(NodeId const nodeId) const
{
    nlohmann::json nodeJson;
    nodeJson["id"] = static_cast<qint64>(nodeId);

    auto it = _models.find(nodeId);
    if (it == _models.end())
        return nodeJson;

    auto &model = it->second;

    nodeJson["internal-data"] = model->ToJson();

    {
        QPointF const pos = nodeData(nodeId, NodeRole::Position).value<QPointF>();

        nlohmann::json posJson;
        posJson["x"] = pos.x();
        posJson["y"] = pos.y();
        nodeJson["position"] = posJson;

        nodeJson["inPortCount"] = nodeData(nodeId, NodeRole::InPortCount).value<int>();
        nodeJson["outPortCount"] = nodeData(nodeId, NodeRole::OutPortCount).value<int>();
    }

    return nodeJson;
}

void StoryGraphModel::PlaySound(const QString &fileName)
{
    m_player->setSource(QUrl::fromLocalFile(fileName));
    m_audioOutput->setVolume(50);
    m_player->play();
}


void StoryGraphModel::LoadNode(const nlohmann::json &nodeJson)
{
    // Possibility of the id clash when reading it from json and not generating a
    // new value.
    // 1. When restoring a scene from a file.
    // Conflict is not possible because the scene must be cleared by the time of
    // loading.
    // 2. When undoing the deletion command.  Conflict is not possible
    // because all the new ids were created past the removed nodes.
    NodeId restoredNodeId = nodeJson["id"].get<int>();

    _nextNodeId = std::max(_nextNodeId, restoredNodeId + 1);

    nlohmann::json internalDataJson = nodeJson["internal-data"];

    std::string delegateModelName = internalDataJson["model-name"].get<std::string>();

//    std::unique_ptr<NodeDelegateModel> model = _registry->create(delegateModelName);

    auto model = createNode(delegateModelName);

    if (model) {
//        connect(model.get(),
//                &NodeDelegateModel::dataUpdated,
//                [restoredNodeId, this](PortIndex const portIndex) {
//                    onOutPortDataUpdated(restoredNodeId, portIndex);
//                });


        _models[restoredNodeId] = model;
        model->setNodeId(restoredNodeId);
        _nodeIds.insert(restoredNodeId);

        Q_EMIT nodeCreated(restoredNodeId);

        nlohmann::json posJson = nodeJson["position"];
        QPointF const pos(posJson["x"].get<double>(), posJson["y"].get<double>());

        setNodeData(restoredNodeId, NodeRole::Position, pos);

        _models[restoredNodeId]->FromJson(internalDataJson);
    } else {
        throw std::logic_error(std::string("No registered model with name ") + delegateModelName);
    }
}

NodeId StoryGraphModel::FindFirstNode() const
{
    NodeId id{InvalidNodeId};

    // First node is the one without connection on its input port

    for (auto const nodeId : allNodeIds())
    {
        bool foundConnection{false};
        for (auto& c : _connectivity)
        {
            if (c.outNodeId == nodeId)
            {
                foundConnection = true;
            }

        }

        if (!foundConnection)
        {
            id = nodeId;
            qDebug() << "First node is: " << id;
            break;
        }
    }

    return id;
}

std::string StoryGraphModel::Build()
{
    std::stringstream chip32;

    FindFirstNode();

    chip32 << "\tjump         .entry\r\n";

    // First generate all constants
    for (auto const nodeId : allNodeIds())
    {
        auto it = _models.find(nodeId);
        if (it != _models.end())
        {
            chip32 << it->second->GenerateConstants() << "\n";
        }
    }

    chip32 << ".entry:\r\n";

    nlohmann::json nodesJsonArray;
    for (auto const nodeId : allNodeIds())
    {
        auto it = _models.find(nodeId);
        if (it == _models.end())
            return "";

        auto &model = it->second;
        chip32 << model->Build() << "\n";
    }

    return chip32.str();
}

void StoryGraphModel::addPort(NodeId nodeId, PortType portType, PortIndex portIndex)
{
    // STAGE 1.
    // Compute new addresses for the existing connections that are shifted and
    // placed after the new ones
    PortIndex first = portIndex;
    PortIndex last = first;
    portsAboutToBeInserted(nodeId, portType, first, last);

    // STAGE 3. Re-create previouly existed and now shifted connections
    portsInserted();   
}

void StoryGraphModel::removePort(NodeId nodeId, PortType portType, PortIndex portIndex)
{
    // STAGE 1.
    // Compute new addresses for the existing connections that are shifted upwards
    // instead of the deleted ports.
    PortIndex first = portIndex;
    PortIndex last = first;
    portsAboutToBeDeleted(nodeId, portType, first, last);

    portsDeleted();
}

QString StoryGraphModel::GetImagesDir() const
{
    return QString(m_project.GetWorkingDir().c_str()) + QDir::separator() + "images";
}

QString StoryGraphModel::BuildFullImagePath(const QString &fileName) const
{
    return GetImagesDir() + QDir::separator() + fileName;
}

QString StoryGraphModel::GetSoundsDir() const
{
    return QString(m_project.GetWorkingDir().c_str()) + QDir::separator() + "sounds";
}

QString StoryGraphModel::BuildFullSoundPath(const QString &fileName) const
{
    return GetSoundsDir() + QDir::separator() + fileName;
}

void StoryGraphModel::Clear()
{
    _nodeIds.clear();
    _connectivity.clear();
    _models.clear();
    emit modelReset();
}
