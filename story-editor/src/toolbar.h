#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QMenuBar>

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar();
    void createActions(QMenuBar *menuBar);

    void AddDockToMenu(QAction *action);

signals:
    void sigNew();
    void sigSave();
    void sigOpen();
    void sigAbout();

private:
    QMenu *m_windowsMenu;
};

#endif // TOOLBAR_H
