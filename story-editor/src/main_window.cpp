

#include <QAction>
#include <QScreen>
#include <QtWidgets/QApplication>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QJsonArray>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QStatusBar>
#include <QLineEdit>
#include <QWidgetAction>
#include <QTreeWidget>
#include <QHeaderView>
#include <QSettings>
#include <QtDebug>
#include <QStandardPaths>
#include <QDir>
#include <QOpenGLWidget>

#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/ConnectionStyle>
#include <QtNodes/GraphicsView>
#include <QtNodes/StyleCollection>
#include <QtNodes/Definitions>

#include <QtNodes/UndoCommands>

#include "main_window.h"
#include "media_node_model.h"

using QtNodes::CreateCommand;
using QtNodes::BasicGraphicsScene;
using QtNodes::ConnectionStyle;
using QtNodes::GraphicsView;
using QtNodes::NodeRole;
using QtNodes::StyleCollection;

int nodeX = 0.0;

typedef void (*message_output_t)(QtMsgType , const QMessageLogContext &, const QString &);

MainWindow::MainWindow()
    : m_model(m_project)
    , m_scene(m_model)
    , m_settings("OpenStoryTeller", "OpenStoryTellerEditor")
{
    m_project.Clear();

    m_scene.setDropShadowEffect(false);
    m_scene.nodeGeometry().setMarginsRatio(0.02);

    m_view = new GraphicsView(&m_scene);
    m_view->setScaleRange(0, 0);
    m_view->setViewport(new QOpenGLWidget());
    setCentralWidget(m_view);

    m_toolbar = new ToolBar();
    m_toolbar->createActions(menuBar());
    addToolBar(Qt::LeftToolBarArea, m_toolbar);
    m_toolbar->setVisible(true);

    connect(m_toolbar, &ToolBar::sigDefaultDocksPosition, this, &MainWindow::slotDefaultDocksPosition);

    createStatusBar();

    m_logDock = new LogDock();
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_logDock);
    m_toolbar->AddDockToMenu(m_logDock->toggleViewAction());

    m_ostHmiDock = new OstHmiDock();
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_ostHmiDock);
    m_toolbar->AddDockToMenu(m_ostHmiDock->toggleViewAction());

    connect(m_ostHmiDock, &OstHmiDock::sigOkButton, [=]() {
        QCoreApplication::postEvent(this, new VmEvent(VmEvent::evOkButton));
    });

    m_resourcesDock = new ResourcesDock(m_project);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_resourcesDock);
    m_toolbar->AddDockToMenu(m_resourcesDock->toggleViewAction());

    m_scriptEditorDock = new ScriptEditorDock();
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, m_scriptEditorDock);
    m_toolbar->AddDockToMenu(m_scriptEditorDock->toggleViewAction());

    m_vmDock = new VmDock(m_assembler);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_vmDock);
    m_toolbar->AddDockToMenu(m_vmDock->toggleViewAction());

    connect(m_vmDock, &VmDock::sigCompile, [=]() {
        m_resourcesDock->SaveToProject();
        m_scriptEditorDock->setScript(m_project.Compile());
    });

    connect(m_vmDock, &VmDock::sigStepInstruction, [=]() {
        QCoreApplication::postEvent(this, new VmEvent(VmEvent::evStep));
    });

    connect(m_vmDock, &VmDock::sigBuild, [=]() {
        buildScript();
    });

    m_ramView = new MemoryViewDock("RamViewDock", "RAM");
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_ramView);
    m_toolbar->AddDockToMenu(m_ramView->toggleViewAction());

    m_romView = new MemoryViewDock("RomViewDock", "ROM");
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_romView);
    m_toolbar->AddDockToMenu(m_romView->toggleViewAction());

    tabifyDockWidget(m_vmDock, m_romView);
    tabifyDockWidget(m_romView, m_ramView);

    m_chooseFileDialog = new QDialog(this);
    m_chooseFileUi.setupUi(m_chooseFileDialog);
    m_chooseFileDialog->close();

    connect(&m_model, &StoryGraphModel::sigChooseFile, [&](NodeId id) {
        m_chooseFileUi.tableView->setModel(&m_resourcesDock->getModel());
        m_chooseFileDialog->exec();

        // Get the file choosen
        QModelIndexList selection = m_chooseFileUi.tableView->selectionModel()->selectedRows();

        if (selection.count() > 0)
        {
            // Take first
            QModelIndex index = selection.at(0);
            Resource res;
            if (m_project.GetResourceAt(index.row(), res))
            {
                QJsonObject obj;
                obj["image"] = res.file.c_str();
                m_model.setNodeData(id, NodeRole::InternalData, obj.toVariantMap());
            }
        }
    });

    m_newProjectDialog = new NewProjectDialog(this);
    m_newProjectDialog->hide();

    // TODO: merge both
    m_model.registerNode<MediaNodeModel>("MediaNode");
    m_model.addModel("MediaNode", "Story Teller");

//    m_project.Load("packs/BE8949F60D854F54828419A1BDAED36A/pack.json");

//    DisplayNode(m_project.m_tree, QtNodes::InvalidNodeId);

    // VM Initialize

    m_chip32_ctx.stack_size = 512;

    m_chip32_ctx.rom.mem = m_rom_data;
    m_chip32_ctx.rom.addr = 0;
    m_chip32_ctx.rom.size = sizeof(m_rom_data);

    m_chip32_ctx.ram.mem = m_ram_data;
    m_chip32_ctx.ram.addr = sizeof(m_rom_data);
    m_chip32_ctx.ram.size = sizeof(m_ram_data);


    Callback<uint8_t(uint8_t)>::func = std::bind(&MainWindow::Syscall, this, std::placeholders::_1);
    m_chip32_ctx.syscall = static_cast<syscall_t>(Callback<uint8_t(uint8_t)>::callback);

    connect(m_toolbar, &ToolBar::sigNew, this, [&]() {
        NewProject();
    });

    connect(m_toolbar, &ToolBar::sigSave, this, [=]() {
        SaveProject();
    });

    connect(m_toolbar, &ToolBar::sigOpen, this, [&]() {
       OpenProjectDialog();
    });

    connect(m_toolbar, &ToolBar::sigClose, this, [&]() {
        CloseProject();
    });

    connect(m_toolbar, &ToolBar::sigExit, this, [&]() {
        ExitProgram();
    });

    connect(m_toolbar, &ToolBar::sigOpenRecent, this, [&](const QString &recent) {
        CloseProject();
        OpenProject(recent);
    });

    // Install event handler now that everythin is initialized
    Callback<void(QtMsgType , const QMessageLogContext &, const QString &)>::func = std::bind(&MainWindow::MessageOutput, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto cb = static_cast<message_output_t>(Callback<void(QtMsgType , const QMessageLogContext &, const QString &)>::callback);


    qInstallMessageHandler(cb);

    readSettings();
    qDebug() << "Settings location: " << m_settings.fileName();
    qDebug() << "Welcome to StoryTeller Editor";

    CloseProject();

//    QMetaObject::invokeMethod(this, "slotWelcome", Qt::QueuedConnection);
}

void MainWindow::slotDefaultDocksPosition()
{
    m_settings.clear();
    m_settings.sync();
}

void MainWindow::slotWelcome()
{
    QMessageBox msgBox;
    msgBox.setText("Welcome to OpenStoryTeller Editor.\nCreate a new project or open an existing one.");
    msgBox.exec();
}

void MainWindow::ExitProgram()
{
    // FIXME: warn if project not saved
}

void MainWindow::RefreshProjectInformation()
{
    setWindowTitle(QString("StoryTeller Editor - ") + m_project.GetProjectFilePath().c_str());
}

void MainWindow::MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    m_logDock->Append(type, context, msg);
}

void MainWindow::readSettings()
{
    m_settings.setDefaultFormat(QSettings::IniFormat);
    m_settings.setPath(QSettings::IniFormat, QSettings::UserScope, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    restoreGeometry(m_settings.value("MainWindow/geometry").toByteArray());
    restoreState(m_settings.value("MainWindow/windowState").toByteArray());

    // Restore recent projects list
    m_recentProjects = m_settings.value("RecentProjects").toStringList();
    m_toolbar->GenerateRecentProjectsMenu(m_recentProjects);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings.setValue("MainWindow/geometry", saveGeometry());
    m_settings.setValue("MainWindow/windowState", saveState());

    // Memorize recent projects list
    m_settings.setValue("RecentProjects", m_recentProjects);

    QMainWindow::closeEvent(event);
}

void MainWindow::DisplayNode(StoryNode *m_tree, QtNodes::NodeId parentId)
{
    QJsonObject nodeInternalData;

    if (m_tree->image >= 0)
    {
        std::string imageFile = ""; // FIXME m_project.GetWorkingDir() + "/rf/" + m_project.m_images[m_tree->image].file + ".bmp";
        std::cout << "Node image: " << imageFile << std::endl;
        nodeInternalData["image"] = imageFile.c_str();
    }
    else
    {
        nodeInternalData["image"] = "";
    }

    if (m_tree->children.size() > 1)
    {
        auto delegate = m_model.createNode("MediaNode");
        QtNodes::NodeId id = m_model.addNode(delegate);
        m_model.setNodeData(id, NodeRole::Position, QPointF(nodeX, 0));
        delegate->load(nodeInternalData);
        nodeX += 100;

        if ((id != QtNodes::InvalidNodeId) && (parentId != QtNodes::InvalidNodeId))
        {
            m_model.addConnection(QtNodes::ConnectionId{parentId, 0, id, 0});
        }

        for (int child_n = 0; child_n < m_tree->children.size(); child_n++)
        {
            DisplayNode(m_tree->children[child_n], id);
        }
    }
    else
    {
        auto delegate = std::make_shared<MediaNodeModel>(m_model);
        QtNodes::NodeId id = m_model.addNode(delegate);
        m_model.setNodeData(id, NodeRole::Position, QPointF(nodeX, 0));
        //        m_model.setNodeData(id, NodeRole::InternalData, nodeInternalData);
        delegate->load(nodeInternalData);

        nodeX += 100;

        if ((id != QtNodes::InvalidNodeId) && (parentId != QtNodes::InvalidNodeId))
        {
            m_model.addConnection(QtNodes::ConnectionId{parentId, 0, id, 0});
        }

        if (m_tree->children.size() == 1)
        {
            DisplayNode(m_tree->children[0], id);
        }
    }
}


void MainWindow::highlightNextLine()
{
    uint32_t pcVal = m_chip32_ctx.registers[PC];

    // On recherche quelle est la ligne qui possède une instruction à cette adresse
    std::vector<Chip32::Instr>::const_iterator ptr = m_assembler.Begin();
    for (; ptr != m_assembler.End(); ++ptr)
    {
        if ((ptr->addr == pcVal) && ptr->isRomCode())
        {
            break;
        }
    }

    if (ptr != m_assembler.End())
    {
        m_dbg.line = (ptr->line - 1);
        m_scriptEditorDock->HighlightLine(m_dbg.line);
    }
    else
    {
        // Not found
        qDebug() << "Reached end or instruction not found: " << m_dbg.line;
    }
}

void MainWindow::updateAll()
{
    m_vmDock->updateRegistersView(m_chip32_ctx);
    highlightNextLine();
    // Refresh RAM content
    m_ramView->SetMemory(m_ram_data, m_chip32_ctx.ram.size);
}

QString MainWindow::GetFileName(uint32_t addr)
{
    char strBuf[100];
    bool isRam = addr & 0x80000000;
    addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
    if (isRam) {
        strcpy(&strBuf[0], (const char *)&m_chip32_ctx.ram.mem[addr]);
    } else {
        strcpy(&strBuf[0], (const char *)&m_chip32_ctx.rom.mem[addr]);
    }
    return QString(strBuf);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == VmEvent::evStep)
    {
        VmEvent *myEvent = static_cast<VmEvent *>(event);

        if (m_dbg.run_result == VM_OK)
        {
            stepInstruction();
        }

        return true;
    }
    else if (event->type() == VmEvent::evOkButton)
    {
        VmEvent *myEvent = static_cast<VmEvent *>(event);

        if (m_dbg.run_result == VM_WAIT_EVENT)
        {
            // Result event is in R1
            m_chip32_ctx.registers[R1] = 0x01;
            stepInstruction();
        }

    }

    // false means it should be send to target also. as in , we dont remove it.
    // if you return true , you will take the event and widget never sees it so be carefull with that.
    return QMainWindow::event(event);
}

uint8_t MainWindow::Syscall(uint8_t code)
{
    uint8_t retCode = 0;
    qDebug() << "SYSCALL: " << (int)code;

    // Media
    if (code == 1)
    {
        // image file name address is in R0
        QString imageFile = GetFileName(m_chip32_ctx.registers[R0]);
        // sound file name address is in R1
        QString soundFile = GetFileName(m_chip32_ctx.registers[R1]);

        qDebug() << "Image: " << imageFile << ", Sound: " << soundFile;
        m_ostHmiDock->SetImage(imageFile);
    }
    // WAIT EVENT bits:
    // 0: block
    // 1: OK button
    // 2: home button
    // 3: pause button
    // 4: rotary left
    // 5: rotary right
    else if (code == 2)
    {
        // Event mask is located in R0
        // optional timeout is located in R1
        // if timeout is set to zero, wait for infinite and beyond
        retCode = 1; // set the VM in pause
    }


    return retCode;
}

void MainWindow::stepInstruction()
{    
    m_dbg.run_result = chip32_step(&m_chip32_ctx);
    updateAll();
}

void MainWindow::buildScript()
{
    if (m_assembler.Parse(m_scriptEditorDock->getScript().toStdString()) == true )
    {
        if (m_assembler.BuildBinary(m_program, m_result) == true)
        {
            m_result.Print();

            // Update ROM memory
            std::copy(m_program.begin(), m_program.end(), m_rom_data);
            m_ramView->SetMemory(m_ram_data, sizeof(m_ram_data));
            m_romView->SetMemory(m_rom_data, m_program.size());
            chip32_initialize(&m_chip32_ctx);
            m_dbg.run_result = VM_OK;
            updateAll();
            DebugContext::DumpCodeAssembler(m_assembler);
        }
        else
        {
            qCritical() << m_assembler.GetLastError().c_str();
        }
    }
    else
    {
        qCritical() << m_assembler.GetLastError().c_str();
    }
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::NewProject()
{
    m_newProjectDialog->Initialize();
    if (m_newProjectDialog->exec() == QDialog::Accepted)
    {
        m_project.Initialize(m_newProjectDialog->GetProjectFileName().toStdString());
        QSize s = m_newProjectDialog->GetDisplayFormat();
        m_project.SetDisplayFormat(s.width(), s.height());
        m_project.SetImageFormat(m_newProjectDialog->GetImageFormat());
        m_project.SetSoundFormat(m_newProjectDialog->GetSoundFormat());
        m_project.SetName(m_newProjectDialog->GetProjectName().toStdString());
        m_project.SetUuid(QUuid::createUuid().toString().toStdString());

        SaveProject();
        EnableProject();
    }
}

void MainWindow::CloseProject()
{
    m_project.Clear();

    m_model.Clear();

    m_ostHmiDock->Close();
    m_resourcesDock->Close();
    m_scriptEditorDock->Close();
    m_vmDock->Close();
    m_ramView->Close();
    m_romView->Close();
    m_logDock->Close();

    m_toolbar->SetActionsActive(false);
    m_view->setEnabled(false);
}

void MainWindow::EnableProject()
{
    // Add to recent if not exists
    if (!m_recentProjects.contains(m_project.GetProjectFilePath().c_str()))
    {
        m_recentProjects.push_front(m_project.GetProjectFilePath().c_str());
        // Limit to 10 recent projects
        if (m_recentProjects.size() > 10) {
            m_recentProjects.pop_back();
        }
        m_toolbar->GenerateRecentProjectsMenu(m_recentProjects);
    }

    m_ostHmiDock->Open();
    m_resourcesDock->Open();
    m_scriptEditorDock->Open();
    m_vmDock->Open();
    m_ramView->Open();
    m_romView->Open();
    m_logDock->Open();

    m_toolbar->SetActionsActive(true);
    m_view->setEnabled(true);
}

void MainWindow::OpenProjectDialog()
{
    QFileDialog dialog(this);

    dialog.setNameFilter(tr("StoryEditor Project (project.json)"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(QDir::homePath());

    if (dialog.exec())
    {
        QStringList fileNames = dialog.selectedFiles();

        if (fileNames.size() == 1)
        {
            OpenProject(fileNames.at(0));
        }
    }
}

void MainWindow::OpenProject(const QString &filePath)
{
    bool success = false;
    QString errorMsg = tr("General error");

    m_project.Initialize(filePath.toStdString());

    nlohmann::json model;

    if (m_project.Load(filePath.toStdString(), model))
    {
        m_model.Load(model);
        EnableProject();
    }
    else
    {
        qWarning() << errorMsg;
        QMessageBox::critical(this, tr("Open project error"), errorMsg);
    }
}


void MainWindow::SaveProject()
{
 //   QJsonObject jsonModel = m_model.save();


    statusBar()->showMessage(tr("Saved '%1'").arg(m_project.GetProjectFilePath().c_str()), 2000);
}


