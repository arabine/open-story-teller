#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QMenuBar>
#include "dock_widget_base.h"

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar();
    void createActions(QMenuBar *menuBar);
    void AddDockToMenu(QAction *action, DockWidgetBase *dock);
    void SetActionsActive(bool enable);
    void GenerateRecentProjectsMenu(const QStringList &recents);
    QVariant GetDocksPreferences() const;
    QVariant SetDocksPreferences(const QVariant &prefs);

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
    QList<DockWidgetBase *> m_docksList;
};

#endif // TOOLBAR_H
