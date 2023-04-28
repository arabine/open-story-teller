#ifndef DOCK_WIDGET_BASE_H
#define DOCK_WIDGET_BASE_H

#include <QDockWidget>
#include <QEvent>

class EventFilter : public QObject
{
    Q_OBJECT
public:
    EventFilter( QObject* parent );
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

class DockWidgetBase : public QDockWidget
{
    Q_OBJECT
public:
    DockWidgetBase(const QString &title);
};

#endif // DOCK_WIDGET_BASE_H
