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
    void sigRun();

private slots:
    void slotAbout();

private:
    QMenu *m_windowsMenu{nullptr};
    QMenu *m_recentProjectsMenu{nullptr};
    QAction *m_saveProjectAction{nullptr};
    QAction *m_closeProjectAction{nullptr};
    QAction *m_runAction{nullptr};
    QList<QAction *> m_actionDockList;
};

#endif // TOOLBAR_H
