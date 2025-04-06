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

            astNode->position = {x, y};
            x += 1; // Espacement horizontal
            if (x > 20) { // Nouvelle ligne
                x = 0;
                y += 1; // Espacement vertical
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
        for (auto& node : nodes) {
            std::string nodeType = node->GetTypeName();
            if (nodeType == "LOOP" || nodeType == "BRANCH") {
                for (auto& connection : connections) {
                    if (connection->inNodeId == node->GetId()) {
                        if (connection->inPortIndex == 0) { // Condition
                            m_ast.nodeMap[node->GetId()]->condition = m_ast.nodeMap[connection->outNodeId];
                        } else if (connection->inPortIndex == 1) { // Body
                            m_ast.nodeMap[node->GetId()]->body.push_back(m_ast.nodeMap[connection->outNodeId]);
                        } else if (nodeType == "BRANCH" && connection->inPortIndex == 2) { // ElseBody
                            // if (m_ast.nodeMap[node->GetId()]->elseBody == nullptr) {
                            //     auto dummyNode = std::make_shared<BaseNode>("DummyNode");
                            //     auto dummyAstNode = std::make_shared<AstNode>();
                            //     dummyAstNode->node = dummyNode;
                            //     m_ast.nodeMap[node->GetId()]->elseBody = dummyAstNode;
                            // }
                            m_ast.nodeMap[node->GetId()]->elseBody->body.push_back(m_ast.nodeMap[connection->outNodeId]);
                        }
                    }
                }
            }
        }
            

    }


    void displayNodeSchema() {
        // Définir une taille fixe pour la grille
        const int gridWidth = 20; // 20 cases de large
        int gridHeight = 0;
    
        // Calculer la hauteur nécessaire pour la grille
        for (const auto& astNode : m_ast.nodeMap) {
            gridHeight = std::max(gridHeight, astNode.second->position.second + 1);
        }
    
        // Créer une grille pour la console
        std::vector<std::vector<std::string>> grid(gridHeight, std::vector<std::string>(gridWidth, "   "));
    
        // Placer les nœuds dans la grille
        for (const auto& astNode : m_ast.nodeMap) {
            auto pos = astNode.second->position;
            std::string nodeName = astNode.second->node->GetTypeName();
    
            // Tronquer le nom du nœud à 3 caractères
            if (nodeName.length() > 3) {
                nodeName = nodeName.substr(0, 3);
            } else {
                // Ajouter des espaces pour centrer le nom dans la case
                nodeName.insert(nodeName.end(), 3 - nodeName.length(), ' ');
            }
    
            // Placer le nom du nœud dans la grille
            grid[pos.second][pos.first] = nodeName;
        }
    
        // Dessiner les connexions
        for (const auto& astNode : m_ast.nodeMap) {
            for (const auto& input : astNode.second->inputs) {
                auto outPos = input->position;
                auto inPos = astNode.second->position;
    
                // Dessiner une ligne entre les nœuds
                int startX = outPos.first + 1; // Commencer après le nom du nœud
                int startY = outPos.second;
                int endX = inPos.first;
                int endY = inPos.second;
    
                // Ligne horizontale
                for (int i = startX; i <= endX; ++i) {
                    if (i < gridWidth) {
                        grid[startY][i][0] = '-'; // Dessiner la ligne dans la première position de la case
                    }
                }
    
                // Ligne verticale
                for (int i = std::min(startY, endY); i <= std::max(startY, endY); ++i) {
                    if (i < gridHeight) {
                        grid[i][endX][0] = '|'; // Dessiner la ligne dans la première position de la case
                    }
                }
            }
        }
    
        // Afficher la grille avec des séparateurs verticaux
        for (const auto& row : grid) {
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << row[i];
                if (i < row.size() - 1) {
                    std::cout << "|"; // Séparateur vertical
                }
            }
            std::cout << std::endl;
        }
    }
    
    
    
    void generateAssembly() {
        std::string assemblyCode;
        std::map<std::string, std::string> labels;

        // Generate all constants
        for (const auto& astNode : m_ast.nodeMap) {
            assemblyCode += astNode.second->node->GenerateConstants();
        }

        // After the constants, the main entry point:
        assemblyCode += ".main:\n";
    
        // Générer des labels uniques pour les nœuds
        for (const auto& nodePair : m_ast.nodeMap) {
            std::string label = m_ast.generateLabel(m_ast);
            labels[nodePair.first] = label;
    
            // Générer du code pour chaque type de nœud
            std::string nodeType = nodePair.second->node->GetTypeName();
            if (nodeType == "LOOP") {
                // Générer du code pour une boucle
                assemblyCode += "  ; Loop start\n";
                // Ajouter des instructions pour la condition de la boucle
                if (nodePair.second->condition) {
                    assemblyCode += "  cmp_eq r0, " + labels[nodePair.second->condition->node->GetId()] + ", 0\n";
                    assemblyCode += "  skipz r0\n";
                    assemblyCode += "  jump " + labels[nodePair.second->body.front()->node->GetId()] + "\n";
                }
                // Ajouter des instructions pour le corps de la boucle
                for (const auto& bodyNode : nodePair.second->body) {
                    assemblyCode += "  ; Loop body\n";
                    // Ajouter des instructions pour chaque nœud du corps
                }
                assemblyCode += "  jump " + label + "\n"; // Retour au début de la boucle
            } else if (nodeType == "BRANCH") {
                // Générer du code pour un branchement
                assemblyCode += "  ; Branch start\n";
                // Ajouter des instructions pour la condition du branchement
                if (nodePair.second->condition) {
                    assemblyCode += "  cmp_eq r0, " + labels[nodePair.second->condition->node->GetId()] + ", 0\n";
                    assemblyCode += "  skipnz r0\n";
                    assemblyCode += "  jump " + labels[nodePair.second->body.front()->node->GetId()] + "\n";
                }
                // Ajouter des instructions pour le corps du branchement
                for (const auto& bodyNode : nodePair.second->body) {
                    assemblyCode += "  ; Branch body\n";
                    // Ajouter des instructions pour chaque nœud du corps
                }
                // Ajouter des instructions pour le corps du else
                if (nodePair.second->elseBody) {
                    assemblyCode += "  ; Else body\n";
                    // Ajouter des instructions pour chaque nœud du corps du else
                }
            } else {
                // Générer du code pour d'autres types de nœuds
                assemblyCode += "  ; Other node type: " + nodeType + "\n";
                assemblyCode += nodePair.second->node->GenerateAssembly();

                // Ajouter des instructions spécifiques au type de nœud
            }
        }
    
        // Ajouter le code assembleur généré à l'AST
        m_ast.assemblyCode = assemblyCode;
    }
    
    std::string GetCode() const {
        return m_ast.assemblyCode;
    }

private:
    AST m_ast;

};
