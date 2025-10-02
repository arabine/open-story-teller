#pragma once

#include "ast_builder.h"
#include "assembly_generator.h"
#include "call_function_node.h"

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

    void GeneratePrintNode(std::shared_ptr<ASTNode> node) {
        auto* printNode = node->GetAs<PrintNode>();
        if (!printNode) return;

        std::string label = printNode->GetLabel();

        m_assembly << "    push r0\n"
                    << "    push r1\n"
                    << "    lcons r0, $" << label << "\n"
                    << "    lcons r1, 0 ; number of arguments\n"  // FIXME: handle arguments
                    << "    syscall 4\n"
                    << "    pop r1\n"
                    << "    pop r0\n";
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
                        // FIXME: hardcoded 4 bytes, replace by actual real variable size
                        m_assembly << "    load r" << reg  << ", $" << var->GetLabel() << ", 4" <<  " ; Load variable " << var->GetVariableName() << "\n";
                        m_assembly << "    push r" << reg << "\n";
                    }
                    else 
                    {
                        throw std::runtime_error("ERROR! Variable not set in node: " + inputNode->node->GetId());
                    }
                }
                reg++;
            }
        }
         
        // Generate operator code based on type
        switch (opNode->GetOperationType()) {
            case OperatorNode::OperationType::ADD:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    add r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::SUBTRACT:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    sub r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::MULTIPLY:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    mul r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::DIVIDE:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    div r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::MODULO:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    mod r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::EQUAL:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    cmp r0, r1\n"
                        << "    lcons r0, 1\n"
                        << "    skipz\n"
                        << "    lcons r0, 0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::NOT_EQUAL:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    cmp r0, r1\n"
                        << "    lcons r0, 0\n"
                        << "    skipz\n"
                        << "    lcons r0, 1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::GREATER_THAN:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    cmp r0, r1\n"
                        << "    lcons r0, 1\n"
                        << "    skipgt\n"
                        << "    lcons r0, 0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::LESS_THAN:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    cmp r0, r1\n"
                        << "    lcons r0, 1\n"
                        << "    skiplt\n"
                        << "    lcons r0, 0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::GREATER_EQUAL:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    cmp r0, r1\n"
                        << "    lcons r0, 1\n"
                        << "    skipge\n"
                        << "    lcons r0, 0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::LESS_EQUAL:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    cmp r0, r1\n"
                        << "    lcons r0, 1\n"
                        << "    skiple\n"
                        << "    lcons r0, 0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::AND:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    and r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::OR:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    or r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::NOT:
                m_assembly << "    pop r0\n"
                        << "    not r0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::BITWISE_AND:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    and r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::BITWISE_OR:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    or r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::BITWISE_XOR:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    xor r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::BITWISE_NOT:
                m_assembly << "    pop r0\n"
                        << "    not r0\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::LEFT_SHIFT:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    shl r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::RIGHT_SHIFT:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
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
};