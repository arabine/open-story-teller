#pragma once

#include <string>
#include <vector>
#include <ranges>

#include "i_story_db.h"

class StoryDb : public IStoryDb
{
public:
    static const int cCommercialStore = 0;
    static const int cCommunityStore = 1;

    StoryDb();
    ~StoryDb();

    bool FindStory(const std::string &uuid, Info &info) override;
    void AddStory(IStoryDb::Info &info, int origin) override;

    void Clear() {
        m_store.clear();
        m_commercialStore.clear();
    }

    void ClearCommercial() {
        m_commercialStore.clear();
    }

    void ClearCommunity() {
        m_store.clear();
    }
    
    auto CommercialDbView() const {
        return std::ranges::views::all(m_commercialStore);
    }

    auto CommunityDbView() const {
        return std::ranges::views::all(m_store);
    }

private:
    std::vector<IStoryDb::Info> m_store;
    std::vector<IStoryDb::Info> m_commercialStore;
};
