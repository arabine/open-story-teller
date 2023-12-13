#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <ranges>

#include "resource.h"

class ResourceManager
{
public:

    ResourceManager()
        : m_images(filter("image"))
        , m_sounds(filter("sound"))
    {

    }

    ~ResourceManager() {

    }

    void Add(std::shared_ptr<Resource> res)
    {
        m_items.push_back(res);
        UpdateIterators();
    }

    void Clear()
    {
        m_items.clear();
        UpdateIterators();
    }


    // Fonction pour créer un itérateur de début et de fin pour les éléments filtrés
    std::pair<FilterIterator, FilterIterator> filter(const std::string &type) const {
        auto begin = std::begin(m_items);
        auto end = std::end(m_items);
        return std::make_pair(FilterIterator(begin, end, type), FilterIterator(end, end, type));
    }

    std::pair<FilterIterator, FilterIterator> Items()
    {
        return filter("");
    }

    std::pair<FilterIterator, FilterIterator> Images()
    {
        return m_images;
    }

    std::pair<FilterIterator, FilterIterator> Sounds()
    {
        return m_sounds;
    }


    /*
 void StoryProject::AppendResource(const Resource &res)
{
    m_resources.push_back(res);
}

bool StoryProject::GetResourceAt(int index, Resource &resOut)
{
    bool success = false;
    if ((index >= 0) && (index < m_resources.size()))
    {
        resOut = m_resources[index];
        success = true;
    }
    return success;
}

void StoryProject::ClearResources()
{
    m_resources.clear();
}

void StoryProject::DeleteResourceAt(int index)
{
    if ((index >= 0) && (index < m_resources.size()))
    {
        m_resources.erase(m_resources.begin() + index);
    }
}
*/


private:
    std::vector<std::shared_ptr<Resource>> m_items;
    std::pair<FilterIterator, FilterIterator> m_images;
    std::pair<FilterIterator, FilterIterator> m_sounds;

    void UpdateIterators()
    {
        m_images = filter("image");
        m_sounds = filter("sound");
    }

};

#endif // RESOURCE_MANAGER_H
