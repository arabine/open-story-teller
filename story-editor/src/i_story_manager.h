#ifndef I_STORY_MANAGER_H
#define I_STORY_MANAGER_H


#include <string>
#include <memory>
#include <list>
#include <functional>

#include "resource.h"
#include "connection.h"
#include "base_node.h"

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) {
        return func(args...);
    }
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

class IStoryManager
{
public:
    virtual ~IStoryManager() {}

    virtual void OpenProject(const std::string &uuid) = 0;
    virtual void ImportProject(const std::string &fileName, int format) = 0;
    virtual void Log(const std::string &txt, bool critical = false) = 0;
    virtual void PlaySoundFile(const std::string &fileName) = 0;
    virtual std::string BuildFullAssetsPath(const std::string_view fileName) const = 0;
    virtual void OpenFunction(const std::string &uuid, const std::string &name) = 0;

    // Resources management
    virtual std::pair<FilterIterator, FilterIterator> Images() = 0;
    virtual std::pair<FilterIterator, FilterIterator> Sounds() = 0;
    virtual std::pair<FilterIterator, FilterIterator> Resources() = 0;
    virtual void AddResource(std::shared_ptr<Resource> res) = 0;
    virtual void ClearResources() = 0;
    virtual void DeleteResource(FilterIterator &it) = 0;

    // Node interaction
    virtual void BuildNodes(bool compileonly) = 0;
    virtual void BuildCode(bool compileonly) = 0;
    virtual  std::shared_ptr<BaseNode> CreateNode(const std::string &type) = 0;
    virtual void DeleteNode(const std::string &id) = 0;
    virtual void DeleteLink(std::shared_ptr<Connection> c) = 0;
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(const std::string &nodeId) = 0;
    virtual void LoadBinaryStory(const std::string &filename) = 0;
    virtual void ToggleBreakpoint(int line) = 0;
    virtual uint32_t GetRegister(int reg) = 0;
 
    virtual void Play() = 0;
    virtual void Step() = 0;
    virtual void Run() = 0;
    virtual void Ok() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void Home() = 0;
    virtual void Next() = 0;
    virtual void Previous() = 0;
    virtual std::string VmState() const = 0;
};

#endif // I_STORY_MANAGER_H
