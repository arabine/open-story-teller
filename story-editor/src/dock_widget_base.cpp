#include "dock_widget_base.h"


EventFilter::EventFilter( QObject* aParent )
    : QObject( aParent )
{

}

bool EventFilter::eventFilter( QObject *obj, QEvent *event )
{
    if ( event->type() == QEvent::Close )
    {
        return true;
    }
    return QObject::eventFilter( obj, event );
}


DockWidgetBase::DockWidgetBase(const QString &title)
    : QDockWidget(title)
{
    EventFilter* filter = new EventFilter( this );
    installEventFilter( filter );
}
