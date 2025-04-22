#pragma once

#include "assembly_generator.h"

#include <iostream>
#include <iomanip>
#include <chrono>

class FlowVisualizer {
public:
    static void PrintHeader(const std::string& user, const std::string& timestamp) {
        std::cout << "==========================================================\n";
        std::cout << "AST Flow Visualization\n";
        std::cout << "User: " << user << "\n";
        std::cout << "Time: " << timestamp << "\n";
        std::cout << "==========================================================\n\n" << std::endl;
    }

    static void PrintNodeExecution(const std::string& nodeName, int depth = 0) {
        PrintTimestamp();
        std::cout << std::string(depth * 2, ' ') << "→ Executing: " << nodeName << "\n";
    }

    static void PrintDataFlow(const std::string& from, const std::string& to, 
                            const std::string& value, int depth = 0) {
        PrintTimestamp();
        std::cout << std::string(depth * 2, ' ') << "  ┌─ Data: " << from 
                 << " → " << to << " (value: " << value << ")\n";
    }

    static void PrintBranchDecision(bool condition, const std::string& value, int depth = 0) {
        PrintTimestamp();
        std::cout << std::string(depth * 2, ' ') << "  ├─ Branch: " 
                 << (condition ? "TRUE" : "FALSE") << " (condition: " << value << ")\n";
    }

private:
    static void PrintTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::cout << "[" << std::put_time(std::localtime(&now_c), "%H:%M:%S") 
                 << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    }
};

class VisualFlowGenerator : public AssemblyGenerator {

public:
    VisualFlowGenerator(const GeneratorContext& context)
        : AssemblyGenerator(context)
    {

    }

protected:

    virtual void AddComment(const std::string& comment) {
       
    }


    virtual void Visit(const std::shared_ptr<Variable> v) {

    }


    virtual void GenerateExit()  {
       
    }

    void GenerateNodeCode(std::shared_ptr<ASTNode> node, bool isDataPath = false) override
    {
        if (!node) return;

        FlowVisualizer::PrintNodeExecution(node->node->GetTypeName(), m_depth);

        if (node->IsType<FunctionEntryNode>()) {
            m_depth++;
            for (auto& child : node->children) {
                GenerateNodeCode(child);
            }
            m_depth--;
        }
        else if (node->IsType<BranchNode>()) {
            m_depth++;
            // Get condition value
            auto conditionNode = node->GetDataInput(0);
            int conditionValue = 0;
            
            FlowVisualizer::PrintBranchDecision(conditionValue > 7, 
                std::to_string(conditionValue), m_depth);

            // Execute appropriate path
            if (conditionValue > 7) {
                GenerateNodeCode(node->GetChild(0));  // True path
            } else {
                GenerateNodeCode(node->GetChild(1));  // False path
            }
            m_depth--;
        }
        else if (node->IsType<PrintNode>()) {
            auto* printNode = node->GetAs<PrintNode>();
            FlowVisualizer::PrintNodeExecution("Print: " + printNode->GetText(), m_depth);
        }
        else if (node->IsType<OperatorNode>()) {
            m_depth++;
            auto* opNode = node->GetAs<OperatorNode>();
            

            
            FlowVisualizer::PrintDataFlow("Operand 1", opNode->GetOperatorSymbol(), 
                std::to_string(0), m_depth);
            FlowVisualizer::PrintDataFlow("Operand 2", opNode->GetOperatorSymbol(), 
                std::to_string(0), m_depth);
            

            FlowVisualizer::PrintDataFlow(opNode->GetOperatorSymbol(), "Result", 
                std::to_string(0), m_depth);

                for (const auto& [port, outputs] : node->dataOutputs) {
                    for (const auto& target : outputs) {
                        GenerateNodeCode(target.node, true);
                    }
                }
                
            m_depth--;
        }
        else if (node->IsType<VariableNode>()) {
            m_depth++;
            auto* varNode = node->GetAs<VariableNode>();
            // FlowVisualizer::PrintNodeExecution("Variable: " + varNode->GetVariableName(), m_depth);

            // If we're processing a data path, traverse data outputs
            if (isDataPath) {
                for (const auto& [port, outputs] : node->dataOutputs) {
                    for (const auto& target : outputs) {
                        GenerateNodeCode(target.node, true);
                    }
                }
            }
            m_depth--;
        }

        
    }

private:
    int m_depth = 0;

};
