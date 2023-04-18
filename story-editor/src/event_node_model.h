#pragma once

#include <iostream>

#include <QtCore/QObject>
#include <QtWidgets/QLabel>

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

#include "ui_event-node.h"

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class EventNodeModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    EventNodeModel();

    ~EventNodeModel() = default;

public:
    QString caption() const override { return QString("Event Node"); }

    QString name() const override { return QString("EventNode"); }

public:
    QJsonObject save() const override;

    void load(QJsonObject const &mediaData) override;

public:
    virtual QString modelName() const { return QString("EventNode"); }

    unsigned int nPorts(PortType const portType) const override;

    NodeDataType dataType(PortType const portType, PortIndex const portIndex) const override;

    std::shared_ptr<NodeData> outData(PortIndex const port) override;

    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex const port) override;

    QWidget *embeddedWidget() override { return m_widget; }

    bool resizable() const override { return true; }

private:
    QWidget *m_widget;

    Ui::eventNodeUi m_ui;

    std::shared_ptr<NodeData> m_nodeData;

    QJsonObject m_mediaData;
};
