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


DockWidgetBase::DockWidgetBase(const QString &title, bool visibility)
    : QDockWidget(title)
    , m_visibility(visibility ? tribool::True : tribool::False)
{
    EventFilter* filter = new EventFilter( this );
    installEventFilter( filter );
    setAllowedAreas(Qt::AllDockWidgetAreas);
}

void DockWidgetBase::Close()
{
    // Memorize prefered visibility
//    m_visibility = isVisible() ? tribool::True : tribool::False;
    hide();
}

void DockWidgetBase::Open()
{
    // Restore prefered visibility
    if (m_visibility) {
        show();
    } else {
        hide();
    }
}


