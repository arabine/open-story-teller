#include "toolbar.h"
#include "qmenubar.h"

ToolBar::ToolBar()
{
    setObjectName("MainToolBar");
}

void ToolBar::createActions(QMenuBar *menuBar)
{
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));

    // ------------ New
    {
        QIcon icon(":/assets/file-document-plus-outline.svg");
        QAction *act = new QAction(icon, tr("&New project"), this);
        act->setShortcuts(QKeySequence::Save);
        act->setStatusTip(tr("Create a new project"));
        connect(act, &QAction::triggered, this, &ToolBar::sigNew);
        fileMenu->addAction(act);
        addAction(act);
    }
    // ------------ Save
    {
        QIcon icon(":/assets/floppy.svg");
        m_saveProjectAction = new QAction(icon, tr("&Save project"), this);
        m_saveProjectAction->setShortcuts(QKeySequence::Save);
        m_saveProjectAction->setStatusTip(tr("Save the current project"));
        connect(m_saveProjectAction, &QAction::triggered, this, &ToolBar::sigSave);
        fileMenu->addAction(m_saveProjectAction);
        addAction(m_saveProjectAction);
    }
    // ------------ Open
    {
        QIcon icon(":/assets/folder-open-outline.svg");
        QAction *act = new QAction(icon, tr("&Open project"), this);
        act->setShortcuts(QKeySequence::Open);
        act->setStatusTip(tr("Open an existing project"));
        connect(act, &QAction::triggered, this, &ToolBar::sigOpen);
        fileMenu->addAction(act);
        addAction(act);
    }
    // ------------ Close
    {
        QIcon icon(":/assets/close-outline.svg");
        m_closeProjectAction = new QAction(icon, tr("&Close project"), this);
        m_closeProjectAction->setShortcuts(QKeySequence::Close);
        m_closeProjectAction->setStatusTip(tr("Close current project"));
        connect(m_closeProjectAction, &QAction::triggered, this, &ToolBar::sigClose);
        fileMenu->addAction(m_closeProjectAction);
        addAction(m_closeProjectAction);
    }

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &ToolBar::sigExit);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    menuBar->addSeparator();

    m_windowsMenu = menuBar->addMenu(tr("&Windows"));

    auto act = m_windowsMenu->addAction(tr("Reset docks position"));
    connect(act, &QAction::triggered, this, &ToolBar::sigDefaultDocksPosition);

    m_closeAllDocksAction = m_windowsMenu->addAction(tr("Show/Hide all docks"));
    m_closeAllDocksAction->setCheckable(true);
    connect(m_closeAllDocksAction, &QAction::triggered, this, [=] (bool checked) {
        SetAllDocks(checked);
    });

    m_windowsMenu->addSeparator();

    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &ToolBar::sigAbout);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}

void ToolBar::SetActionsActive(bool enable)
{
    for (auto d : m_actionDockList)
    {
        d->setEnabled(enable);
    }
    m_windowsMenu->setEnabled(enable);
    m_closeProjectAction->setEnabled(enable);
    m_saveProjectAction->setEnabled(enable);
}

void ToolBar::ShowAllDocks(bool enable)
{
    m_closeAllDocksAction->setEnabled(enable);
    m_closeAllDocksAction->trigger();
}

void ToolBar::SetAllDocks(bool enable)
{
    for (auto d : m_actionDockList)
    {
        d->setChecked(!enable);
        d->trigger();
    }
}

void ToolBar::AddDockToMenu(QAction *action)
{
    m_windowsMenu->addAction(action);
    m_actionDockList.push_back(action);
}
