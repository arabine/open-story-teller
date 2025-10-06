// ===================================================================
// tac.h - Three-Address Code Representation (CLEAN VERSION)
// ===================================================================
// Ce fichier contient UNIQUEMENT la représentation TAC et le générateur
// La conversion TAC → Assembleur est dans AssemblyGeneratorChip32TAC
// ===================================================================

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

// Forward declarations
class ASTNode;
class VariableNode;
class OperatorNode;
class PrintNode;
class BranchNode;
class FunctionEntryNode;
class CallFunctionNode;

// ===================================================================
// TAC Operand - Représente une opérande (variable, temporaire, constante)
// ===================================================================
class TACOperand {
public:
    enum class Type {
        VARIABLE,      // Variable globale ($label)
        TEMPORARY,     // Variable temporaire (t0, t1, ...)
        CONSTANT,      // Constante numérique
        LABEL,         // Label de saut
        REGISTER       // Registre explicite (r0, r1, ...)
    };

    TACOperand(Type type, const std::string& value)
        : m_type(type), m_value(value) {}

    Type GetType() const { return m_type; }
    std::string GetValue() const { return m_value; }

    std::string ToString() const {
        switch (m_type) {
            case Type::VARIABLE: return "$" + m_value;
            case Type::TEMPORARY: return "%" + m_value;
            case Type::CONSTANT: return m_value;
            case Type::LABEL: return "." + m_value;
            case Type::REGISTER: return m_value;
        }
        return m_value;
    }

private:
    Type m_type;
    std::string m_value;
};

// ===================================================================
// TAC Instruction - Une instruction à 3 adresses
// ===================================================================
class TACInstruction {
public:
    enum class OpCode {
        // Arithmetic
        ADD,        // dest = op1 + op2
        SUB,        // dest = op1 - op2
        MUL,        // dest = op1 * op2
        DIV,        // dest = op1 / op2
        
        // Comparison
        EQ,         // dest = op1 == op2
        NE,         // dest = op1 != op2
        GT,         // dest = op1 > op2
        LT,         // dest = op1 < op2
        GE,         // dest = op1 >= op2
        LE,         // dest = op1 <= op2
        
        // Logical
        AND,        // dest = op1 && op2
        OR,         // dest = op1 || op2
        NOT,        // dest = !op1
        
        // Bitwise
        BIT_AND,    // dest = op1 & op2
        BIT_OR,     // dest = op1 | op2
        BIT_XOR,    // dest = op1 ^ op2
        BIT_NOT,    // dest = ~op1
        
        // Assignment
        COPY,       // dest = op1
        LOAD,       // dest = *op1 (load from memory)
        STORE,      // *dest = op1 (store to memory)
        
        // Control flow
        LABEL,      // Define a label
        GOTO,       // Unconditional jump
        IF_FALSE,   // if (!op1) goto label
        IF_TRUE,    // if (op1) goto label
        
        // Function calls
        PARAM,      // Push parameter for call
        CALL,       // Call function (dest = result)
        RETURN,     // Return from function
        
        // Special
        PRINT,      // Print (syscall 4)
        SYSCALL,    // Generic syscall
        NOP         // No operation
    };

    TACInstruction(OpCode op)
        : m_opcode(op) {}

    TACInstruction(OpCode op, std::shared_ptr<TACOperand> dest)
        : m_opcode(op), m_dest(dest) {}

    TACInstruction(OpCode op, std::shared_ptr<TACOperand> dest, 
                   std::shared_ptr<TACOperand> op1)
        : m_opcode(op), m_dest(dest), m_op1(op1) {}

    TACInstruction(OpCode op, std::shared_ptr<TACOperand> dest,
                   std::shared_ptr<TACOperand> op1,
                   std::shared_ptr<TACOperand> op2)
        : m_opcode(op), m_dest(dest), m_op1(op1), m_op2(op2) {}

    OpCode GetOpCode() const { return m_opcode; }
    std::shared_ptr<TACOperand> GetDest() const { return m_dest; }
    std::shared_ptr<TACOperand> GetOp1() const { return m_op1; }
    std::shared_ptr<TACOperand> GetOp2() const { return m_op2; }

    std::string ToString() const {
        std::stringstream ss;
        
        switch (m_opcode) {
            case OpCode::ADD:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " + " << m_op2->ToString();
                break;
            case OpCode::SUB:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " - " << m_op2->ToString();
                break;
            case OpCode::MUL:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " * " << m_op2->ToString();
                break;
            case OpCode::DIV:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " / " << m_op2->ToString();
                break;
            case OpCode::EQ:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " == " << m_op2->ToString();
                break;
            case OpCode::NE:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " != " << m_op2->ToString();
                break;
            case OpCode::GT:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " > " << m_op2->ToString();
                break;
            case OpCode::LT:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " < " << m_op2->ToString();
                break;
            case OpCode::GE:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " >= " << m_op2->ToString();
                break;
            case OpCode::LE:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " <= " << m_op2->ToString();
                break;
            case OpCode::AND:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " && " << m_op2->ToString();
                break;
            case OpCode::OR:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " || " << m_op2->ToString();
                break;
            case OpCode::NOT:
                ss << m_dest->ToString() << " = !" << m_op1->ToString();
                break;
            case OpCode::BIT_AND:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " & " << m_op2->ToString();
                break;
            case OpCode::BIT_OR:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " | " << m_op2->ToString();
                break;
            case OpCode::BIT_XOR:
                ss << m_dest->ToString() << " = " << m_op1->ToString() 
                   << " ^ " << m_op2->ToString();
                break;
            case OpCode::BIT_NOT:
                ss << m_dest->ToString() << " = ~" << m_op1->ToString();
                break;
            case OpCode::COPY:
                ss << m_dest->ToString() << " = " << m_op1->ToString();
                break;
            case OpCode::LOAD:
                ss << m_dest->ToString() << " = *" << m_op1->ToString();
                break;
            case OpCode::STORE:
                ss << "*" << m_dest->ToString() << " = " << m_op1->ToString();
                break;
            case OpCode::LABEL:
                ss << m_dest->ToString() << ":";
                break;
            case OpCode::GOTO:
                ss << "goto " << m_dest->ToString();
                break;
            case OpCode::IF_FALSE:
                ss << "if (!" << m_op1->ToString() << ") goto " << m_dest->ToString();
                break;
            case OpCode::IF_TRUE:
                ss << "if (" << m_op1->ToString() << ") goto " << m_dest->ToString();
                break;
            case OpCode::PARAM:
                ss << "param " << m_dest->ToString();
                break;
            case OpCode::CALL:
                if (m_op1) {
                    ss << m_dest->ToString() << " = call " << m_op1->ToString();
                } else {
                    ss << "call " << m_dest->ToString();
                }
                break;
            case OpCode::RETURN:
                if (m_dest) {
                    ss << "return " << m_dest->ToString();
                } else {
                    ss << "return";
                }
                break;
            case OpCode::PRINT:
                ss << "print " << m_dest->ToString();
                if (m_op1) ss << ", " << m_op1->ToString();
                if (m_op2) ss << ", ...";
                break;
            case OpCode::SYSCALL:
                ss << "syscall " << m_dest->ToString();
                break;
            case OpCode::NOP:
                ss << "nop";
                break;
            default:
                ss << "??? opcode=" << static_cast<int>(m_opcode);
        }
        
        return ss.str();
    }

private:
    OpCode m_opcode;
    std::shared_ptr<TACOperand> m_dest;
    std::shared_ptr<TACOperand> m_op1;
    std::shared_ptr<TACOperand> m_op2;
};

// ===================================================================
// TAC Program - Liste d'instructions TAC
// ===================================================================
class TACProgram {
public:
    void AddInstruction(std::shared_ptr<TACInstruction> instr) {
        m_instructions.push_back(instr);
    }

    const std::vector<std::shared_ptr<TACInstruction>>& GetInstructions() const {
        return m_instructions;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << "=== Three-Address Code ===\n";
        for (const auto& instr : m_instructions) {
            ss << instr->ToString() << "\n";
        }
        return ss.str();
    }

    void Clear() {
        m_instructions.clear();
    }

    size_t Size() const {
        return m_instructions.size();
    }

private:
    std::vector<std::shared_ptr<TACInstruction>> m_instructions;
};

// ===================================================================
// TAC Generator - Convertit AST → TAC
// ===================================================================
// Note: Nécessite les includes des types de nœuds concrets
// ===================================================================

#include "ast_builder.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "branch_node.h"
#include "function_entry_node.h"
#include "call_function_node.h"

class TACGenerator {
public:
    TACGenerator() : m_tempCounter(0), m_labelCounter(0) {}

    // Génère le TAC à partir de l'AST
    TACProgram Generate(const std::vector<std::shared_ptr<ASTNode>>& astNodes) {
        std::cout << "\n=== TACGenerator::Generate() START ===\n";
        std::cout << "Number of AST nodes received: " << astNodes.size() << "\n";
        
        m_program.Clear();
        m_tempCounter = 0;
        m_labelCounter = 0;
        m_nodeResults.clear();
        m_visitedNodes.clear();
        
        // Simplement appeler GenerateNode pour chaque nœud
        // La gestion des doublons se fait dans GenerateNode()
        for (size_t i = 0; i < astNodes.size(); i++) {
            const auto& node = astNodes[i];
            std::cout << "Processing AST node [" << i << "]: " 
                      << node->node->GetTypeName() 
                      << " (ID: " << node->GetId() << ")\n";
            
            GenerateNode(node);
        }
        
        std::cout << "Total TAC instructions generated: " << m_program.Size() << "\n";
        std::cout << "=== TACGenerator::Generate() END ===\n\n";
        
        return m_program;
    }

    // Accesseur pour obtenir le dernier programme généré
    const TACProgram& GetProgram() const {
        return m_program;
    }

private:
    TACProgram m_program;
    int m_tempCounter;
    int m_labelCounter;
    
    // Map pour garder en mémoire où sont stockés les résultats des nœuds
    std::map<std::string, std::shared_ptr<TACOperand>> m_nodeResults;
    
    // Set pour éviter de visiter deux fois le même nœud
    std::set<std::string> m_visitedNodes;

    // ===================================================================
    // HELPERS
    // ===================================================================

    // Génère un nouveau temporaire
    std::shared_ptr<TACOperand> NewTemp() {
        return std::make_shared<TACOperand>(
            TACOperand::Type::TEMPORARY,
            "t" + std::to_string(m_tempCounter++)
        );
    }

    // Génère un nouveau label
    std::shared_ptr<TACOperand> NewLabel(const std::string& prefix = "L") {
        return std::make_shared<TACOperand>(
            TACOperand::Type::LABEL,
            prefix + std::to_string(m_labelCounter++)
        );
    }

    // ===================================================================
    // GÉNÉRATION RÉCURSIVE
    // ===================================================================

    // Génère le code pour un nœud et retourne où est stocké son résultat
    std::shared_ptr<TACOperand> GenerateNode(std::shared_ptr<ASTNode> node) {
        if (!node) return nullptr;

        std::cout << "  GenerateNode(" << node->node->GetTypeName() 
                  << ", ID: " << node->GetId() << ")\n";

        // NOUVEAU : Vérifier si ce nœud a déjà été complètement traité
        if (m_visitedNodes.find(node->GetId()) != m_visitedNodes.end()) {
            std::cout << "    -> Node already fully processed, SKIPPING\n";
            // Retourner le résultat en cache s'il existe
            auto it = m_nodeResults.find(node->GetId());
            if (it != m_nodeResults.end()) {
                return it->second;
            }
            return nullptr;
        }

        // Marquer ce nœud comme visité DÈS LE DÉBUT
        m_visitedNodes.insert(node->GetId());

        // Vérifier si on a déjà évalué ce nœud (résultat en cache)
        auto it = m_nodeResults.find(node->GetId());
        if (it != m_nodeResults.end()) {
            std::cout << "    -> Result already cached, returning " 
                      << it->second->ToString() << "\n";
            return it->second;
        }

        std::shared_ptr<TACOperand> result = nullptr;

        if (node->IsType<VariableNode>()) {
            std::cout << "    -> Type: VariableNode\n";
            result = GenerateVariableNode(node);
        }
        else if (node->IsType<OperatorNode>()) {
            std::cout << "    -> Type: OperatorNode\n";
            result = GenerateOperatorNode(node);
        }
        else if (node->IsType<PrintNode>()) {
            std::cout << "    -> Type: PrintNode\n";
            GeneratePrintNode(node);
        }
        else if (node->IsType<BranchNode>()) {
            std::cout << "    -> Type: BranchNode\n";
            GenerateBranchNode(node);
        }
        else if (node->IsType<CallFunctionNode>()) {
            std::cout << "    -> Type: CallFunctionNode\n";
            GenerateCallFunctionNode(node);
        }
        else if (node->IsType<FunctionEntryNode>()) {
            std::cout << "    -> Type: FunctionEntryNode (generating children)\n";
            // Entry point, générer les enfants
            for (size_t i = 0; i < node->children.size(); i++) {
                std::cout << "      Processing child [" << i << "]\n";
                GenerateNode(node->children[i]);
            }
        }

        // Mémoriser le résultat
        if (result) {
            std::cout << "    -> Caching result: " << result->ToString() << "\n";
            m_nodeResults[node->GetId()] = result;
        }

        return result;
    }

    // ===================================================================
    // GÉNÉRATION PAR TYPE DE NŒUD
    // ===================================================================

    std::shared_ptr<TACOperand> GenerateVariableNode(std::shared_ptr<ASTNode> node) {
        auto* varNode = node->GetAs<VariableNode>();
        if (!varNode) {
            std::cout << "    ERROR: node is not a VariableNode!\n";
            return nullptr;
        }

        auto var = varNode->GetVariable();
        
        if (!var) {
            std::cout << "    ERROR: Variable is NULL for node " << varNode->GetId() 
                    << " (UUID: " << varNode->GetVariableUuid() << ")\n";
            std::cout << "    This should have been resolved before TAC generation!\n";
            return nullptr;
        }

        // Créer une opérande qui référence la variable
        return std::make_shared<TACOperand>(
            TACOperand::Type::VARIABLE,
            var->GetLabel()
        );
    }

    std::shared_ptr<TACOperand> GenerateOperatorNode(std::shared_ptr<ASTNode> node) {
        auto* opNode = node->GetAs<OperatorNode>();
        if (!opNode) return nullptr;

        // Évaluer les opérandes (récursif)
        std::vector<std::shared_ptr<TACOperand>> operands;
        
        // Collecter et trier les inputs par port index
        std::vector<std::pair<unsigned int, std::shared_ptr<ASTNode>>> sortedInputs;
        for (const auto& [port, inputNode] : node->dataInputs) {
            sortedInputs.push_back({port, inputNode});
        }
        std::sort(sortedInputs.begin(), sortedInputs.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; });

        for (const auto& [port, inputNode] : sortedInputs) {
            auto operand = GenerateNode(inputNode);
            if (operand) {
                operands.push_back(operand);
            }
        }

        if (operands.size() < 2) {
            throw std::runtime_error("Operator needs at least 2 operands");
        }

        // Créer un temporaire pour le résultat
        auto resultTemp = NewTemp();

        // Générer l'instruction TAC appropriée
        TACInstruction::OpCode opcode;
        switch (opNode->GetOperationType()) {
            case OperatorNode::OperationType::ADD:
                opcode = TACInstruction::OpCode::ADD;
                break;
            case OperatorNode::OperationType::SUBTRACT:
                opcode = TACInstruction::OpCode::SUB;
                break;
            case OperatorNode::OperationType::MULTIPLY:
                opcode = TACInstruction::OpCode::MUL;
                break;
            case OperatorNode::OperationType::DIVIDE:
                opcode = TACInstruction::OpCode::DIV;
                break;
            case OperatorNode::OperationType::EQUAL:
                opcode = TACInstruction::OpCode::EQ;
                break;
            case OperatorNode::OperationType::NOT_EQUAL:
                opcode = TACInstruction::OpCode::NE;
                break;
            case OperatorNode::OperationType::GREATER_THAN:
                opcode = TACInstruction::OpCode::GT;
                break;
            case OperatorNode::OperationType::LESS_THAN:
                opcode = TACInstruction::OpCode::LT;
                break;
            case OperatorNode::OperationType::GREATER_EQUAL:
                opcode = TACInstruction::OpCode::GE;
                break;
            case OperatorNode::OperationType::LESS_EQUAL:
                opcode = TACInstruction::OpCode::LE;
                break;
            default:
                throw std::runtime_error("Unsupported operator");
        }

        auto instr = std::make_shared<TACInstruction>(
            opcode, resultTemp, operands[0], operands[1]
        );
        m_program.AddInstruction(instr);

        return resultTemp;
    }

    void GeneratePrintNode(std::shared_ptr<ASTNode> node) {
        auto* printNode = node->GetAs<PrintNode>();
        if (!printNode) return;

        std::cout << "    GeneratePrintNode: START\n";

        // RÉCUPÉRER ET CONVERTIR LE FORMAT STRING
        std::string formatString = printNode->GetText();
        
        // Évaluer tous les arguments et déterminer leurs types
        std::vector<std::shared_ptr<TACOperand>> args;
        std::vector<Variable::ValueType> argTypes;
        
        // Collecter et trier les inputs par port index
        std::vector<std::pair<unsigned int, std::shared_ptr<ASTNode>>> sortedInputs;
        for (const auto& [port, inputNode] : node->dataInputs) {
            std::cout << "      Found input at port " << port 
                    << " -> " << inputNode->node->GetTypeName() << "\n";
            sortedInputs.push_back({port, inputNode});
        }
        std::sort(sortedInputs.begin(), sortedInputs.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; });

        // Générer le code pour chaque argument ET récupérer son type
        for (const auto& [port, inputNode] : sortedInputs) {
            std::cout << "      Processing input port " << port << "\n";
            auto argOperand = GenerateNode(inputNode);
            if (argOperand) {
                std::cout << "        -> Got operand: " << argOperand->ToString() << "\n";
                args.push_back(argOperand);
                
                // DÉTERMINER LE TYPE DE L'ARGUMENT
                Variable::ValueType argType = Variable::ValueType::INTEGER; // défaut
                
                if (inputNode->IsType<VariableNode>()) {
                    auto* varNode = inputNode->GetAs<VariableNode>();
                    auto var = varNode->GetVariable();
                    if (var) {
                        argType = var->GetValueType();
                        std::cout << "        -> Variable type: " 
                                << Variable::ValueTypeToString(argType) << "\n";
                    }
                }
                // Pour les OperatorNode, le résultat est toujours un INTEGER
                else if (inputNode->IsType<OperatorNode>()) {
                    argType = Variable::ValueType::INTEGER;
                }
                
                argTypes.push_back(argType);
            }
        }

        std::cout << "      Total args collected: " << args.size() << "\n";

        // CONVERTIR LES PLACEHOLDERS EN FONCTION DU TYPE
        for (size_t i = 0; i < argTypes.size() && i < 4; i++) {
            std::string placeholder = "{" + std::to_string(i) + "}";
            std::string formatSpec;
            
            switch (argTypes[i]) {
                case Variable::ValueType::STRING:
                    formatSpec = "{" + std::to_string(i) + ":s}";
                    break;
                case Variable::ValueType::INTEGER:
                    formatSpec = "{" + std::to_string(i) + ":d}";
                    break;
                case Variable::ValueType::FLOAT:
                    formatSpec = "{" + std::to_string(i) + ":f}";
                    break;
                default:
                    formatSpec = "{" + std::to_string(i) + ":d}";
            }
            
            // Remplacer {0} par {0:d} ou {0:s}
            size_t pos = formatString.find(placeholder);
            if (pos != std::string::npos) {
                formatString.replace(pos, placeholder.length(), formatSpec);
            }
        }
        
        // METTRE À JOUR LE FORMAT STRING DANS LA VARIABLE
        auto formatVar = printNode->GetVariable(printNode->GetLabel());
        if (formatVar) {
            formatVar->SetTextValue(formatString);
            std::cout << "      Updated format string to: " << formatString << "\n";
        }

        // Créer l'opérande pour la chaîne de format (avec le format mis à jour)
        auto formatOperand = std::make_shared<TACOperand>(
            TACOperand::Type::VARIABLE,
            printNode->GetLabel()
        );

        // Générer les instructions PARAM pour chaque argument
        for (size_t i = 0; i < args.size(); i++) {
            std::cout << "      Generating PARAM instruction [" << i << "] for " 
                    << args[i]->ToString() << "\n";
            auto paramInstr = std::make_shared<TACInstruction>(
                TACInstruction::OpCode::PARAM,
                args[i]
            );
            m_program.AddInstruction(paramInstr);
        }

        // Générer l'instruction PRINT
        std::cout << "      Generating PRINT instruction\n";
        auto printInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PRINT,
            formatOperand
        );
        m_program.AddInstruction(printInstr);
        
        std::cout << "    GeneratePrintNode: END (generated " 
                << (args.size() + 1) << " instructions)\n";
    }

    void GenerateCallFunctionNode(std::shared_ptr<ASTNode> node) {
        auto* callNode = node->GetAs<CallFunctionNode>();
        if (!callNode) return;
        
        std::string pageUuid = callNode->GetFunctionUuid();
        std::string pageLabel = "page_" + pageUuid;
        
        std::cout << "    Generating CALL to page: " << pageLabel << "\n";
        
        // Créer l'opérande label pour la page cible
        auto targetLabel = std::make_shared<TACOperand>(
            TACOperand::Type::LABEL,
            pageLabel
        );
        
        // Générer l'instruction CALL
        auto callInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::CALL,
            targetLabel
        );
        
        m_program.AddInstruction(callInstr);
    }

    void GenerateBranchNode(std::shared_ptr<ASTNode> node) {
        // Évaluer la condition (le BranchNode reçoit la condition sur le port 1)
        auto conditionInput = node->GetDataInput(1);
        if (!conditionInput) {
            throw std::runtime_error("BranchNode missing condition input on port 1");
        }
        
        auto conditionOperand = GenerateNode(conditionInput);
        if (!conditionOperand) {
            throw std::runtime_error("BranchNode condition evaluation returned null");
        }

        // Créer les labels pour true/false
        auto trueLabel = NewLabel("true_branch");
        auto falseLabel = NewLabel("false_branch");
        auto endLabel = NewLabel("end_branch");

        // if (!condition) goto falseLabel
        auto ifInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::IF_FALSE,
            falseLabel,
            conditionOperand
        );
        m_program.AddInstruction(ifInstr);

        // True branch label
        auto labelInstr1 = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::LABEL,
            trueLabel
        );
        m_program.AddInstruction(labelInstr1);
        
        // True branch code
        if (node->GetChild(0)) {
            GenerateNode(node->GetChild(0));
        }

        // goto end
        auto gotoEnd = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::GOTO,
            endLabel
        );
        m_program.AddInstruction(gotoEnd);

        // False branch label
        auto labelInstr2 = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::LABEL,
            falseLabel
        );
        m_program.AddInstruction(labelInstr2);
        
        // False branch code
        if (node->GetChild(1)) {
            GenerateNode(node->GetChild(1));
        }

        // End label
        auto labelInstr3 = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::LABEL,
            endLabel
        );
        m_program.AddInstruction(labelInstr3);
    }
};

// ===================================================================
// FIN DU FICHIER
// ===================================================================
// Note: La conversion TAC → Assembleur est dans AssemblyGeneratorChip32TAC
// Ce fichier ne contient QUE la représentation TAC pure
// ===================================================================