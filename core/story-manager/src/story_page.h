#pragma once

#include <list>
#include <memory>
#include <string>

#include "i_story_page.h"
#include "i_story_project.h"
#include "base_node.h"
#include "connection.h"

class StoryPage : public IStoryPage
{

public:
    StoryPage(const std::string &uuid)
        : m_uuid(uuid)
    {
    }
    ~StoryPage() {
        m_links.clear();
        m_nodes.clear();
        std::cout << "StoryPage destructor" << std::endl;
    }

    std::string_view Uuid() const { return m_uuid; }

    void AddNode(std::shared_ptr<BaseNode> node) {
        m_nodes.push_back(node);
    }

    void AddLink(std::shared_ptr<Connection> link) {
        m_links.push_back(link);
    }

    void PrintStats() const {
        std::cout << "Nodes: " << m_nodes.size() << ", links: " << m_links.size() << std::endl;
    }

    std::pair<std::list<std::shared_ptr<BaseNode>>::iterator, std::list<std::shared_ptr<BaseNode>>::iterator> Nodes() {
        return std::make_pair(m_nodes.begin(), m_nodes.end());
    }

    std::pair<std::list<std::shared_ptr<Connection>>::iterator, std::list<std::shared_ptr<Connection>>::iterator> Links() {
        return std::make_pair(m_links.begin(), m_links.end());
    }

    void Clear() {
        m_links.clear();
        m_nodes.clear();
    }

    void Build(std::stringstream &code, IStoryProject &project)
    {
        // // First generate all constants
        // for (const auto & n : m_nodes)
        // {
        //     code << n->GenerateConstants(*this, project, OutputsCount(n->GetId())) << "\n";
        // }

        // for (const auto & n : m_nodes)
        // {
        //     code << n->Build(*this, project.GetOptions(), OutputsCount(n->GetId())) << "\n";
        // }
    }

    virtual void GetNodeConnections(std::list<std::shared_ptr<Connection>> &c, const std::string &nodeId) override
    {
        for (const auto & l : m_links)
        { 
            if (l->outNodeId == nodeId)
            {
                c.push_back(l);
            }
        }
    }

    std::string FindFirstNode() const
    {
        std::string id;

        // First node is the one without connection on its input port

        for (const auto & n : m_nodes)
        {
            bool foundConnection = false;

            for (const auto& l : m_links)
            {
                if (l->inNodeId == n->GetId())
                {
                    foundConnection = true;
                }
            }

            if (!foundConnection)
            {
                id = n->GetId();
                std::cout << "First node is: " + id << std::endl;
                break;
            }
        }

        return id;
    }

    int OutputsCount(const std::string &nodeId) const
    {
        int count = 0;
        for (const auto & l : m_links)
        { 
            if (l->outNodeId == nodeId)
            {
                count++;
            }
        }
        return count;
    }


    nlohmann::json ToJson() const
    {
        nlohmann::json model;
        model["uuid"] = m_uuid;

        nlohmann::json nodes = nlohmann::json::array();
        for (const auto & n : m_nodes)
        {
            nodes.push_back(n->ToJson());
        }

        model["nodes"] = nodes;

        // Save links
        nlohmann::json connections = nlohmann::json::array();
        for (const auto& cnx : m_links)
        {
            nlohmann::json c(*cnx);
            connections.push_back(c);
        }

        model["connections"] = connections;

        return model;
    }

    void DeleteLink(std::shared_ptr<Connection> c)
    {    
        auto it = std::find_if(m_links.begin(), 
                        m_links.end(), 
                        [&c](std::shared_ptr<Connection> const &cnx) {
            return *cnx == *c; 
        });

        if ( it != m_links.end() )
        {
            it->reset();
            m_links.erase(it);
        }
    } 

    void DeleteNode(const std::string &id)
    {

        auto it = std::find_if(m_nodes.begin(), 
                        m_nodes.end(), 
                        [&id](std::shared_ptr<BaseNode> const &n) { return n->GetId() == id; });

        if ( it != m_nodes.end() )
        {
            it->reset();
            m_nodes.erase(it);
        }
    }

private:
    std::string m_uuid;
    std::string m_name;
    std::list<std::shared_ptr<Connection>> m_links;
    std::list<std::shared_ptr<BaseNode>> m_nodes;

};
