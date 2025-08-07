#pragma once

#include "resource.h"

struct StoryOptions
{
    Resource::ImageFormat image_format{Resource::IMG_SAME_FORMAT};
    Resource::SoundFormat sound_format{Resource::SND_SAME_FORMAT};
    int display_w{320};
    int display_h{240};
};
