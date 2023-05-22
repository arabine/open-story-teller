#include "media_node_model.h"

#include "story_node_data.h"

#include <QtNodes/NodeDelegateModelRegistry>

#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtWidgets/QFileDialog>
#include <QMenu>
#include <qdebug.h>

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

    connect(m_ui.spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int i) {

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
        emit m_model.sigChooseFile(getNodeId(), "image");
    });

    connect(m_ui.selectSoundButton, &QPushButton::clicked, [&](bool enable) {
        emit m_model.sigChooseFile(getNodeId(), "sound");
    });

    connect(m_ui.playSoundButton, &QPushButton::clicked, [&](bool enable) {
        m_model.PlaySound(m_soundFilePath);
    });

    m_ui.playSoundButton->setEnabled(false);

    // default model
    m_mediaData = {
        {"image", ""},
        {"sound", ""}
    };

    m_mediaData.merge_patch(StoryNodeBase::ToJson());
}

QString MediaNodeModel::caption() const
{
    return QString("Media Node " + QString::number(getNodeId()));
}

nlohmann::json MediaNodeModel::ToJson() const
{
    // Always start with generic
    nlohmann::json j = StoryNodeBase::ToJson();
    // Merge two objects
    j.merge_patch(m_mediaData);
    return j;
}

void MediaNodeModel::FromJson(nlohmann::json &j)
{
    setInternalData(j); // Merge with SetInternalData ?
}

void MediaNodeModel::setImage(const QString &fileName)
{
    QPixmap pix(m_model.BuildFullImagePath(fileName));

    if (!pix.isNull())
    {
        int w = m_ui.image->width();
        int h = m_ui.image->height();
        pix.scaled(w, h, Qt::KeepAspectRatio);
        m_ui.image->setPixmap(pix);
        m_ui.imageName->setText(fileName);
    }
}

void MediaNodeModel::setInternalData(const nlohmann::json &j)
{
    if (j.contains("image")) {
        setImage(j["image"].get<std::string>().c_str());
    }

    if (j.contains("sound")) {
        QString fileName = j["sound"].get<std::string>().c_str();
        m_soundFilePath = m_model.BuildFullSoundPath(fileName);
        m_ui.soundName->setText(fileName);
        m_ui.playSoundButton->setEnabled(true);
    }

    // Merge new data into local object
    m_mediaData.merge_patch(j);
}

std::string MediaNodeModel::GenerateConstants()
{
    std::string s;

    std::string image = m_mediaData["image"].get<std::string>();
    std::string sound = m_mediaData["sound"].get<std::string>();
    if (image.size() > 0)
    {
        s = StoryProject::FileToConstant(image);
    }
    if (sound.size() > 0)
    {
        s += StoryProject::FileToConstant(sound);
    }

    // FIXME: Generate choice table if needed (out ports > 1)
    std::unordered_set<ConnectionId> conns = m_model.allConnectionIds(getNodeId());

    int nb_out_ports = 0;

    for (auto & c : conns)
    {
        if (c.outNodeId > 0)
        {
            nb_out_ports++;
        }
    }

    return s;
}

std::string MediaNodeModel::Build()
{
    std::stringstream ss;

    ss << R"(; ---------------- )" << GetNodeTitle() << "\n";
    std::string image = StoryProject::RemoveFileExtension(m_mediaData["image"].get<std::string>());
    std::string sound = StoryProject::RemoveFileExtension(m_mediaData["sound"].get<std::string>());
    if (image.size() > 0)
    {
        ss << "lcons r0, $" << image  << "\n";
    }
    else
    {
        ss << "lcons r0, 0\n";
    }

    if (sound.size() > 0)
    {
        ss << "lcons r1, $" << sound  << "\n";
    }
    else
    {
        ss << "lcons r1, 0\n";
    }
    // Call the media executor (image, sound)
    ss << "syscall 1\n";

    NodeId id = getNodeId();
    std::unordered_set<ConnectionId> conns = m_model.allConnectionIds(id);

    int nb_out_ports = 0;

    for (auto & c : conns)
    {
        if (c.inNodeId == id)
        {
            nb_out_ports++;
        }
    }

    if (nb_out_ports == 0)
    {
        ss << "halt\n";
    }
    else
    {

    }

    // Check output connections number
    // == 0: end, generate halt
    // == 1: jump directly to the other node
    // > 1 : call the node choice manager

//        lcons r0, $ChoiceObject
//            jump .media ; no return possible, so a jump is enough


/*



        syscall 1
        lcons r0, $ChoiceObject
            jump .media ; no return possible, so a jump is enough
*/
    return ss.str();
}

void MediaNodeModel::SetOutPortCount(int count) {

   // m_ui.spinBox->blockSignals(true);
    m_ui.spinBox->setValue(count);
 //   m_ui.spinBox->blockSignals(true);
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
