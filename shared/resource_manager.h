#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <ranges>

#include "resource.h"
#include "i_logger.h"

struct Media {
    std::string type;
    std::string format;
};



class ResourceManager
{
public:
    enum KindOfInfo {
        InfoFormat = 0, // mp3, jpg ...
        InfoType = 1, // image or sound
    };

    ResourceManager(ILogger &log)
        : m_log(log)
        , m_images(filter("image"))
        , m_sounds(filter("sound"))
    {
    }

    static std::string ExtentionInfo(std::string extension, int info_type);

    void ConvertResources(const std::filesystem::path &assetsPath, const std::filesystem::path &destAssetsPath,  Resource::ImageFormat imageFormat, Resource::SoundFormat soundFormat);

    ~ResourceManager() {

    }

    void Add(std::shared_ptr<Resource> res)
    {
        m_items.push_back(res);
        UpdateIterators();
    }

    void Delete(FilterIterator &it)
    {
        m_items.erase(it.Current());
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


private:
    ILogger &m_log;
    std::list<std::shared_ptr<Resource>> m_items;
    std::pair<FilterIterator, FilterIterator> m_images;
    std::pair<FilterIterator, FilterIterator> m_sounds;

    void UpdateIterators()
    {
        m_images = filter("image");
        m_sounds = filter("sound");
    }

};

#endif // RESOURCE_MANAGER_H
