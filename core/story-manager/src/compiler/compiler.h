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
    
    void generateAssembly() {
        std::ostringstream assemblyCode;
        std::map<std::string, std::string> labels;

        // Trouver le nœud de type "function-entry-node"
        std::shared_ptr<AstNode> entryNode;
        for (const auto& pair : m_ast.nodeMap) {
            if (pair.second->node->GetType() == "function-entry-node") {
                entryNode = pair.second;
                break;
            }
        }

        if (!entryNode) {
            throw std::runtime_error("No function-entry-node found in the AST.");
        }

        // Fonction récursive pour générer le code assembleur
        std::function<void(const std::shared_ptr<AstNode>&)> generateCode = [&](const std::shared_ptr<AstNode>& node) {
            if (node->node->GetType() == "print") {
                // Générer le code pour un nœud de type "print"
                assemblyCode << "PRINT ";
                // Supposons que le nœud print a une entrée qui est une variable
                if (!node->inputs.empty()) {
                    assemblyCode << node->inputs[0]->node->GetTitle();
                }
                assemblyCode << std::endl;
            } else if (node->node->GetType() == "variable-node") {
                // Générer le code pour un nœud de type "variable-node"
                assemblyCode << "LOAD " << node->node->GetTitle() << " INTO REGISTER" << std::endl;
            } else if (node->node->GetType() == "branch-node") {
                // Générer le code pour un nœud de type "branch-node"
                assemblyCode << "BRANCH TO LABEL_" << node->node->GetId() << std::endl;
                // Générer le code pour les sorties du nœud branch
                for (const auto& outputNode : node->body) {
                    generateCode(outputNode);
                }
            } else if (node->node->GetType() == "function-entry-node") {
                // Générer le code pour un nœud de type "function-entry-node"
                assemblyCode << "FUNCTION ENTRY POINT" << std::endl;
                // Générer le code pour les nœuds connectés à ce nœud
                for (const auto& inputNode : node->inputs) {
                    generateCode(inputNode);
                }
            }
        };

        // // Generate all constants
        // for (const auto& astNode : m_ast.nodeMap) {
        //     assemblyCode << astNode.second->node->GenerateConstants();
        // }

        // After the constants, the main entry point:
        assemblyCode << ".main:\n";
    
        // Commencer la génération de code à partir du nœud d'entrée
        generateCode(entryNode);

        // Stocker le code assembleur généré dans l'AST
        m_ast.assemblyCode = assemblyCode.str();
    }
    
    std::string GetCode() const {
        return m_ast.assemblyCode;
    }

private:
    AST m_ast;

};

/*

class AssemblyGenerator {
    public:
        AssemblyGenerator() {
            Reset();
        }
    
        void Reset() {
            m_assembly.str("");
            m_labelCounter = 0;
            m_variableAddresses.clear();
            m_currentStackOffset = 0;
        }
    
        std::string GenerateAssembly(const std::vector<std::shared_ptr<ASTNode>>& roots) {
            Reset();
            
            // Generate data section
            m_assembly << "section .data\n";
            // Add string constants for print nodes
            m_assembly << "msg_good db 'Good!',0xa,0\n";
            m_assembly << "msg_bad db 'Bad :(',0xa,0\n";
            
            // Generate code section
            m_assembly << "\nsection .text\n";
            m_assembly << "global _start\n\n";
            m_assembly << "_start:\n";
    
            // Generate entry point code
            for (const auto& root : roots) {
                GenerateNodeCode(root);
            }
    
            // Add exit syscall
            m_assembly << "    mov eax, 1\n";  // exit syscall
            m_assembly << "    mov ebx, 0\n";  // return 0
            m_assembly << "    int 0x80\n";
    
            return m_assembly.str();
        }
    
    private:
        std::stringstream m_assembly;
        int m_labelCounter;
        std::unordered_map<std::string, int> m_variableAddresses;
        int m_currentStackOffset;
    
        std::string GenerateUniqueLabel(const std::string& prefix) {
            return prefix + "_" + std::to_string(m_labelCounter++);
        }
    
        void GenerateNodeCode(std::shared_ptr<ASTNode> node) {
            if (!node) return;
    
            if (node->IsType<FunctionEntryNode>()) {
                // Generate code for function entry
                m_assembly << "    ; Function Entry\n";
                for (auto& child : node->children) {
                    GenerateNodeCode(child);
                }
            }
            else if (node->IsType<BranchNode>()) {
                GenerateBranchCode(node);
            }
            else if (node->IsType<PrintNode>()) {
                GeneratePrintCode(node);
            }
            else if (node->IsType<VariableNode>()) {
                GenerateVariableCode(node);
            }
            else if (node->IsType<OperatorNode>()) {
                GenerateOperatorCode(node);
            }
        }
    
        void GenerateBranchCode(std::shared_ptr<ASTNode> node) {
            std::string labelTrue = GenerateUniqueLabel("true");
            std::string labelFalse = GenerateUniqueLabel("false");
            std::string labelEnd = GenerateUniqueLabel("end");
    
            m_assembly << "    ; Branch condition evaluation\n";
            
            // Generate condition code
            if (auto conditionNode = node->GetDataInput(0)) {
                GenerateNodeCode(conditionNode);
            }
    
            m_assembly << "    pop eax\n";
            m_assembly << "    cmp eax, 0\n";
            m_assembly << "    je " << labelFalse << "\n";
    
            // True branch
            m_assembly << labelTrue << ":\n";
            if (node->GetChildCount() > 0) {
                GenerateNodeCode(node->GetChild(0));
            }
            m_assembly << "    jmp " << labelEnd << "\n";
    
            // False branch
            m_assembly << labelFalse << ":\n";
            if (node->GetChildCount() > 1) {
                GenerateNodeCode(node->GetChild(1));
            }
    
            m_assembly << labelEnd << ":\n";
        }
    
        void GeneratePrintCode(std::shared_ptr<ASTNode> node) {
            auto* printNode = node->GetAs<PrintNode>();
            if (!printNode) return;
    
            m_assembly << "    ; Print message\n";
            if (printNode->GetText() == "Good!") {
                m_assembly << "    mov edx, 6\n";  // message length
                m_assembly << "    mov ecx, msg_good\n";
            } else {
                m_assembly << "    mov edx, 6\n";  // message length
                m_assembly << "    mov ecx, msg_bad\n";
            }
            m_assembly << "    mov ebx, 1\n";      // file descriptor (stdout)
            m_assembly << "    mov eax, 4\n";      // sys_write
            m_assembly << "    int 0x80\n";
        }
    
        void GenerateVariableCode(std::shared_ptr<ASTNode> node) {
            auto* varNode = node->GetAs<VariableNode>();
            if (!varNode) return;
    
            // For this example, we'll just push a value onto the stack
            std::string varName = varNode->GetVariableName();
            if (m_variableAddresses.find(varName) == m_variableAddresses.end()) {
                m_variableAddresses[varName] = m_currentStackOffset;
                m_currentStackOffset += 4;
            }
    
            m_assembly << "    ; Load variable " << varName << "\n";
            m_assembly << "    mov eax, [ebp-" << m_variableAddresses[varName] << "]\n";
            m_assembly << "    push eax\n";
        }
    
        void GenerateOperatorCode(std::shared_ptr<ASTNode> node) {
            auto* opNode = node->GetAs<OperatorNode>();
            if (!opNode) return;
    
            // Generate code for operands
            for (const auto& [port, inputNode] : node->dataInputs) {
                GenerateNodeCode(inputNode);
            }
    
            m_assembly << "    ; Operator " << static_cast<int>(opNode->GetOperationType()) << "\n";
            
            switch (opNode->GetOperationType()) {
                case OperatorNode::ADD:
                    m_assembly << "    pop ebx\n";
                    m_assembly << "    pop eax\n";
                    m_assembly << "    add eax, ebx\n";
                    m_assembly << "    push eax\n";
                    break;
                case OperatorNode::SUBTRACT:
                    m_assembly << "    pop ebx\n";
                    m_assembly << "    pop eax\n";
                    m_assembly << "    sub eax, ebx\n";
                    m_assembly << "    push eax\n";
                    break;
                // Add other operators...
            }
        }
    };

*/
/*

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




*/

