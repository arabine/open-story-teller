#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <filesystem>

#include "json.hpp"
// #include "media_node.h"
#include "function_node.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "syscall_node.h"
#include "story_project.h"

static const std::string OperatorNodeUuid = "0226fdac-8f7a-47d7-8584-b23aceb712ec";
static const std::string FunctionNodeUuid = "02745f38-9b11-49fe-94b1-b2a6b78249fb";
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
        registerNode<OperatorNode>(OperatorNodeUuid, nullptr);
        registerNode<FunctionNode>(FunctionNodeUuid, nullptr);
        registerNode<VariableNode>(VariableNodeUuid, nullptr);
        registerNode<PrintNode>(PrintNodeUuid, nullptr);
        registerNode<SyscallNode>(SyscallNodeUuid, nullptr);
    }

    ~NodesFactory() = default;

    std::vector<std::string> GetNodeTypes() const { 
        std::vector<std::string> l;
        for(auto const& imap: m_registry) l.push_back(imap.first);
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

    void SetModulesRootDirectory(const std::string &rootPath) {
        m_rootPath = rootPath;
    }

    void ScanModules() {
        // This function should scan the rootPath for modules and register them
        // For now, we just log the root path
        std::cout << "Scanning modules in: " << m_rootPath << std::endl;
        // Here you would implement the logic to scan the directory and register modules

        // For example, you could read JSON files in the directory and create nodes based on their content
        // This is a placeholder for actual scanning logic
        // Example: registerNode<CustomModuleNode>("custom-module-node");
        
        // Loop through files in m_rootPath
        // and register them as nodes if they match certain criteria
        for (const auto& entry : std::filesystem::directory_iterator(m_rootPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                // Load the JSON file and register a node based on its content
                std::ifstream file(entry.path());
                nlohmann::json j;
                file >> j;

                auto p = std::make_shared<StoryProject>(m_log);
                p->ParseStoryInformation(j);

                if (p->IsModule())
                {
                    registerNode<FunctionNode>(FunctionNodeUuid, p);
                        // For now, function node use only primitives nodes
                    // FIXME: in the future, allow function node to use other function nodes
                    // Need a list of required nodes to be registered

                    // Maybe this algorithm:
                    // 1. load all function nodes
                    // 2. for each function node, check if it has a "requires" field
                    // 3. if it does, check if we have them
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
    void registerNode(const std::string& uuid, std::shared_ptr<IStoryProject> moduleInfo = nullptr) {
        info.creator = ;
        m_registry.insert(typename Registry::value_type(uuid, std::make_pair<moduleInfo, Factory<Derived>::create_func>));
    }
};
