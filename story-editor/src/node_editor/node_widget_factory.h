#pragma once

#include <map>
#include <set>
#include <utility>

#include "base_node_widget.h"
#include "nodes_factory.h"

class NodeWidgetFactory
{


public:
    template<class NodeType>
    struct Factory {
        static std::shared_ptr<BaseNodeWidget> create_func(IStoryManager &manager, std::shared_ptr<BaseNode> base) {
            return std::make_shared<NodeType>(manager, base);
        }
    };

    typedef std::shared_ptr<BaseNodeWidget> (*GenericCreator)(IStoryManager &manager, std::shared_ptr<BaseNode> base);
    typedef std::map<std::string, GenericCreator> Registry;
    Registry m_registry;

    template<class Derived>
    void registerNode(const std::string& key) {
        m_registry.insert(typename Registry::value_type(key, Factory<Derived>::create_func));
    }

    std::shared_ptr<BaseNodeWidget> CreateNodeWidget(const std::string& key, IStoryManager &manager, std::shared_ptr<BaseNode> base) {
        typename Registry::const_iterator i = m_registry.find(key);
        if (i == m_registry.end()) {
            throw std::invalid_argument(std::string(__PRETTY_FUNCTION__) +
                                        ": key not registered");
        }
        else
        {
            return i->second(manager, base);
        }
    }
};

