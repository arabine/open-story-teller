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
class WaitEventNode;
class WaitDelayNode;
class PlayMediaNode;
class SendSignalNode;

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

// ===================================================================
// SECTION 1: INCLUDES (à ajouter après les includes existants)
// ===================================================================

#include "ast_builder.h"
#include "variable_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "branch_node.h"
#include "function_entry_node.h"
#include "call_function_node.h"
#include "wait_event_node.h"
#include "wait_delay_node.h"
#include "play_media_node.h"
#include "send_signal_node.h"

// ===================================================================
// SECTION 2: CLASSE TACGenerator - MODIFICATIONS
// ===================================================================

class TACGenerator {
public:
    TACGenerator() : m_tempCounter(0), m_labelCounter(0) {}

    // ===================================================================
    // NOUVELLE SIGNATURE: Génère le TAC à partir de l'AST + Variables
    // ===================================================================
    TACProgram Generate(const std::vector<std::shared_ptr<ASTNode>>& astNodes,
                       const std::vector<std::shared_ptr<Variable>>& variables) {
        std::cout << "\n=== TACGenerator::Generate() START ===\n";
        std::cout << "Number of AST nodes received: " << astNodes.size() << "\n";
        std::cout << "Number of variables: " << variables.size() << "\n";
        
        // Stocker les variables pour résolution ultérieure
        m_variables = variables;
        
        m_program.Clear();
        m_tempCounter = 0;
        m_labelCounter = 0;
        m_nodeResults.clear();
        m_visitedNodes.clear();
        
        // Générer le code pour chaque nœud
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
    
    // NOUVEAU: Variables disponibles pour résolution
    std::vector<std::shared_ptr<Variable>> m_variables;

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
    // NOUVEAU: HELPERS POUR RÉSOLUTION DE VARIABLES
    // ===================================================================
    
    std::shared_ptr<Variable> ResolveVariableByUuid(const std::string& uuid) const {
        if (uuid.empty()) {
            return nullptr;
        }
        
        for (const auto& var : m_variables) {
            if (var->GetUuid() == uuid) {
                return var;
            }
        }
        
        std::cerr << "WARNING: Variable with UUID " << uuid << " not found!\n";
        return nullptr;
    }
    
    std::string GetVariableLabel(const std::string& uuid) const {
        auto var = ResolveVariableByUuid(uuid);
        if (var) {
            return var->GetLabel();
        }
        return "unknown_var";
    }

    // ===================================================================
    // GÉNÉRATION RÉCURSIVE - DISPATCHER
    // ===================================================================

    std::shared_ptr<TACOperand> GenerateNode(std::shared_ptr<ASTNode> node) {
        if (!node) return nullptr;

        std::cout << "  GenerateNode(" << node->node->GetTypeName() 
                  << ", ID: " << node->GetId() << ")\n";

        // Vérifier si ce nœud a déjà été complètement traité
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

        // ===================================================================
        // DISPATCHER PAR TYPE DE NŒUD
        // ===================================================================
        
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
        // ===================================================================
        // NŒUDS SYSCALL
        // ===================================================================
        else if (node->IsType<WaitEventNode>()) {
            std::cout << "    -> Type: WaitEventNode\n";
            GenerateWaitEventNode(node);
        }
        else if (node->IsType<WaitDelayNode>()) {
            std::cout << "    -> Type: WaitDelayNode\n";
            GenerateWaitDelayNode(node);
        }
        else if (node->IsType<PlayMediaNode>()) {
            std::cout << "    -> Type: PlayMediaNode\n";
            GeneratePlayMediaNode(node);
        }
        else if (node->IsType<SendSignalNode>()) {
            std::cout << "    -> Type: SendSignalNode\n";
            GenerateSendSignalNode(node);
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
    // GÉNÉRATION PAR TYPE DE NŒUD - EXISTANTS (pas de changement)
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
        
        // Évaluer tous les arguments
        std::vector<std::shared_ptr<TACOperand>> args;
        
        // Collecter et trier les inputs par port index
        std::vector<std::pair<unsigned int, std::shared_ptr<ASTNode>>> sortedInputs;
        for (const auto& [port, inputNode] : node->dataInputs) {
            sortedInputs.push_back({port, inputNode});
        }
        std::sort(sortedInputs.begin(), sortedInputs.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; });

        // Générer le code pour chaque argument
        for (const auto& [port, inputNode] : sortedInputs) {
            auto argOperand = GenerateNode(inputNode);
            if (argOperand) {
                args.push_back(argOperand);
            }
        }

        // Générer les instructions PARAM pour chaque argument
        for (size_t i = 0; i < args.size(); i++) {
            auto paramInstr = std::make_shared<TACInstruction>(
                TACInstruction::OpCode::PARAM,
                args[i]
            );
            m_program.AddInstruction(paramInstr);
        }

        // Créer l'opérande pour la chaîne de format
        auto formatOperand = std::make_shared<TACOperand>(
            TACOperand::Type::VARIABLE,
            printNode->GetLabel()
        );

        // Générer l'instruction PRINT
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
        auto* branchNode = node->GetAs<BranchNode>();
        if (!branchNode) {
            throw std::runtime_error("BranchNode cast failed");
        }

        // Évaluer la condition (le BranchNode reçoit la condition sur le port 1)
        auto conditionInput = node->GetDataInput(1);
        if (!conditionInput) {
            throw std::runtime_error("BranchNode missing condition input on port 1");
        }
        
        auto conditionOperand = GenerateNode(conditionInput);
        if (!conditionOperand) {
            throw std::runtime_error("BranchNode condition evaluation returned null");
        }

        // Créer les labels pour true/false/end
        auto trueLabel = NewLabel("branch_true");
        auto falseLabel = NewLabel("branch_false");
        auto endLabel = NewLabel("branch_end");

        // Convention: 0 = false, toute autre valeur = true
        // if (!condition) goto falseLabel
        auto ifInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::IF_FALSE,
            falseLabel,
            conditionOperand
        );
        m_program.AddInstruction(ifInstr);

        // True branch label
        auto labelTrue = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::LABEL,
            trueLabel
        );
        m_program.AddInstruction(labelTrue);
        
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
        auto labelFalse = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::LABEL,
            falseLabel
        );
        m_program.AddInstruction(labelFalse);
        
        // False branch code
        if (node->GetChild(1)) {
            GenerateNode(node->GetChild(1));
        }

        // End label
        auto labelEnd = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::LABEL,
            endLabel
        );
        m_program.AddInstruction(labelEnd);
    }

    // ===================================================================
    // NOUVEAUX GÉNÉRATEURS POUR LES NŒUDS SYSCALL
    // ===================================================================

    void GenerateWaitDelayNode(std::shared_ptr<ASTNode> node) {
        auto* waitDelayNode = node->GetAs<WaitDelayNode>();
        if (!waitDelayNode) return;

        std::cout << "    GenerateWaitDelayNode: duration=" 
                  << waitDelayNode->GetDuration() << "ms\n";

        // Vérifier s'il y a un override depuis un port data (IN 1)
        auto durationInput = node->GetDataInput(1);
        std::shared_ptr<TACOperand> durationOperand;
        
        if (durationInput) {
            // Évaluer l'expression connectée
            durationOperand = GenerateNode(durationInput);
            std::cout << "      Duration from port 1: " << durationOperand->ToString() << "\n";
        } else {
            // Utiliser la valeur par défaut
            durationOperand = std::make_shared<TACOperand>(
                TACOperand::Type::CONSTANT,
                std::to_string(waitDelayNode->GetDuration())
            );
            std::cout << "      Duration from property: " << waitDelayNode->GetDuration() << "\n";
        }

        // PARAM pour la durée (R0)
        auto paramInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PARAM,
            durationOperand
        );
        m_program.AddInstruction(paramInstr);

        // SYSCALL 5
        auto syscallNumOperand = std::make_shared<TACOperand>(
            TACOperand::Type::CONSTANT, "5"
        );
        auto syscallInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::SYSCALL,
            syscallNumOperand
        );
        m_program.AddInstruction(syscallInstr);
    }

    void GenerateSendSignalNode(std::shared_ptr<ASTNode> node) {
        auto* sendSignalNode = node->GetAs<SendSignalNode>();
        if (!sendSignalNode) return;

        std::cout << "    GenerateSendSignalNode: signal_id=" 
                  << sendSignalNode->GetSignalId() << "\n";

        // Vérifier s'il y a un override depuis un port data (IN 1)
        auto signalInput = node->GetDataInput(1);
        std::shared_ptr<TACOperand> signalOperand;
        
        if (signalInput) {
            signalOperand = GenerateNode(signalInput);
            std::cout << "      Signal ID from port 1: " << signalOperand->ToString() << "\n";
        } else {
            signalOperand = std::make_shared<TACOperand>(
                TACOperand::Type::CONSTANT,
                std::to_string(sendSignalNode->GetSignalId())
            );
            std::cout << "      Signal ID from property: " << sendSignalNode->GetSignalId() << "\n";
        }

        // PARAM pour le signal ID (R0)
        auto paramInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PARAM,
            signalOperand
        );
        m_program.AddInstruction(paramInstr);

        // SYSCALL 3
        auto syscallNumOperand = std::make_shared<TACOperand>(
            TACOperand::Type::CONSTANT, "3"
        );
        auto syscallInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::SYSCALL,
            syscallNumOperand
        );
        m_program.AddInstruction(syscallInstr);
    }

    void GeneratePlayMediaNode(std::shared_ptr<ASTNode> node) {
        auto* playMediaNode = node->GetAs<PlayMediaNode>();
        if (!playMediaNode) return;

        std::cout << "    GeneratePlayMediaNode (data-driven)\n";

        // Image name (R0) - depuis le port data IN 1
        auto imageInput = node->GetDataInput(1);
        std::shared_ptr<TACOperand> imageOperand;
        
        if (imageInput) {
            imageOperand = GenerateNode(imageInput);
            std::cout << "      Image from port 1: " << imageOperand->ToString() << "\n";
        } else {
            // Pas de connexion = null pointer (0)
            imageOperand = std::make_shared<TACOperand>(
                TACOperand::Type::CONSTANT, "0"
            );
            std::cout << "      No image connected (null)\n";
        }

        // Sound name (R1) - depuis le port data IN 2
        auto soundInput = node->GetDataInput(2);
        std::shared_ptr<TACOperand> soundOperand;
        
        if (soundInput) {
            soundOperand = GenerateNode(soundInput);
            std::cout << "      Sound from port 2: " << soundOperand->ToString() << "\n";
        } else {
            // Pas de connexion = null pointer (0)
            soundOperand = std::make_shared<TACOperand>(
                TACOperand::Type::CONSTANT, "0"
            );
            std::cout << "      No sound connected (null)\n";
        }

        // PARAMs
        auto paramImage = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PARAM, imageOperand
        );
        m_program.AddInstruction(paramImage);

        auto paramSound = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PARAM, soundOperand
        );
        m_program.AddInstruction(paramSound);

        // SYSCALL 1
        auto syscallNumOperand = std::make_shared<TACOperand>(
            TACOperand::Type::CONSTANT, "1"
        );
        auto syscallInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::SYSCALL,
            syscallNumOperand
        );
        m_program.AddInstruction(syscallInstr);

        // Optionnel: stocker le status de retour
        std::string statusVarUuid = playMediaNode->GetStatusVariableUuid();
        if (!statusVarUuid.empty()) {
            std::string varLabel = GetVariableLabel(statusVarUuid);
            std::cout << "      Storing status in variable: " << varLabel << "\n";
            
            auto resultOperand = std::make_shared<TACOperand>(
                TACOperand::Type::VARIABLE,
                varLabel
            );
            
            auto r0Operand = std::make_shared<TACOperand>(
                TACOperand::Type::REGISTER, "r0"
            );
            
            auto storeInstr = std::make_shared<TACInstruction>(
                TACInstruction::OpCode::STORE,
                resultOperand,
                r0Operand
            );
            m_program.AddInstruction(storeInstr);
        }
    }

    void GenerateWaitEventNode(std::shared_ptr<ASTNode> node) {
        auto* waitEventNode = node->GetAs<WaitEventNode>();
        if (!waitEventNode) return;

        std::cout << "    GenerateWaitEventNode: mask=0x" 
                  << std::hex << waitEventNode->GetEventMask() << std::dec 
                  << ", timeout=" << waitEventNode->GetTimeout() << "ms\n";

        // Event mask (R0)
        auto maskInput = node->GetDataInput(1);
        std::shared_ptr<TACOperand> maskOperand;
        
        if (maskInput) {
            maskOperand = GenerateNode(maskInput);
            std::cout << "      Event mask from port 1: " << maskOperand->ToString() << "\n";
        } else {
            maskOperand = std::make_shared<TACOperand>(
                TACOperand::Type::CONSTANT,
                std::to_string(waitEventNode->GetEventMask())
            );
            std::cout << "      Event mask from property: 0x" << std::hex 
                      << waitEventNode->GetEventMask() << std::dec << "\n";
        }

        // Timeout (R1)
        auto timeoutInput = node->GetDataInput(2);
        std::shared_ptr<TACOperand> timeoutOperand;
        
        if (timeoutInput) {
            timeoutOperand = GenerateNode(timeoutInput);
            std::cout << "      Timeout from port 2: " << timeoutOperand->ToString() << "\n";
        } else {
            timeoutOperand = std::make_shared<TACOperand>(
                TACOperand::Type::CONSTANT,
                std::to_string(waitEventNode->GetTimeout())
            );
            std::cout << "      Timeout from property: " << waitEventNode->GetTimeout() << "ms\n";
        }

        // PARAMs
        auto paramMask = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PARAM, maskOperand
        );
        m_program.AddInstruction(paramMask);

        auto paramTimeout = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::PARAM, timeoutOperand
        );
        m_program.AddInstruction(paramTimeout);

        // SYSCALL 2
        auto syscallNumOperand = std::make_shared<TACOperand>(
            TACOperand::Type::CONSTANT, "2"
        );
        auto syscallInstr = std::make_shared<TACInstruction>(
            TACInstruction::OpCode::SYSCALL,
            syscallNumOperand
        );
        m_program.AddInstruction(syscallInstr);

        // IMPORTANT: Stocker le résultat dans la variable configurée
        std::string resultVarUuid = waitEventNode->GetResultVariableUuid();
        if (!resultVarUuid.empty()) {
            std::string varLabel = GetVariableLabel(resultVarUuid);
            std::cout << "      Storing result in variable: " << varLabel << "\n";
            
            auto resultOperand = std::make_shared<TACOperand>(
                TACOperand::Type::VARIABLE,
                varLabel
            );
            
            auto r0Operand = std::make_shared<TACOperand>(
                TACOperand::Type::REGISTER, "r0"
            );
            
            auto storeInstr = std::make_shared<TACInstruction>(
                TACInstruction::OpCode::STORE,
                resultOperand,
                r0Operand
            );
            m_program.AddInstruction(storeInstr);
        } else {
            std::cout << "      No result variable, R0 will contain event code\n";
        }
    }

};

// ===================================================================
// FIN DES ADDITIONS
// ===================================================================
