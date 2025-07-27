#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <filesystem>

#include "json.hpp"
// #include "media_node.h"
#include "call_function_node.h"
#include "module_node.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "syscall_node.h"
#include "story_project.h"
#include "story_primitive.h"

static const std::string OperatorNodeUuid = "0226fdac-8f7a-47d7-8584-b23aceb712ec";
static const std::string CallFunctionNodeUuid = "02745f38-9b11-49fe-94b1-b2a6b78249fb";
static const std::string VariableNodeUuid = "020cca4e-9cdc-47e7-a6a5-53e4c9152ed0";
static const std::string PrintNodeUuid = "02ee27bc-ff1d-4f94-b700-eab55052ad1c";
static const std::string SyscallNodeUuid = "02225cff-4975-400e-8130-41524d8af773";



typedef std::shared_ptr<BaseNode> (*GenericCreator)(const std::string &type);

class NodesFactory
{

public:
    NodesFactory(ILogger &log) 
        : m_log(log)
        , m_rootPath("")
    {
        // Register node types
        // registerNode<MediaNode>("media-node");
        registerNode<OperatorNode>(OperatorNodeUuid, std::make_shared<StoryPrimitive>("Operator"));
        registerNode<CallFunctionNode>(CallFunctionNodeUuid, std::make_shared<StoryPrimitive>("Call function"));
        registerNode<VariableNode>(VariableNodeUuid, std::make_shared<StoryPrimitive>("Variable"));
        registerNode<PrintNode>(PrintNodeUuid, std::make_shared<StoryPrimitive>("Print"));
        registerNode<SyscallNode>(SyscallNodeUuid, std::make_shared<StoryPrimitive>("System call"));
    }

    ~NodesFactory() = default;

    struct NodeInfo {
        std::string uuid;
        std::string name;
        std::string description;
    };

    // key: uuid, value: node name
    std::vector<NodeInfo> LitOfNodes() const { 
        std::vector<NodeInfo> l;
        for (auto const& imap : m_registry) {
            l.push_back(NodeInfo{imap.first, imap.second.first->GetName(), imap.second.first->GetDescription() });
        }
        return l;
    }

    std::vector<NodeInfo> LitOfModules() const { 
        std::vector<NodeInfo> l;
        for (auto const& imap : m_registry) {
            if (imap.second.first->IsModule()) {
                l.push_back(NodeInfo{imap.first, imap.second.first->GetName(), imap.second.first->GetDescription() });
            }
        }
        return l;
    }

    std::shared_ptr<BaseNode> CreateNode(const std::string &type)
    {
        typename Registry::const_iterator i = m_registry.find(type);
        if (i == m_registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else
        {
            auto n = i->second.second(type);
            return n;
        }
    }

    std::shared_ptr<StoryProject> GetModule(const std::string &uuid)
    {
        std::shared_ptr<StoryProject> module;

        // Scan all function nodes and find the one with that name
        for (auto n : m_registry)
        {
            if (n.first == uuid)
            {
                if (n.second.first)
                {
                    auto p = dynamic_cast<StoryProject*>(n.second.first.get());
                    if (p == nullptr)
                    {
                        throw std::runtime_error("Node is not a StoryProject");
                    }
                    module = p->shared_from_this();
                }
            }
        }

        return module;
    }

    void SaveAllModules(ResourceManager &manager)
    {
        for (const auto &entry : m_registry)
        {
            // Only modules
            auto module = std::dynamic_pointer_cast<StoryProject>(entry.second.first);
            if (module)
            {
                module->Save(manager);
            }
        }
    }

    // Creates a new empty module (StoryProject) and registers it as a module node.
    std::shared_ptr<StoryProject> NewModule(const std::string& moduleName = "Untitled Module") {
        // Create a new StoryProject with the given name
        auto module = std::make_shared<StoryProject>(m_log);
        module->New(Uuid().String(), m_rootPath);
        module->SetName(moduleName);
        module->SetTitleImage("");
        module->SetTitleSound("");
        module->SetDisplayFormat(320, 240);
        module->SetImageFormat(Resource::ImageFormat::IMG_SAME_FORMAT);
        module->SetSoundFormat(Resource::SoundFormat::SND_SAME_FORMAT);
        module->SetDescription("");
        module->SetProjectType(IStoryProject::PROJECT_TYPE_MODULE);
        
        // Register as module node if not already in registry
        registerNode<ModuleNode>(module->GetUuid(), module);

        return module;
    }

    

    void SetModulesRootDirectory(const std::string &rootPath) {
        m_rootPath = rootPath;
    }

    void ScanModules() {
        std::cout << "Scanning modules in: " << m_rootPath << std::endl;
        
        // Loop through files in m_rootPath
        // and register them as nodes if they match certain criteria
        for (const auto& entry : std::filesystem::directory_iterator(m_rootPath))
        {
            // Enter directory and look for .json files
            if (entry.is_directory())
            {
                std::cout << "Scanning directory: " << entry.path() << std::endl;
                for (const auto& subEntry : std::filesystem::directory_iterator(entry.path()))
                {
                    if (subEntry.is_regular_file() && subEntry.path().extension() == ".json")
                    {
                        std::cout << "Found module file: " << subEntry.path() << std::endl;
                        // Load the JSON file and register a node based on its content
                        std::ifstream file(subEntry.path());
                        nlohmann::json j;
                        file >> j;
                        auto p = std::make_shared<StoryProject>(m_log);
                        p->ParseStoryInformation(j);
                        if (p->IsModule())
                        {
                            registerNode<ModuleNode>(p->GetUuid(), p);
                            // For now, function node use only primitives nodes 
                            // FIXME: in the future, allow function node to use other function nodes
                            // Need a list of required nodes to be registered

                            // Maybe this algorithm:
                            // 1. load all function nodes
                            // 2. for each function node, check if it has a "requires" field
                            // 3. if it does, check if we have them
                        }
                        else
                        {
                            std::cout << "Skipping non-module project: " << p->GetName() << std::endl;
                        }
                    }
                }
            }
        }
    }

private:
    ILogger &m_log;
    std::string m_rootPath;

    template<class NodeType>
    struct Factory {
        static std::shared_ptr<BaseNode> create_func(const std::string &type) {
            return std::make_shared<NodeType>(type);
        }
    };

    // UUID is the key, and the value is a function that creates the node
    typedef std::map<std::string, std::pair<std::shared_ptr<IStoryProject>, GenericCreator>> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& uuid, std::shared_ptr<IStoryProject> moduleInfo) {
        m_registry.insert(std::make_pair(uuid, std::make_pair(moduleInfo, Factory<Derived>::create_func)));
    }
};
