#pragma once

#include "json.hpp"
// #include "media_node.h"
#include "function_node.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "syscall_node.h"

class NodesFactory
{

public:
    NodesFactory() {
        // Register node types
        // registerNode<MediaNode>("media-node");
        registerNode<OperatorNode>("operator-node");
        registerNode<FunctionNode>("function-node");
        registerNode<VariableNode>("variable-node");
        registerNode<PrintNode>("print-node");
        registerNode<SyscallNode>("syscall-node");
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
            auto n = i->second(type);
            return n;
        }
    }

private:

    template<class NodeType>
    struct Factory {
        static std::shared_ptr<BaseNode> create_func(const std::string &type) {
            return std::make_shared<NodeType>(type);
        }
    };

    typedef std::shared_ptr<BaseNode> (*GenericCreator)(const std::string &type);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& key) {
        m_registry.insert(typename Registry::value_type(key, Factory<Derived>::create_func));
    }
};
