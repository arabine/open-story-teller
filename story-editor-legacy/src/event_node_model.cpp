#include "event_node_model.h"

#include "story_node_data.h"

#include <QtNodes/NodeDelegateModelRegistry>

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtWidgets/QFileDialog>



EventNodeModel::EventNodeModel()
    : m_widget(new QWidget())
{
    m_ui.setupUi(m_widget);
}

QJsonObject EventNodeModel::save() const
{
    QJsonObject obj = NodeDelegateModel::save();

    // Merge two objects
    QVariantMap map = obj.toVariantMap();
    map.insert(m_mediaData.toVariantMap());

    return QJsonObject::fromVariantMap(map);
}

void EventNodeModel::load(const QJsonObject &mediaData)
{
    m_mediaData = mediaData;

}

unsigned int EventNodeModel::nPorts(PortType portType) const
{
    unsigned int result = 1;

    switch (portType) {
    case PortType::In:
        result = 1;
        break;

    case PortType::Out:
        result = 1;

    default:
        break;
    }

    return result;
}


NodeDataType EventNodeModel::dataType(PortType const, PortIndex const) const
{
    return StoryNodeData().type();
}

std::shared_ptr<NodeData> EventNodeModel::outData(PortIndex)
{
    return m_nodeData;
}

void EventNodeModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex const)
{
    m_nodeData = nodeData;



    Q_EMIT dataUpdated(0);
}
