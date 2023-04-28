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
        const QIcon icon = QIcon::fromTheme("document-save", QIcon(":/assets/file-document-plus-outline.svg"));
        QAction *act = new QAction(icon, tr("&New project"), this);
        act->setShortcuts(QKeySequence::Save);
        act->setStatusTip(tr("Create a new project"));
        connect(act, &QAction::triggered, this, &ToolBar::sigNew);
        fileMenu->addAction(act);
        addAction(act);
    }

    // ------------ Save
    {
        const QIcon icon = QIcon::fromTheme("document-save", QIcon(":/assets/floppy.svg"));
        QAction *act = new QAction(icon, tr("&Save project"), this);
        act->setShortcuts(QKeySequence::Save);
        act->setStatusTip(tr("Save the current project"));
        connect(act, &QAction::triggered, this, &ToolBar::sigSave);
        fileMenu->addAction(act);
        addAction(act);
    }
    // ------------ Open
    {
        const QIcon icon = QIcon::fromTheme("document-open", QIcon(":/assets/folder-open.svg"));
        QAction *act = new QAction(icon, tr("&Open project"), this);
        act->setShortcuts(QKeySequence::Open);
        act->setStatusTip(tr("Open an existing project"));
        connect(act, &QAction::triggered, this, &ToolBar::sigOpen);
        fileMenu->addAction(act);
        addAction(act);
    }

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    menuBar->addSeparator();

    m_windowsMenu = menuBar->addMenu(tr("&Windows"));

    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &ToolBar::sigAbout);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}

void ToolBar::AddDockToMenu(QAction *action)
{
    m_windowsMenu->addAction(action);
}
