#pragma once


#include "base_node.h"
#include "connection.h"
#include "function_entry_node.h"

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

struct PathTree {
    std::shared_ptr<ASTNode> root;
    std::vector<std::shared_ptr<Connection>> connections;
    bool isExecutionPath;  // true for main flow, false for input paths
};

class ASTBuilder {
public:

    ASTBuilder(const std::vector<std::shared_ptr<BaseNode>>& nodes,
               const std::vector<std::shared_ptr<Connection>>& connections)
        : m_nodes(nodes), m_connections(connections) {}

    std::vector<PathTree> BuildAST() {
        // Create node map for quick lookups
        std::unordered_map<std::string, std::shared_ptr<BaseNode>> nodeMap;
        for (const auto& node : m_nodes) {
            nodeMap[node->GetId()] = node;
        }

        // Find all root nodes (nodes without incoming execution connections)
        std::unordered_set<std::string> hasIncomingExec;
        std::unordered_set<std::string> hasIncomingData;

        for (const auto& conn : m_connections) {
            if (conn->type == Connection::EXECUTION_LINKJ) {
                hasIncomingExec.insert(conn->inNodeId);
            } else {
                hasIncomingData.insert(conn->inNodeId);
            }
        }

        // Collect root nodes
        std::vector<std::shared_ptr<BaseNode>> execRoots;
        std::vector<std::shared_ptr<BaseNode>> dataRoots;

        for (const auto& node : m_nodes) {
            if (hasIncomingExec.find(node->GetId()) == hasIncomingExec.end()) {
                if (dynamic_cast<FunctionEntryNode*>(node.get())) {
                    execRoots.push_back(node);
                }
            }
            // Nodes that have data outputs but no data inputs are data path roots
            if (hasIncomingData.find(node->GetId()) == hasIncomingData.end()) {
                // Check if the node has any outgoing data connections
                bool hasDataOutput = false;
                for (const auto& conn : m_connections) {
                    if (conn->type == Connection::DATA_LINK && 
                        conn->outNodeId == node->GetId()) {
                        hasDataOutput = true;
                        break;
                    }
                }
                if (hasDataOutput) {
                    dataRoots.push_back(node);
                }
            }
        }

        std::vector<PathTree> pathTrees;

        // Build execution path trees
        for (const auto& root : execRoots) {
            PathTree tree;
            tree.root = std::make_shared<ASTNode>(root);
            tree.isExecutionPath = true;
            BuildExecutionPath(tree, nodeMap);
            pathTrees.push_back(tree);
        }

        // Build data path trees
        for (const auto& root : dataRoots) {
            PathTree tree;
            tree.root = std::make_shared<ASTNode>(root);
            tree.isExecutionPath = false;
            BuildDataPath(tree, nodeMap);
            pathTrees.push_back(tree);
        }

        return pathTrees;
    }

private:
    const std::vector<std::shared_ptr<BaseNode>>& m_nodes;
    const std::vector<std::shared_ptr<Connection>>& m_connections;

    void BuildExecutionPath(PathTree& tree, 
                          const std::unordered_map<std::string, 
                          std::shared_ptr<BaseNode>>& nodeMap) {
        std::queue<std::shared_ptr<ASTNode>> queue;
        queue.push(tree.root);

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();

            // Find execution connections from this node
            for (const auto& conn : m_connections) {
                if (conn->type == Connection::EXECUTION_LINKJ && 
                    conn->outNodeId == current->node->GetId()) {
                    auto targetNode = nodeMap.find(conn->inNodeId);
                    if (targetNode != nodeMap.end()) {
                        auto childNode = std::make_shared<ASTNode>(targetNode->second);
                        current->children.push_back(childNode);
                        queue.push(childNode);
                        tree.connections.push_back(conn);

                        // For each execution node, find its data inputs
                        BuildDataInputs(childNode, nodeMap);
                    }
                }
            }
        }
    }

    void BuildDataPath(PathTree& tree,
                      const std::unordered_map<std::string, 
                      std::shared_ptr<BaseNode>>& nodeMap) {
        std::queue<std::shared_ptr<ASTNode>> queue;
        queue.push(tree.root);
        std::unordered_set<std::string> visited;

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();

            if (visited.find(current->node->GetId()) != visited.end()) {
                continue;
            }
            visited.insert(current->node->GetId());

            // Find data connections from this node
            for (const auto& conn : m_connections) {
                if (conn->type == Connection::DATA_LINK && 
                    conn->outNodeId == current->node->GetId()) {
                    auto targetNode = nodeMap.find(conn->inNodeId);
                    if (targetNode != nodeMap.end()) {
                        auto childNode = std::make_shared<ASTNode>(targetNode->second);
                        current->dataOutputs[conn->outPortIndex].push_back(
                            {childNode, conn->inPortIndex});
                        queue.push(childNode);
                        tree.connections.push_back(conn);
                    }
                }
            }
        }
    }

    void BuildDataInputs(std::shared_ptr<ASTNode> node,
                        const std::unordered_map<std::string, 
                        std::shared_ptr<BaseNode>>& nodeMap) {
        for (const auto& conn : m_connections) {
            if (conn->type == Connection::DATA_LINK && 
                conn->inNodeId == node->node->GetId()) {
                auto sourceNode = nodeMap.find(conn->outNodeId);
                if (sourceNode != nodeMap.end()) {
                    auto inputNode = std::make_shared<ASTNode>(sourceNode->second);
                    node->dataInputs[conn->inPortIndex] = inputNode;
                }
            }
        }
    }
};
