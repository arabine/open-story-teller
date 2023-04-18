#include "toolbar.h"
#include "qmenubar.h"

ToolBar::ToolBar()
{
    setObjectName("MainToolBar");
}

void ToolBar::createActions(QMenuBar *menuBar)
{
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));

    //    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    //    QAction *newLetterAct = new QAction(newIcon, tr("&New Letter"), this);
    //    newLetterAct->setShortcuts(QKeySequence::New);
    //    newLetterAct->setStatusTip(tr("Create a new form letter"));
    //    connect(newLetterAct, &QAction::triggered, this, &MainWindow::newLetter);
    //    fileMenu->addAction(newLetterAct);
    //    fileToolBar->addAction(newLetterAct);

    // ------------ Save
    {
        const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/assets/floppy.svg"));
        QAction *saveAct = new QAction(saveIcon, tr("&Save..."), this);
        saveAct->setShortcuts(QKeySequence::Save);
        saveAct->setStatusTip(tr("Save the current project"));
        connect(saveAct, &QAction::triggered, this, &ToolBar::sigSave);
        fileMenu->addAction(saveAct);
        addAction(saveAct);
    }
    // ------------ Open
    {
        const QIcon saveIcon = QIcon::fromTheme("document-open", QIcon(":/assets/folder-open.svg"));
        QAction *openAct = new QAction(saveIcon, tr("&Open..."), this);
        openAct->setShortcuts(QKeySequence::Open);
        openAct->setStatusTip(tr("Open the current project"));
        connect(openAct, &QAction::triggered, this, &ToolBar::sigOpen);
        fileMenu->addAction(openAct);
        addAction(openAct);
    }
    /*
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/images/print.png"));
    QAction *printAct = new QAction(printIcon, tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the current form letter"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);
    fileMenu->addAction(printAct);
    fileToolBar->addAction(printAct);
*/
    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    //    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    //    QToolBar *editToolBar = addToolBar(tr("Edit"));
    //    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png"));
    //    QAction *undoAct = new QAction(undoIcon, tr("&Undo"), this);
    //    undoAct->setShortcuts(QKeySequence::Undo);
    //    undoAct->setStatusTip(tr("Undo the last editing action"));
    //    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);
    //    editMenu->addAction(undoAct);
    //    editToolBar->addAction(undoAct);

    //    viewMenu = menuBar()->addMenu(tr("&View"));

    menuBar->addSeparator();

    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &ToolBar::sigAbout);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}
