#ifndef NODEEDITORDOCK_H
#define NODEEDITORDOCK_H

#include <QDockWidget>
#include <QtNodes/GraphicsView>
#include <QtNodes/BasicGraphicsScene>
using QtNodes::BasicGraphicsScene;
using QtNodes::GraphicsView;

class NodeEditorDock : public QDockWidget
{
public:
    NodeEditorDock(BasicGraphicsScene *scene);

private:
    GraphicsView *m_view{nullptr};
};

#endif // NODEEDITORDOCK_H
