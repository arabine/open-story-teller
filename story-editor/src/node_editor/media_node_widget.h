#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "gui.h"
#include "media_node.h"

class MediaNodeWidget : public BaseNodeWidget
{
public:
    MediaNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node);

    void Draw() override;

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override;
    virtual void Initialize() override;

private:
    IStoryManager &m_manager;
    std::shared_ptr<MediaNode> m_mediaNode;
    Gui::Image  m_image;

    std::string m_soundPath;
    std::string m_buttonUniqueName;
 
};
