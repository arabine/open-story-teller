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
    void SetActionsActive(bool enable);
    void GenerateRecentProjectsMenu(const QStringList &recents);

signals:
    void sigNew();
    void sigSave();
    void sigOpen();
    void sigClose();
    void sigExit();
    void sigDefaultDocksPosition();
    void sigOpenRecent(const QString &project);

private slots:
    void slotAbout();

private:
    QMenu *m_windowsMenu;
    QMenu *m_recentProjectsMenu;
    QAction *m_saveProjectAction;
    QAction *m_closeProjectAction;
    QList<QAction *> m_actionDockList;
};

#endif // TOOLBAR_H
