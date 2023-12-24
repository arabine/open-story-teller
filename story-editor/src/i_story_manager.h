#ifndef I_STORY_MANAGER_H
#define I_STORY_MANAGER_H


#include <string>
#include <vector>
#include <memory>
#include <list>

#include "resource.h"
#include "connection.h"

class IStoryManager
{
public:
    virtual ~IStoryManager() {}

    virtual void Log(const std::string &txt, bool critical = false) = 0;
    virtual void PlaySoundFile(const std::string &fileName) = 0;
    virtual std::string BuildFullAssetsPath(const std::string &fileName) const = 0;

    // Resources management
    virtual std::pair<FilterIterator, FilterIterator> Images() = 0;
    virtual std::pair<FilterIterator, FilterIterator> Sounds() = 0;
    virtual std::pair<FilterIterator, FilterIterator> Resources() = 0;
    virtual void AddResource(std::shared_ptr<Resource> res) = 0;
    virtual void ClearResources() = 0;
    virtual void DeleteResource(FilterIterator &it) = 0;

    // Node interaction
    virtual void Build() = 0;
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(unsigned long nodeId) = 0;
    virtual std::string GetNodeEntryLabel(unsigned long nodeId) = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Next() = 0;
    virtual void Previous() = 0;


};

#endif // I_STORY_MANAGER_H
