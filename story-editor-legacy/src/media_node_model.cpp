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
    , m_widget(new StoryNodeWidgetBase())
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
        m_model.PlaySoundFile(m_soundFilePath);
    });

    m_ui.playSoundButton->setEnabled(false);

    // default model
    m_mediaData = {
        {"image", ""},
        {"sound", ""}
    };

    m_mediaData.merge_patch(StoryNodeBase::ToJson());

    QtNodes::NodeStyle _nodeStyle;

    QColor bgColor = QColor(94, 94, 94);
    _nodeStyle.GradientColor0 = bgColor;
    _nodeStyle.GradientColor1 = bgColor;
    _nodeStyle.GradientColor2 = bgColor;
    _nodeStyle.GradientColor3 = bgColor;
    _nodeStyle.NormalBoundaryColor = bgColor;
    _nodeStyle.FontColor = Qt::white;
    _nodeStyle.FontColorFaded = QColor(125, 125, 125);
    _nodeStyle.ShadowColor = QColor(20, 20, 20);
    _nodeStyle.ConnectionPointColor = QColor(125, 125, 125);
    _nodeStyle.FilledConnectionPointColor = QColor(206, 206, 206);

    _nodeStyle.SelectedBoundaryColor = QColor(20, 146, 202);
    _nodeStyle.Opacity = 1.0;
    _nodeStyle.PenWidth = 0;
    _nodeStyle.HoveredPenWidth = 2.0;
    _nodeStyle.ConnectionPointDiameter = 3.5;

    setNodeStyle(_nodeStyle);
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
    QPixmap pix(m_model.BuildFullAssetsPath(fileName));

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
        m_soundFilePath = m_model.BuildFullAssetsPath(fileName);
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
        s = StoryProject::FileToConstant(image, ".qoi");  // FIXME: Generate the extension setup in user option of output format
    }
    if (sound.size() > 0)
    {
        s += StoryProject::FileToConstant(sound, ".wav");  // FIXME: Generate the extension setup in user option of output format
    }

    int nb_out_conns = ComputeOutputConnections();
    if (nb_out_conns > 1)
    {
        // Generate choice table if needed (out ports > 1)
        std::stringstream ss;
        std::string label = ChoiceLabel();
        ss << "$" << label
           << " DC32, "
           << nb_out_conns << ", ";

        std::unordered_set<ConnectionId> conns = m_model.allConnectionIds(getNodeId());
        int i = 0;
        for (auto & c : conns)
        {
            std::stringstream ssChoice;

            // On va chercher le label d'entrée du noeud connecté à l'autre bout
            ss << m_model.GetNodeEntryLabel(c.inNodeId);
            if (i < (nb_out_conns - 1))
            {
                ss << ", ";
            }
            else
            {
                ss << "\n";
            }
            i++;
        }

        s += ss.str();
    }

    return s;
}

std::string MediaNodeModel::ChoiceLabel() const
{
    std::stringstream ss;
    ss << "mediaChoice" << std::setw(4) << std::setfill('0') << getNodeId();
    return ss.str();
}

std::string MediaNodeModel::Build()
{
    std::stringstream ss;
    int nb_out_conns = ComputeOutputConnections();

    ss << R"(; ---------------------------- )"
       << GetNodeTitle()
        << " Type: "
       << (nb_out_conns == 0 ? "End" : nb_out_conns == 1 ? "Transition" : "Choice")
       << "\n";
    std::string image = StoryProject::RemoveFileExtension(m_mediaData["image"].get<std::string>());
    std::string sound = StoryProject::RemoveFileExtension(m_mediaData["sound"].get<std::string>());

    // Le label de ce noeud est généré de la façon suivante :
    // "media" + Node ID + id du noeud parent. Si pas de noeud parent, alors rien
    ss << EntryLabel() << ":\n";

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

    // Check output connections number
    // == 0: end node        : generate halt
    // == 1: transition node : image + sound on demand, jump directly to the other node when OK
    // > 1 : choice node     : call the node choice manager

    if (nb_out_conns == 0) // End node
    {
        ss << "halt\n";
    }
    else if (nb_out_conns == 1) // Transition node
    {
        std::unordered_set<ConnectionId> conns = m_model.allConnectionIds(getNodeId());


        for (auto c : conns)
        {
            if (c.outNodeId == getNodeId())
            {
                // On place dans R0 le prochain noeud à exécuter en cas de OK
                ss << "lcons r0, "
                   << m_model.GetNodeEntryLabel(c.inNodeId) << "\n"
                   << "ret\n";
            }
        }

    }
    else // Choice node
    {
        ss << "lcons r0, $" << ChoiceLabel() << "\n"
           << "jump .media ; no return possible, so a jump is enough";
    }
    return ss.str();
}

int MediaNodeModel::ComputeOutputConnections()
{
    NodeId id = getNodeId();
    std::unordered_set<ConnectionId> conns = m_model.allConnectionIds(id);

    int nb_out_conns = 0;

    for (auto & c : conns)
    {
        if (c.outNodeId == id)
        {
            nb_out_conns++;
        }
    }
    return nb_out_conns;
}

void MediaNodeModel::SetOutPortCount(int count)
{
    m_ui.spinBox->setValue(count);
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

std::string MediaNodeModel::EntryLabel() const
{
    std::stringstream ss;
    ss << ".mediaEntry" << std::setw(4) << std::setfill('0') << getNodeId();
    return ss.str();
}

StoryNodeWidgetBase::StoryNodeWidgetBase() {
    //        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    //        setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    //        setAttribute(Qt::WA_PaintOnScreen);

    //        setAttribute(Qt::WA_TransparentForMouseEvents);

    setStyleSheet("QLabel { background-color: rgba(0,0,0,0) }; QWidget { background-color: rgba(0,0,0,0) };");
}
