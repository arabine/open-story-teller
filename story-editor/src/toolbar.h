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
    void SetAllDocks(bool enable);
    void SetActionsActive(bool enable);
    void ShowAllDocks(bool enable);

signals:
    void sigNew();
    void sigSave();
    void sigOpen();
    void sigClose();
    void sigExit();
    void sigAbout();
    void sigDefaultDocksPosition();

private:
    QMenu *m_windowsMenu;
    QAction *m_closeAllDocksAction;
    QAction *m_saveProjectAction;
    QAction *m_closeProjectAction;
    QList<QAction *> m_actionDockList;
};

#endif // TOOLBAR_H
