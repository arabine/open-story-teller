#include "story_db.h"

StoryDb::StoryDb()
{
}

StoryDb::~StoryDb()
{
}

bool StoryDb::FindStory(const std::string &uuid, Info &info)
{
    for (const auto &s : m_store)
    {
        if (s.uuid == uuid)
        {
            info = s;
            return true;
        }
    }

    for (const auto &s : m_commercialStore)
    {
        if (s.uuid == uuid)
        {
            info = s;
            return true;
        }
    }

    return false;
}

void StoryDb::AddStory(IStoryDb::Info &info, int origin)
{
    if (origin == cCommercialStore)
    {
        m_commercialStore.push_back(info);
    }

    if (origin == cCommunityStore)
    {
        m_store.push_back(info);
    }

}
