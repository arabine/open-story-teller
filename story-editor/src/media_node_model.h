// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023-2099 Anthony Rabine <anthony@rabine.fr>

#pragma once

#include <iostream>

#include <QtCore/QObject>
#include <QtWidgets/QLabel>

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeDelegateModelRegistry>

#include "ui_media-node.h"

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

#include "story_graph_model.h"
#include "story_node_base.h"

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class MediaNodeModel : public StoryNodeBase
{
    Q_OBJECT

public:
    MediaNodeModel(StoryGraphModel &model);
    ~MediaNodeModel() = default;

public:
    QString caption() const override;

    QString name() const override { return QString("MediaNode"); }

public:
    virtual nlohmann::json ToJson() const override;
    virtual void FromJson(nlohmann::json &j) override;

    void setInternalData(const nlohmann::json &j) override;

    virtual std::string GenerateConstants() override;
    virtual std::string Build() override;

    virtual void SetOutPortCount(int count) override;

public:
    virtual QString modelName() const { return QString("MediaNode"); }

    unsigned int nPorts(PortType const portType) const override;

    NodeDataType dataType(PortType const portType, PortIndex const portIndex) const override;

    std::shared_ptr<NodeData> outData(PortIndex const port) override;

    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex const port) override;

    QWidget *embeddedWidget() override { return m_widget; }

    bool resizable() const override { return true; }

    virtual std::string EntryLabel() const override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    StoryGraphModel &m_model;

    unsigned int m_ports{1};
    QWidget *m_widget;

    QString m_soundFilePath;

    Ui::mediaNodeUi m_ui;
    std::shared_ptr<NodeData> m_nodeData;

    nlohmann::json m_mediaData;
    void setImage(const QString &fileName);
    int ComputeOutputConnections();
    std::string ChoiceLabel() const;
};
