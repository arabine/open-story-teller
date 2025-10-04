#pragma once

#include "ast_builder.h"
#include "assembly_generator.h"
#include "call_function_node.h"
#include <algorithm>

class AssemblyGeneratorChip32 : public AssemblyGenerator
{
public:
    AssemblyGeneratorChip32(const GeneratorContext& context)
        : AssemblyGenerator(context)
        , m_currentContext(FunctionContext::MAIN_PROGRAM)
    {

    }
    virtual ~AssemblyGeneratorChip32() = default;


    void GenerateNodeCode(std::shared_ptr<ASTNode> node, bool isDataPath = false) override
    {
        if (!node) return;

        if (m_context.debugOutput) {
            AddComment("Node: " + node->node->GetTypeName() + " (ID: " + node->node->GetId() + ")");
        }
        // Node label
        m_assembly << node->node->GetMyEntryLabel() << ":\n";

        if (node->IsType<OperatorNode>()) {
            GenerateOperatorNode(node);
        }
        else if (node->IsType<FunctionEntryNode>()) {
            // Détecter si c'est le main ou une sous-fonction
            // Weight 100 = fonction principale (main)
            auto* entry = node->GetAs<FunctionEntryNode>();
            m_currentContext = (entry->GetWeight() >= 100) 
                ? FunctionContext::MAIN_PROGRAM 
                : FunctionContext::SUB_FUNCTION;
                
            GenerateFunctionEntry(node);
        }
        else if (node->IsType<BranchNode>()) {
            GenerateBranchNode(node);
        }
        else if (node->IsType<PrintNode>()) {
            GeneratePrintNode(node);
        }
        else if (node->IsType<CallFunctionNode>()) {
            GenerateCallFunctionNode(node);
        }

        // Détection automatique des fins de fonction/programme
        if (node->GetChildCount() == 0)
        {
            if (m_currentContext == FunctionContext::MAIN_PROGRAM) {
                AddComment("Program exit (automatic)");
                m_assembly << "    halt\n";
            } else {
                AddComment("Function return (automatic)");
                m_assembly << "    ret\n";
            }
        }
    }

    virtual void AddComment(const std::string& comment) {
        m_assembly << std::string(m_depth * 4, ' ') << "; " << comment << "\n";
    }

    virtual void GenerateExit() override {
        AddComment("Program exit");
        m_assembly << "    halt\n";
    }

private:
    enum class FunctionContext {
        MAIN_PROGRAM,
        SUB_FUNCTION
    };
    
    FunctionContext m_currentContext;

    virtual void GenerateMain() override  {
        // Program entry point
        m_assembly << ".main:\n";
    }

    void GenerateFunctionEntry(std::shared_ptr<ASTNode> node) {
        auto* entry = node->GetAs<FunctionEntryNode>();
        
        if (m_currentContext == FunctionContext::MAIN_PROGRAM) {
            AddComment("Main function entry");
        } else {
            AddComment("Function entry");
            // Si nécessaire, sauvegarder les registres
            // m_assembly << "    push r0\n";
            // m_assembly << "    push r1\n";
            // etc.
        }
    }

    void GenerateCallFunctionNode(std::shared_ptr<ASTNode> node) {
        auto* callNode = node->GetAs<CallFunctionNode>();
        if (!callNode) return;

        std::string functionName = callNode->GetFunctionName();
        
        AddComment("Call function: " + functionName);
        m_depth++;

        // Préparer les arguments si nécessaire
        // Dans votre système, les variables globales sont utilisées
        // donc pas besoin de passer des arguments sur la pile
        
        // Appel de la fonction
        m_assembly << "    call " << GetFunctionLabel(functionName) << "\n";
        
        // Après le retour de la fonction, les variables globales 
        // ont potentiellement été modifiées et sont directement accessibles
        
        m_depth--;
    }

    void GenerateBranchNode(std::shared_ptr<ASTNode> node)
    {
        AddComment("Branch condition evaluation");
        m_depth++;

        auto trueBranch = node->GetChild(0);
        auto falseBranch = node->GetChild(1);

        // Compare result and jump
        m_assembly << "    pop r0\n"
                << "    skipz r0\n"
                << "    jump " << trueBranch->node->GetMyEntryLabel() << "\n"
                << "    jump " << falseBranch->node->GetMyEntryLabel() << "\n";

        m_depth--;
    }

    void GeneratePrintNode(std::shared_ptr<ASTNode> node)
    {
        auto* printNode = node->GetAs<PrintNode>();
        if (!printNode) return;

        std::string label = printNode->GetLabel();
        
        AddComment("Print: " + printNode->GetText());
        m_depth++;

        // Count the number of arguments connected to the print node
        int argCount = 0;
        std::vector<std::pair<unsigned int, std::shared_ptr<ASTNode>>> sortedInputs;
        
        // Collect and sort data inputs by port index
        for (const auto& [port, inputNode] : node->dataInputs) {
            sortedInputs.push_back({port, inputNode});
        }
        
        // Sort by port index to ensure correct argument order (arg0, arg1, arg2, arg3)
        std::sort(sortedInputs.begin(), sortedInputs.end(), 
                [](const auto& a, const auto& b) { return a.first < b.first; });
        
        argCount = sortedInputs.size();
        
        // Save registers that we'll use
        m_assembly << "    push r0\n"
                    << "    push r1\n";
        
        // Save argument registers if we have arguments
        if (argCount > 0) m_assembly << "    push r2\n";
        if (argCount > 1) m_assembly << "    push r3\n";
        if (argCount > 2) m_assembly << "    push r4\n";
        if (argCount > 3) m_assembly << "    push r5\n";
        
        // Load arguments into registers r2, r3, r4, r5
        int regIndex = 2;  // Start with r2 for first argument
        for (const auto& [port, inputNode] : sortedInputs) {
            if (regIndex > 5) {
                // Maximum 4 arguments (r2, r3, r4, r5)
                throw std::runtime_error("Print node supports maximum 4 arguments");
            }
            
            // Check if the input node is a variable
            if (inputNode->IsType<VariableNode>()) {
                auto* varNode = inputNode->GetAs<VariableNode>();
                if (varNode) {
                    std::string varUuid = varNode->GetVariableUuid();
                    
                    // Find variable in the context
                    auto var = m_context.FindVariableByUuid(varUuid);
                    if (var) {
                        m_assembly << "    ; Load arg" << (regIndex - 2) << ": " 
                                << var->GetVariableName() << "\n";
                        int varSize = GetVariableSize(var);
                        m_assembly << "    load r" << regIndex << ", $" 
                                << var->GetLabel() << ", " << varSize << " ; " 
                                << var->GetVariableName() << "\n";
                    } else {
                        throw std::runtime_error("Variable not found in context for print argument");
                    }
                }
            } else {
                // For non-variable inputs, we might need to evaluate expressions
                // For now, we only support direct variable connections
                throw std::runtime_error("Print node currently only supports direct variable connections");
            }
            
            regIndex++;
        }
        
        // Load format string address into r0
        m_assembly << "    lcons r0, $" << label << " ; format string\n";
        
        // Load number of arguments into r1
        m_assembly << "    lcons r1, " << argCount << " ; number of arguments\n";
        
        // Call printf syscall
        m_assembly << "    syscall 4\n";
        
        // Restore argument registers (in reverse order)
        if (argCount > 3) m_assembly << "    pop r5\n";
        if (argCount > 2) m_assembly << "    pop r4\n";
        if (argCount > 1) m_assembly << "    pop r3\n";
        if (argCount > 0) m_assembly << "    pop r2\n";
        
        // Restore r0 and r1
        m_assembly << "    pop r1\n"
                    << "    pop r0\n";
        
        m_depth--;
    }

    void GenerateOperatorNode(std::shared_ptr<ASTNode> node) {
        auto* opNode = node->GetAs<OperatorNode>();
        if (!opNode) return;

        AddComment("Operator: " + opNode->GetOperatorSymbol());
        m_depth++;

        // Generate code for variables usage
        int reg = 0;
        for (const auto& [port, inputNode] : node->dataInputs)
        {
            // Check if the input node is a variable
            if (inputNode->IsType<VariableNode>())
            {
                auto* varNode = inputNode->GetAs<VariableNode>();
                if (varNode) {
                    std::string varUuid = varNode->GetVariableUuid();

                    // Find variable in the context
                    auto var = m_context.FindVariableByUuid(varUuid);
                    if (var)
                    {
                        // Generate code to load the variable value
                        int varSize = GetVariableSize(var);
                        m_assembly << "    load r" << reg  << ", $" << var->GetLabel() 
                                << ", " << varSize <<  " ; Load variable " 
                                << var->GetVariableName() << "\n";
                        m_assembly << "    push r" << reg << "\n";
                    }
                    else 
                    {
                        throw std::runtime_error("Variable not set in node: " + inputNode->node->GetId());
                    }
                }
            }
            reg++;
        }
        
        // Generate operator code based on type
        switch (opNode->GetOperationType()) {
            // ===== ARITHMETIC OPERATORS =====
            case OperatorNode::OperationType::ADD:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    add r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::SUBTRACT:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    sub r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::MULTIPLY:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    mul r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::DIVIDE:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    div r0, r1\n"
                        << "    push r0\n";
                break;
            
            // ===== COMPARISON OPERATORS =====
            // Utilise les instructions eq, gt, lt de Chip32
            // Syntaxe: eq r_dest, r_op1, r_op2  →  r_dest = (r_op1 == r_op2 ? 1 : 0)
            
            case OperatorNode::OperationType::EQUAL:
                m_assembly << "    pop r1  ; second operand\n"
                        << "    pop r0  ; first operand\n"
                        << "    eq r0, r0, r1  ; r0 = (r0 == r1 ? 1 : 0)\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::NOT_EQUAL:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    eq r0, r0, r1  ; r0 = (r0 == r1 ? 1 : 0)\n"
                        << "    lcons r2, 1\n"
                        << "    xor r0, r2  ; inverse: 0→1, 1→0\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::GREATER_THAN:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    gt r0, r0, r1  ; r0 = (r0 > r1 ? 1 : 0)\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::LESS_THAN:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    lt r0, r0, r1  ; r0 = (r0 < r1 ? 1 : 0)\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::GREATER_EQUAL:
                // >= est équivalent à NOT(<)
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    lt r0, r0, r1  ; r0 = (r0 < r1 ? 1 : 0)\n"
                        << "    lcons r2, 1\n"
                        << "    xor r0, r2  ; inverse: >= est NOT(<)\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::LESS_EQUAL:
                // <= est équivalent à NOT(>)
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    gt r0, r0, r1  ; r0 = (r0 > r1 ? 1 : 0)\n"
                        << "    lcons r2, 1\n"
                        << "    xor r0, r2  ; inverse: <= est NOT(>)\n"
                        << "    push r0\n";
                break;
            
            // ===== LOGICAL OPERATORS =====
            case OperatorNode::OperationType::AND:
                // AND logique: résultat 1 si les deux sont non-zéro
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    ; Logical AND\n"
                        << "    lcons r2, 0\n"
                        << "    skipz r0\n"
                        << "    skipz r1\n"
                        << "    lcons r2, 1\n"
                        << "    mov r0, r2\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::OR:
                // OR logique: résultat 1 si au moins un est non-zéro
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    ; Logical OR\n"
                        << "    or r0, r1  ; bitwise or\n"
                        << "    lcons r2, 0\n"
                        << "    skipz r0\n"
                        << "    lcons r2, 1\n"
                        << "    mov r0, r2\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::NOT:
                // NOT logique: 0→1, non-zero→0
                m_assembly << "    pop r0\n"
                        << "    ; Logical NOT\n"
                        << "    lcons r1, 1\n"
                        << "    skipz r0\n"
                        << "    lcons r1, 0\n"
                        << "    mov r0, r1\n"
                        << "    push r0\n";
                break;
            
            // ===== BITWISE OPERATORS =====
            case OperatorNode::OperationType::BITWISE_AND:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    and r0, r1\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::BITWISE_OR:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    or r0, r1\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::BITWISE_XOR:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    xor r0, r1\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::BITWISE_NOT:
                m_assembly << "    pop r0\n"
                        << "    not r0\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::LEFT_SHIFT:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    shl r0, r1\n"
                        << "    push r0\n";
                break;
                
            case OperatorNode::OperationType::RIGHT_SHIFT:
                m_assembly << "    pop r1\n"
                        << "    pop r0\n"
                        << "    shr r0, r1\n"
                        << "    push r0\n";
                break;
                
            default:
                throw std::runtime_error("Unsupported operator type");
        }

        m_depth--;
    }

    // Helper pour générer le label d'une fonction à partir de son nom
    std::string GetFunctionLabel(const std::string& functionName) {
        // Convertir le nom de fonction en label valide
        // Par exemple: "MyFunction" -> ".func_MyFunction"
        return ".func_" + functionName;
    }

        virtual void Visit(const std::shared_ptr<Variable> v) override
    {
        if (v->IsConstant())
        {
            if (v->GetValueType() == Variable::ValueType::STRING)
            {
                m_assembly  << "$" << v->GetLabel() << " DC8, \""  << v->GetValue<std::string>() << "\" ; "  << v->GetVariableName() << "\n";
            }
            else if (v->GetValueType() == Variable::ValueType::INTEGER)
            {
                m_assembly  << "$" << v->GetLabel() << " DC32, "  << v->GetValue<int>() << " ; "  << v->GetVariableName() << "\n";
            }
            else if (v->GetValueType() == Variable::ValueType::FLOAT)
            {
                m_assembly  << "$" << v->GetLabel() << " DC32, "  << v->GetValue<float>() << " ; "  << v->GetVariableName() << "\n";
            }
            else if (v->GetValueType() == Variable::ValueType::BOOL)
            {
                m_assembly  << "$" << v->GetLabel() << " DCB, "  << (v->GetValue<bool>() ? "1" : "0") << " ; "  << v->GetVariableName() << "\n";
            }

        }
    }


    virtual void GenerateVariable(const std::shared_ptr<Variable> v) 
    {
        if (v->GetValueType() == Variable::ValueType::STRING)
        {
            m_assembly  << "$" << v->GetLabel() << " DV8, \""  << v->GetValue<std::string>() << "\" ; "  << v->GetVariableName() << "\n";
        }
        else if (v->GetValueType() == Variable::ValueType::INTEGER)
        {
            m_assembly  << "$" << v->GetLabel() << " DV32, "  << v->GetValue<int>() << " ; "  << v->GetVariableName() << "\n";
        }
        else if (v->GetValueType() == Variable::ValueType::FLOAT)
        {
            m_assembly  << "$" << v->GetLabel() << " DV32, "  << v->GetValue<float>() << " ; "  << v->GetVariableName() << "\n";
        }
        else if (v->GetValueType() == Variable::ValueType::BOOL)
        {
            m_assembly  << "$" << v->GetLabel() << " DVB, "  << (v->GetValue<bool>() ? "1" : "0") << " ; "  << v->GetVariableName() << "\n";
        }
    }
private:
        // Helper pour obtenir la taille en bytes d'une variable selon son type
    int GetVariableSize(std::shared_ptr<Variable> var) const {
        switch (var->GetValueType()) {
            case Variable::ValueType::INTEGER:
                return 4;  // 32 bits = 4 bytes (DV32/DC32)
            case Variable::ValueType::FLOAT:
                return 4;  // 32 bits = 4 bytes (DV32/DC32)
            case Variable::ValueType::BOOL:
                return 1;  // 8 bits = 1 byte (DVB/DCB)
            case Variable::ValueType::STRING:
                // Pour les strings, on charge l'adresse (pointeur 32-bit)
                return 4;  // Adresse = 4 bytes
            default:
                throw std::runtime_error("Unknown variable type");
        }
    }
};