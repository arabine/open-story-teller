#pragma once

#include <string>
#include "i_story_manager.h"
#include "base_node.h"
#include "i_script_node.h"
#include "i_story_project.h"

class MediaNode : public BaseNode
{
public:
    MediaNode(const std::string &type);

    virtual void Initialize() override;


    void SetImage(const std::string &image);
    std::string_view GetImage() const;
    void SetSound(const std::string &sound);
    std::string_view GetSound() const;
    void StoreInternalData();

private:
    std::string m_image;
    std::string m_sound;
};

