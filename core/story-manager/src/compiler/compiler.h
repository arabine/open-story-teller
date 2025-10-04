#pragma once

#include "sys_lib.h"
#include "i_story_project.h"
#include "base_node.h"
#include "connection.h"
#include "function_entry_node.h"

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

        // Analyser les connexions pour établir les relations entre les nœuds
        for (const auto& connection : connections) {
            auto outNode = m_ast.nodeMap[connection->outNodeId];
            auto inNode = m_ast.nodeMap[connection->inNodeId];

            // // Utiliser les indices des ports pour établir les connexions
            // if (outNode->inputs.size() <= connection->inPortIndex) {
            //     outNode->inputs.resize(connection->inPortIndex + 1);
            // }
            // outNode->inputs[connection->inPortIndex] = inNode;

            // Gérer les sorties en utilisant connection->outPortIndex
            if (outNode->body.size() <= connection->outPortIndex) {
                outNode->body.resize(connection->outPortIndex + 1);
            }
            outNode->body[connection->outPortIndex] = inNode;
        }
    }

    void printAST(const AST& ast, const std::shared_ptr<AstNode>& node, const std::string& prefix = "", bool isLast = true) {
        // Afficher le nœud actuel
        std::cout << prefix;
        std::cout << (isLast ? "└──" : "├──");
    
        // Afficher les informations du nœud
        std::cout << "Node: " << node->node->GetTypeName() << " (" << node->node->GetId() << ")" << std::endl;
    
        // Préparer le préfixe pour les enfants
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    
        // Parcourir les entrées (inputs) du nœud
        // for (size_t i = 0; i < node->inputs.size(); ++i) {
        //     bool isLastInput = (i == node->inputs.size() - 1);
        //     printAST(ast, node->inputs[i], newPrefix, isLastInput);
        // }
    
        // Parcourir les sorties (body) du nœud
        for (size_t i = 0; i < node->body.size(); ++i) {
            bool isLastBody = (i == node->body.size() - 1);
            printAST(ast, node->body[i], newPrefix, isLastBody);
        }
    
        // Parcourir le conditionnel (condition) du nœud, s'il existe
        if (node->condition) {
            printAST(ast, node->condition, newPrefix, true);
        }
    
        // Parcourir le corps du else (elseBody) du nœud, s'il existe
        if (node->elseBody) {
            printAST(ast, node->elseBody, newPrefix, true);
        }
    }
    
    void printAST() {
        // Trouver le nœud de type "function-entry-node"
        std::shared_ptr<AstNode> entryNode;
        for (const auto& pair : m_ast.nodeMap) {
            if (pair.second->node->GetType() == "function-entry-node") {
                entryNode = pair.second;
                break;
            }
        }
    
        if (!entryNode) {
            std::cerr << "No function-entry-node found in the AST." << std::endl;
            return;
        }
    
        // Commencer l'affichage de l'arbre à partir du nœud d'entrée
        printAST(m_ast, entryNode);
    }
    
    std::string GetCode() const {
        return m_ast.assemblyCode;
    }

private:
    AST m_ast;

};
