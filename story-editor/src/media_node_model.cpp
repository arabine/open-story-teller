#include "media_node_model.h"

#include "story_node_data.h"

#include <QtNodes/NodeDelegateModelRegistry>

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtWidgets/QFileDialog>
#include <QMenu>

MediaNodeModel::MediaNodeModel(StoryGraphModel &model)
    : m_model(model)
    , m_widget(new QWidget())
{
    m_ui.setupUi(m_widget);
    m_ui.image->setText("Image will appear here");
    m_ui.image->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

    QFont f = m_ui.image->font();
    f.setBold(true);
    f.setItalic(true);

    m_ui.image->setFont(f);
    m_ui.image->setMinimumSize(320, 240);
    m_ui.image->installEventFilter(this);

    connect(m_ui.spinBox, &QSpinBox::valueChanged, [&](int i) {
        bool addAction = false;
        if (m_ports < i) {
            addAction = true;
        }

        if (addAction) {
            m_model.addPort(getNodeId(), PortType::Out, m_ports - 1);
        } else {
            m_model.removePort(getNodeId(), PortType::Out, m_ports - 1);
        }

        m_ports = i;
        Q_EMIT m_model.nodeUpdated(getNodeId());
    });

    connect(m_ui.selectImageButton, &QPushButton::clicked, [&](bool enable) {
        emit m_model.sigChooseFile(getNodeId());
    });
}

QJsonObject MediaNodeModel::save() const
{
    QJsonObject obj = NodeDelegateModel::save();

    // Merge two objects
    QVariantMap map = obj.toVariantMap();
    map.insert(m_mediaData);

    return QJsonObject::fromVariantMap(map);
}

void MediaNodeModel::load(const QJsonObject &mediaData)
{
    m_mediaData = mediaData.toVariantMap();

    // Display loaded image
    QString imagePath = m_mediaData["image"].toString();

    if (!imagePath.isEmpty())
    {
        setImage(imagePath);
    }
}

void MediaNodeModel::setImage(const QString &imagePath)
{
    QPixmap pix(imagePath);

    if (pix.isNull())
    {
        std::cout << "!!!!!!! " << m_mediaData["image"].toString().toStdString() << std::endl;
    }

    int w = m_ui.image->width();
    int h = m_ui.image->height();
    pix.scaled(w, h, Qt::KeepAspectRatio);
    m_ui.image->setPixmap(pix);
}

void MediaNodeModel::setInternalData(const QVariant &value)
{
    QJsonObject obj = value.toJsonObject();
    if (obj.contains("image")) {
        setImage(obj.value("image").toString());
    }

    // Merge new data into local object
    m_mediaData.insert(obj.toVariantMap());
}

unsigned int MediaNodeModel::nPorts(PortType portType) const
{
    unsigned int result = 1;

    switch (portType) {
    case PortType::In:
        result = 1;
        break;

    case PortType::Out:
        result = m_ports;

    default:
        break;
    }

    return result;
}

bool MediaNodeModel::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_ui.image) {
        int w = m_ui.image->width();
        int h = m_ui.image->height();

        if (event->type() == QEvent::Resize) {
            auto d = std::dynamic_pointer_cast<StoryNodeData>(m_nodeData);
            if (d) {
                //_label->setPixmap(d->pixmap().scaled(w, h, Qt::KeepAspectRatio));
            }
        }
    }

    return false;
}

NodeDataType MediaNodeModel::dataType(PortType const, PortIndex const) const
{
    return StoryNodeData().type();
}

std::shared_ptr<NodeData> MediaNodeModel::outData(PortIndex)
{
    return m_nodeData;
}

void MediaNodeModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex const)
{
    m_nodeData = nodeData;

    if (m_nodeData) {
        auto d = std::dynamic_pointer_cast<StoryNodeData>(m_nodeData);

        int w = m_ui.image->width();
        int h = m_ui.image->height();

     //   _label->setPixmap(d->pixmap().scaled(w, h, Qt::KeepAspectRatio));
    } else {
    //    _label->setPixmap(QPixmap());
    }

    Q_EMIT dataUpdated(0);
}
