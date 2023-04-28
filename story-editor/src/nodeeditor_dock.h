#ifndef NODEEDITORDOCK_H
#define NODEEDITORDOCK_H

#include "dock_widget_base.h"
#include <QtNodes/GraphicsView>
#include <QtNodes/BasicGraphicsScene>
using QtNodes::BasicGraphicsScene;
using QtNodes::GraphicsView;


class NodeEditorDock : public DockWidgetBase
{
public:
    NodeEditorDock(BasicGraphicsScene *scene);

private:
    GraphicsView *m_view{nullptr};
};

#endif // NODEEDITORDOCK_H
