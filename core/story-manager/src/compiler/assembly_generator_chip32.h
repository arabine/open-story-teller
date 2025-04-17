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

        if (isDataPath)
        {
            
            if (node->IsType<OperatorNode>()) {
                GenerateOperatorNode(node);
            }
            else if (node->IsType<VariableNode>()) {
                GenerateVariableNode(node);
            }
        }
        else
        {
            if (node->IsType<FunctionEntryNode>()) {
                GenerateFunctionEntry(node);
            }
            else if (node->IsType<BranchNode>()) {
                GenerateBranchNode(node);
            }
            else if (node->IsType<PrintNode>()) {
                GeneratePrintNode(node);
            }
        }

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
        
        for (auto& child : node->children) {
            GenerateNodeCode(child);
        }
        
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

        auto lastNode = std::find_if(m_roots.begin(), m_roots.end(),
            [&node](const PathTree& tree) {
                return tree.lastNode && tree.lastNode->node->GetId() == node->node->GetId();
            });

        AddComment("Last node: " + lastNode->lastNode->node->GetTypeName() + " (ID: " + lastNode->lastNode->node->GetId() + ")");
        

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

        std::string text = printNode->GetText();
        std::string label = AddStringLiteral(text);

        // System call to write
        m_assembly << "    mov edx, " << label << "_len\n"
                << "    mov ecx, " << label << "\n"
                << "    mov ebx, 1\n"    // stdout
                << "    mov eax, 4\n"    // sys_write
                << "    int 0x80\n";
    }

    void GenerateOperatorNode(std::shared_ptr<ASTNode> node) {
        auto* opNode = node->GetAs<OperatorNode>();
        if (!opNode) return;

        AddComment("Operator: " + std::to_string(static_cast<int>(opNode->GetOperationType())));
        m_depth++;

        // Generate code for operands
        for (const auto& [port, inputNode] : node->dataInputs) {
            GenerateNodeCode(inputNode);
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
        }

        m_depth--;
    }

    void GenerateVariableNode(std::shared_ptr<ASTNode> node) {
        auto* varNode = node->GetAs<VariableNode>();
        if (!varNode) return;

        std::string varName = varNode->GetVariableName();
        
        AddComment("Load variable: " + varName);
        m_assembly << "    mov eax, [" << m_variableAddresses[varName] << "]\n"
                  << "    push eax\n";
    }

};


