#pragma once

#include "sys_lib.h"
#include "i_story_project.h"
#include "base_node.h"
#include "connection.h"

struct AstNode
{
    std::shared_ptr<BaseNode> node; // pointeur vers le noeud en cours

    std::vector<std::shared_ptr<AstNode>> inputs;
    std::shared_ptr<AstNode> condition; // Pour les boucles et les branchements
    std::vector<std::shared_ptr<AstNode>> body; // Pour les boucles et les branchements
    std::shared_ptr<AstNode> elseBody; // Pour les branchements

    std::pair<int, int> position;
};


// AST (Arbre de syntaxe abstraite)
struct AST {

    std::map<std::string, int> variables; // Stockage des variables (adresses mémoire)
    std::string assemblyCode;
    int labelCounter = 0;
    std::map<std::string, std::shared_ptr<AstNode>> nodeMap;

    // Fonction pour générer un label unique
    std::string generateLabel(AST& ast) {
        return "label" + std::to_string(ast.labelCounter++);
    }
};




class Compiler
{

public:
    Compiler() = default;
    ~Compiler() = default;


    static std::string FileToConstant(const std::string &FileName, const std::string &extension, IStoryProject &project);


    // Fonction pour construire l'AST à partir des nœuds et des connexions
    void buildAST(std::vector<std::shared_ptr<BaseNode>>& nodes, std::vector<std::shared_ptr<Connection>>& connections) {

        m_ast.nodeMap.clear();

        // Créer une map pour accéder aux nœuds par ID
        int x = 0, y = 0;
        for (auto& node : nodes) {
            auto astNode = std::make_shared<AstNode>();

            astNode->position = {0, 0};
            x += 10; // Espacement horizontal
            if (x > 50) { // Nouvelle ligne
                x = 0;
                y += 5; // Espacement vertical
            }

            astNode->node = node;
            m_ast.nodeMap[node->GetId()] = astNode;
        }

        // Construire les connexions entre les nœuds
        for (auto& connection : connections) {
            std::shared_ptr<AstNode> outNode = m_ast.nodeMap[connection->outNodeId];
            std::shared_ptr<AstNode> inNode = m_ast.nodeMap[connection->inNodeId];
            inNode->inputs.push_back(outNode); // Ajoute le noeud de sortie en entrée du noeud d'entrée
        }

        // Gestion des conditions et des corps de boucles/branches
        /*
        for(auto& node : nodes) {
            if (node.type == Node::Type::LOOP || node.type == Node::Type::BRANCH){
                for (auto& connection : connections) {
                    if (connection.inNodeId == node.id && connection.inPortIndex == 0) { // Condition
                        node.condition = nodeMap[connection.outNodeId];
                    }
                    if (connection.inNodeId == node.id && connection.inPortIndex == 1) { // Body
                        node.body.push_back(nodeMap[connection.outNodeId]);
                    }
                    if (node.type == Node::Type::BRANCH && connection.inNodeId == node.id && connection.inPortIndex == 2) { // ElseBody
                        if(node.elseBody == nullptr) {
                            node.elseBody = new Node(Node::Type::VARIABLE, "dummy"); // Créer un noeud dummy juste pour l'elsebody.
                            node.elseBody->body.push_back(nodeMap[connection.outNodeId]);
                        } else {
                            node.elseBody->body.push_back(nodeMap[connection.outNodeId]);
                        }
                    }
                }
            }
        }
            */

    }


    // Fonction pour afficher le schéma des nœuds
    void displayNodeSchema() {
        
        // Afficher les nœuds
        for (auto& astNode : m_ast.nodeMap) {
            auto pos = astNode.second->position;
            std::cout << "\033[" << pos.second + 1 << ";" << pos.first + 1 << "H"; // Déplacer le curseur
            std::cout << "[" << astNode.second->node->GetTypeName() << "]";
        }
/*
        // Afficher les connexions
        for (auto& connection : connections) {
            auto outPos = nodePositions[connection.outNodeId];
            auto inPos = nodePositions[connection.inNodeId];

            // Dessiner une ligne entre les nœuds (simplifié)
            int startX = outPos.first + 10;
            int startY = outPos.second + 1;
            int endX = inPos.first;
            int endY = inPos.second + 1;

            if (startY == endY) { // Ligne horizontale
                for (int i = startX; i < endX; ++i) {
                    std::cout << "\033[" << startY << ";" << i + 1 << "H-";
                }
            } else { // Ligne verticale + horizontale
                for (int i = startX; i < startX + (endX - startX) / 2; ++i) {
                    std::cout << "\033[" << startY << ";" << i + 1 << "H-";
                }
                for (int i = std::min(startY, endY); i < std::max(startY, endY); ++i) {
                    std::cout << "\033[" << i + 1 << ";" << startX + (endX - startX) / 2 + 1 << "H|";
                }
                for (int i = startX + (endX - startX) / 2 + 1; i < endX; ++i) {
                    std::cout << "\033[" << endY << ";" << i + 1 << "H-";
                }
            }
            std::cout << "\033[" << endY << ";" << endX << "H>";
        }

        std::cout << "\033[" << 100 << ";" << 1 << "H"; // Déplacer le curseur en bas

        */
    }

private:
    AST m_ast;

};
