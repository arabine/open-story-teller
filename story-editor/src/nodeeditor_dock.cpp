#include "nodeeditor_dock.h"
#include <QOpenGLWidget>

NodeEditorDock::NodeEditorDock(BasicGraphicsScene *scene)
    : DockWidgetBase(tr("Node editor"))
{
    setObjectName("NodeEditorDock");

    m_view = new GraphicsView(scene);
    m_view->setScaleRange(0, 0);
    m_view->setViewport(new QOpenGLWidget());

    setAllowedAreas(Qt::AllDockWidgetAreas);
    setWidget(m_view);
}
