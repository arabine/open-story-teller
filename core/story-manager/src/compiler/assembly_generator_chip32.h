#pragma once

#include "ast_builder.h"
#include "assembly_generator.h"

class AssemblyGeneratorChip32 : public AssemblyGenerator
{
public:
    AssemblyGeneratorChip32(const GeneratorContext& context = GeneratorContext())
        : AssemblyGenerator(context)
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
            GenerateFunctionEntry(node);
        }
        else if (node->IsType<BranchNode>()) {
            GenerateBranchNode(node);
        }
        else if (node->IsType<PrintNode>()) {
            GeneratePrintNode(node);
        }

        // // If there is no any children, put an halt
        if (node->GetChildCount() == 0)
        {
            AddComment("Program exit");
            m_assembly << "    halt\n";
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

    virtual void GenerateMain() override  {
        // Program entry point
        m_assembly << ".main:\n";
    }

    void GenerateFunctionEntry(std::shared_ptr<ASTNode> node) {
        AddComment("Function Entry");
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


        m_assembly << "  push r0\n"
                    << "  push r1\n"
                    << "  lcons r0, $" << label << "\n"
                    << "  lcons r1, 0 ; number of arguments\n"  // FIXME: handle arguments
                    << "  syscall 4\n"
                    << "  pop r1\n"
                    << "  pop r0\n";

//        << ""mov r2, %2 // arguments are in r2, r3, r4 etc.

    }

    void GenerateOperatorNode(std::shared_ptr<ASTNode> node) {
        auto* opNode = node->GetAs<OperatorNode>();
        if (!opNode) return;

        AddComment("Operator: " + std::to_string(static_cast<int>(opNode->GetOperationType())));
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
                    auto var = varNode->GetVariable();
                    // Generate code to load the variable value
                    // FIXME: hardcoded 4 bytes, replace by actual real variable size
                    m_assembly << "    load r" << reg  << ", $" << var->GetLabel() << ", 4" <<  "; Load variable " << var->GetVariableName() << "\n";
                    m_assembly << "    push r" << reg << "\n";
                    // Assuming we have a function to load the variable value
                    // m_assembly << "    load r0, " << varNode->GetVariableName() << "\n";
                }
                reg++;
            }


          // m_assembly << "    load r0, " << inputNode.node->GetId() << "\n";
        }
         
 
        // Generate operator code
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
            case OperatorNode::OperationType::AND:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    and r0, r1\n"
                        << "    push r0\n";
                break;
            case OperatorNode::OperationType::GREATER_THAN:
                m_assembly << "    pop r0\n"
                        << "    pop r1\n"
                        << "    gt r0, r0, r1\n"
                        << "    push r0\n";
                break;
            default:
                // Make voluntary bad assembly
                m_assembly << "------>>>> OPERATOR NOT IMPLEMENTED: " << opNode->GetOperatorSymbol() << "\n";
                break;
        }

        m_depth--;
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
/*
    void GenerateVariableNode(std::shared_ptr<ASTNode> node) {
        auto* varNode = node->GetAs<VariableNode>();
        if (!varNode) return;

        std::string varName = varNode->GetVariableName();
        
        AddComment("Load variable: " + varName);
        m_assembly << "    mov eax, [" << m_variableAddresses[varName] << "]\n"
                  << "    push eax\n";
    }
*/
};


