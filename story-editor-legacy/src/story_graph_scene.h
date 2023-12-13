#ifndef STORYGRAPHSCENE_H
#define STORYGRAPHSCENE_H

#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/ConnectionStyle>
#include <QtNodes/GraphicsView>
#include <QtNodes/StyleCollection>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>

using QtNodes::BasicGraphicsScene;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::ConnectionStyle;
using QtNodes::GraphicsView;
using QtNodes::NodeRole;
using QtNodes::StyleCollection;
using QtNodes::DataFlowGraphModel;
using QtNodes::NodeDelegateModelRegistry;

#include "story_graph_model.h"

class StoryGraphScene : public BasicGraphicsScene
{
public:
    StoryGraphScene(StoryGraphModel &model);

    QMenu *createSceneMenu(QPointF const scenePos) override;

private:
    StoryGraphModel &m_model;
};


#endif // STORYGRAPHSCENE_H
