#pragma once


#include "base_node.h"
#include "connection.h"
#include "function_entry_node.h"
#include "variable_node.h"

#include <unordered_map>
#include <unordered_set>
#include <queue>

class ASTNode {
public:
    // Structure to represent a data connection target
    struct DataTarget {
        std::shared_ptr<ASTNode> node;
        unsigned int portIndex;
    };

    explicit ASTNode(std::shared_ptr<BaseNode> node) 
        : node(node) {}

    // The actual node from the visual editor
    std::shared_ptr<BaseNode> node;

    // Execution flow children (for EXECUTION_LINKJ connections)
    std::vector<std::shared_ptr<ASTNode>> children;   

    // Data inputs: port_index -> source node
    std::unordered_map<unsigned int, std::shared_ptr<ASTNode>> dataInputs;

    // Data outputs: output_port_index -> vector of (target node, target port)
    std::unordered_map<unsigned int, std::vector<DataTarget>> dataOutputs;

    bool HasChildren(std::shared_ptr<ASTNode> c) const {
        return std::find(children.begin(), children.end(), c) != children.end();
    }

    bool IsExecutionNode() const {
        return node->GetBehavior() == BaseNode::Behavior::BEHAVIOR_EXECUTION;
    }

    bool IsDataNode() const {
        return node->GetBehavior() == BaseNode::Behavior::BEHAVIOR_DATA;
    }

    // Helper methods
    bool HasDataInput(unsigned int portIndex) const {
        return dataInputs.find(portIndex) != dataInputs.end();
    }

    bool HasDataOutput(unsigned int portIndex) const {
        return dataOutputs.find(portIndex) != dataOutputs.end();
    }

    std::shared_ptr<ASTNode> GetDataInput(unsigned int portIndex) const {
        auto it = dataInputs.find(portIndex);
        if (it != dataInputs.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::vector<DataTarget>& GetDataOutputs(unsigned int portIndex) const {
        static const std::vector<DataTarget> empty;
        auto it = dataOutputs.find(portIndex);
        if (it != dataOutputs.end()) {
            return it->second;
        }
        return empty;
    }

    // Add a data input connection
    void AddDataInput(unsigned int portIndex, std::shared_ptr<ASTNode> sourceNode) {
        dataInputs[portIndex] = sourceNode;
    }

    // Add a data output connection
    void AddDataOutput(unsigned int sourcePort, std::shared_ptr<ASTNode> targetNode, unsigned int targetPort) {
        DataTarget target{targetNode, targetPort};
        dataOutputs[sourcePort].push_back(target);
    }

    // Add an execution child
    void AddChild(std::shared_ptr<ASTNode> child) {
        children.push_back(child);
    }

    // Get execution path child count
    size_t GetChildCount() const {
        return children.size();
    }

    // Get child at specific index
    std::shared_ptr<ASTNode> GetChild(size_t index) const {
        if (index < children.size()) {
            return children[index];
        }
        return nullptr;
    }

    // Utility method to check if this is a specific node type
    template<typename T>
    bool IsType() const {
        return dynamic_cast<const T*>(node.get()) != nullptr;
    }

    // Get node as specific type
    template<typename T>
    T* GetAs() const {
        return dynamic_cast<T*>(node.get());
    }

    std::string GetId() const {
        return node->GetId();
    }

    int GetWeight() const {
        return node->GetWeight();
    }

    // Debug information
    std::string GetDebugString() const {
        std::string result = "Node: " + node->GetTypeName() + " (ID: " + node->GetId() + ")\n";
        
        // Add data inputs info
        for (const auto& [port, input] : dataInputs) {
            result += "  Input port " + std::to_string(port) + 
                        " <- " + input->node->GetTypeName() + "\n";
        }

        // Add data outputs info
        for (const auto& [port, outputs] : dataOutputs) {
            for (const auto& target : outputs) {
                result += "  Output port " + std::to_string(port) + 
                            " -> " + target.node->node->GetTypeName() + 
                            " (port " + std::to_string(target.portIndex) + ")\n";
            }
        }

        // Add execution children info
        for (size_t i = 0; i < children.size(); ++i) {
            result += "  Child " + std::to_string(i) + ": " + 
                        children[i]->node->GetTypeName() + "\n";
        }

        return result;
    }
};


class ASTBuilder {
public:

    ASTBuilder(const std::vector<std::shared_ptr<BaseNode>>& nodes,
               const std::vector<std::shared_ptr<Connection>>& connections)
        : m_nodes(nodes), m_connections(connections) {}

    std::vector<std::shared_ptr<ASTNode>> BuildAST()
    {
        // Create node map for quick lookups
        std::unordered_map<std::string, std::shared_ptr<ASTNode>> nodeMap;
        for (const auto& node : m_nodes) {
            nodeMap[node->GetId()] = std::make_shared<ASTNode>(node);
        }


        // Build adjacency list for the nodes from the connections
        for (const auto& conn : m_connections) {

            // Don't add the variables nodes, as they are input data nodes.
            auto rawNode = nodeMap[conn->outNodeId].get()->node;

            if (dynamic_cast<VariableNode*>(rawNode.get())) {
                continue;
            }
            m_adjList[conn->outNodeId].push_back(conn->inNodeId);
        }

        std::vector<std::shared_ptr<ASTNode>> topologicalOrder = ApplyKahnAlgorithm(nodeMap);

        // Maintenant, on va ajouter les connexions de données
        for (const auto& conn : m_connections)
        {
            auto outNode = nodeMap[conn->outNodeId];
            auto inNode = nodeMap[conn->inNodeId];

            // Keep variables nodes as data inputs
            if (dynamic_cast<VariableNode*>(outNode->node.get()))
            {
                inNode->AddDataInput(conn->inPortIndex, outNode);
            }
        }

        // Build execution paths
        BuildExecutionPath(topologicalOrder, nodeMap);

        return topologicalOrder;

    }

private:
    const std::vector<std::shared_ptr<BaseNode>>& m_nodes;
    const std::vector<std::shared_ptr<Connection>>& m_connections;

    void BuildExecutionPath(std::vector<std::shared_ptr<ASTNode>>& tree,
                const std::unordered_map<std::string, std::shared_ptr<ASTNode>>& nodeMap)
    {
        // For each node in the tree, find its children based on the connections
        for (const auto& node : tree)
        {
            std::queue<std::shared_ptr<ASTNode>> queue;
            queue.push(node);

            while (!queue.empty()) {
                auto current = queue.front();
                queue.pop();

                // Find connections from this node
                for (const auto& conn : m_connections)
                {
                    if (conn->outNodeId == current->node->GetId())
                    {
                        auto targetNode = nodeMap.find(conn->inNodeId);
                        if (targetNode != nodeMap.end())
                        {
                            auto childNode = targetNode->second;

                            // Si le noeud n'a pas déjà cet enfant, on l'ajoute
                            if (!current->HasChildren(childNode))
                            {
                                current->children.push_back(childNode);
                                queue.push(childNode); 
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<std::shared_ptr<ASTNode>> ApplyKahnAlgorithm(const std::unordered_map<std::string, std::shared_ptr<ASTNode>> &nodeMap)
    {

        // Pour le container de la queue, on utilise un comparateur pour trier les noeuds par poids
        // Cela permet de prioriser les noeuds avec un poids plus faible
        auto compare = [](const std::shared_ptr<ASTNode>& a, const std::shared_ptr<ASTNode>& b) {
            return a->GetWeight() < b->GetWeight();
        };
        std::priority_queue<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>, decltype(compare)> queue(compare);

     //   std::queue<std::string> q;
        std::unordered_map<std::string, int> inDegree;
 
        std::vector<std::shared_ptr<ASTNode>> res;
        int visitedCount = 0;

        // Calculate indegree
        for (auto p: m_adjList)
        {
            std::string u = p.first; 

            // On  initialise à zéro si le node n'est pas dans la liste
            if (inDegree.find(u) == inDegree.end())
            {
                inDegree[u] = 0;
            }

            for (auto v: p.second)
            {
                inDegree[v]++;
            }
        }

        // insert vertices with 0 indegree in queue
        for (auto i: inDegree)
        {
            if (i.second == 0)
            {
                queue.push(nodeMap.at(i.first));
            }
        }
        
        // Process the queue
        while(!queue.empty())
        {
            auto x = queue.top();
            queue.pop();
            visitedCount++;

            res.push_back(x);

            // Reduce indegree of neighbours 
            for (auto dest: m_adjList[x->GetId()])
            {
                inDegree[dest]--;
                if (inDegree[dest] == 0)
                {
                    queue.push(nodeMap.at(dest));
                }
            }
        }

        if (visitedCount != nodeMap.size()) {
            // cout << "There exists a cycle in the graph";
          //  throw std::runtime_error("Graph has a cycle");
        }

        // Debug: print in the console all the nodes in topological order
        std::cout << "Topological order: \n\n";
        for (const auto& a : res)
        {
            std::cout << a->node->GetTypeName() << " (" << a->GetId() << ") \n";
        }

        return res;      
    }


    // Ids (UUID strings) of nodes
    std::unordered_map<std::string, std::vector<std::string>> m_adjList; 


    bool AreAllInputsVariables(const std::shared_ptr<BaseNode>& node, const std::unordered_map<std::string, std::shared_ptr<BaseNode>>& nodeMap) const
    {
        for (const auto& conn : m_connections)
        {
            if (conn->type == Connection::DATA_LINK && conn->inNodeId == node->GetId())
            {
                auto sourceNode = nodeMap.find(conn->outNodeId);
                if (sourceNode != nodeMap.end() && !dynamic_cast<VariableNode*>(sourceNode->second.get()))
                {
                    return false;
                }
            }
        }
        return true;
    }



};
