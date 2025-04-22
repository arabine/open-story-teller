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
/*
        if (isDataPath)
        {
            
            
            // else if (node->IsType<VariableNode>()) {
            //     GenerateVariableNode(node);
            // }
        }
        else
        {

        */
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
        // }

        // If we're processing a data path, traverse data outputs
        if (isDataPath) {
            for (const auto& [port, outputs] : node->dataOutputs) {
                for (const auto& target : outputs) {
                    GenerateNodeCode(target.node, true);
                }
            }
        }

    }

    virtual void AddComment(const std::string& comment) {
        m_assembly << std::string(m_depth * 4, ' ') << "; " << comment << "\n";
    }



    virtual void GenerateExit()  {
        AddComment("Program exit");
        m_assembly << "    halt\n";
    }

private:

    void GenerateFunctionEntry(std::shared_ptr<ASTNode> node) {
        AddComment("Function Entry");
        m_depth++;
        
        // for (auto& child : node->children) {
        //     GenerateNodeCode(child);
        // }
        
        m_depth--;
    }

    void GenerateBranchNode(std::shared_ptr<ASTNode> node)
    {
        std::string labelTrue = GenerateUniqueLabel("true");
        std::string labelFalse = GenerateUniqueLabel("false");
        std::string labelEnd = GenerateUniqueLabel("end");

        AddComment("Branch condition evaluation");
        m_depth++;

        // Generate condition code
        // We search a path tree that have a last node equivalent to our node
        // (this is the input of the condition)
/*
        auto lastNode = std::find_if(m_roots.begin(), m_roots.end(),
            [&node](const PathTree& tree) {
                return tree.lastNode && tree.lastNode->node->GetId() == node->node->GetId();
            });

        AddComment("Last node: " + lastNode->lastNode->node->GetTypeName() + " (ID: " + lastNode->lastNode->node->GetId() + ")");
  */      

        // Compare result and jump
        m_assembly << "    pop eax\n"
                << "    cmp eax, 0\n"
                << "    je " << labelFalse << "\n";

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

        // Generate code for variables
        for (const auto& [port, inputNode] : node->dataInputs) {
          // m_assembly << "    load r0, " << inputNode.node->GetId() << "\n";
        }
        

        // Generate operator code
        switch (opNode->GetOperationType()) {
            case OperatorNode::OperationType::ADD:
                m_assembly << "    pop ebx\n"
                        << "    pop eax\n"
                        << "    add eax, ebx\n"
                        << "    push eax\n";
                break;
            case OperatorNode::OperationType::SUBTRACT:
                m_assembly << "    pop ebx\n"
                        << "    pop eax\n"
                        << "    sub eax, ebx\n"
                        << "    push eax\n";
                break;
            case OperatorNode::OperationType::MULTIPLY:
                m_assembly << "    pop ebx\n"
                        << "    pop eax\n"
                        << "    imul eax, ebx\n"
                        << "    push eax\n";
                break;
            case OperatorNode::OperationType::DIVIDE:
                m_assembly << "    pop ebx\n"
                        << "    pop eax\n"
                        << "    cdq\n"
                        << "    idiv ebx\n"
                        << "    push eax\n";
                break;
            // Add other operators...

/*


    std::string GenerateAssembly() const override {
        std::stringstream ss;
        
        switch (m_operationType) {
            case OperationType::ADD:
                ss << "    pop ebx\n"
                   << "    pop eax\n"
                   << "    add eax, ebx\n"
                   << "    push eax\n";
                break;
            case OperationType::SUBTRACT:
                ss << "    pop ebx\n"
                   << "    pop eax\n"
                   << "    sub eax, ebx\n"
                   << "    push eax\n";
                break;
            case OperationType::MULTIPLY:
                ss << "    pop ebx\n"
                   << "    pop eax\n"
                   << "    imul eax, ebx\n"
                   << "    push eax\n";
                break;
            case OperationType::DIVIDE:
                ss << "    pop ebx\n"
                   << "    pop eax\n"
                   << "    cdq\n"         // Sign extend eax into edx
                   << "    idiv ebx\n"
                   << "    push eax\n";   // Push quotient
                break;
            case OperationType::AND:
                ss << "    pop ebx\n"
                   << "    pop eax\n"
                   << "    and eax, ebx\n"
                   << "    push eax\n";
                break;
            case OperationType::OR:
                ss << "    pop ebx\n"
                   << "    pop eax\n"
                   << "    or eax, ebx\n"
                   << "    push eax\n";
                break;
            // Add other operators...
        }

        return ss.str();
    }


*/



        }

        m_depth--;
    }

    virtual void Visit(const std::shared_ptr<Variable> v) override
    {
        if (v->IsConstant())
        {
            if (v->GetValueType() == Variable::ValueType::STRING)
            {
                m_assembly  << "$" << v->GetVariableName() << " DC8, \""  << v->GetValue<std::string>() << "\"\n";
            }
            else if (v->GetValueType() == Variable::ValueType::INTEGER)
            {
                m_assembly  << "$" << v->GetVariableName() << " DC32, "  << v->GetValue<int>() << "\n";
            }
            else if (v->GetValueType() == Variable::ValueType::FLOAT)
            {
                m_assembly  << "$" << v->GetVariableName() << " DC32, "  << v->GetValue<float>() << "\n";
            }
            else if (v->GetValueType() == Variable::ValueType::BOOL)
            {
                m_assembly  << "$" << v->GetVariableName() << " DCB, "  << (v->GetValue<bool>() ? "1" : "0") << "\n";
            }

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


