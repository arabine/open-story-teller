/*
The MIT License

Copyright (c) 2022 Anthony Rabine

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <iostream>
#include <thread>

#include "catch.hpp"
#include "chip32_assembler.h"
#include "chip32_macros.h"
#include "compiler.h"
#include "branch_node.h"
#include "print_node.h"
#include "variable_node.h"
#include "function_entry_node.h"
#include "operator_node.h"
#include "chip32_machine.h"
#include "variable.h"

#include <stdarg.h>
#include <string.h>

#include "ast_builder.h"
#include "assembly_generator.h"
#include "flow_generator.h"
#include "assembly_generator_chip32.h"

TEST_CASE( "Check various indentations and typos" ) {

    Compiler compiler;

    auto printNodeTrue = std::make_shared<PrintNode>("print-node");
    printNodeTrue->SetText("Good!");

    auto printNodeFalse = std::make_shared<PrintNode>("print-node");
    printNodeFalse->SetText("Bad :(");

    auto branchNode = std::make_shared<BranchNode>("branch-node");

    auto functionEntryNode = std::make_shared<FunctionEntryNode>("function-entry-node");
 

    std::vector<std::shared_ptr<Variable>> variables;
    auto var1 = std::make_shared<Variable>("X");
    var1->SetValue(5);
    var1->SetValueType(Variable::ValueType::INTEGER);

    auto var2 = std::make_shared<Variable>("Y");
    var2->SetValue(10);
    var2->SetValueType(Variable::ValueType::INTEGER);

    auto var3 = std::make_shared<Variable>("A");
    var3->SetValue(7);
    var3->SetValueType(Variable::ValueType::INTEGER);

    auto var4 = std::make_shared<Variable>("B");
    var4->SetValue(2);
    var4->SetValueType(Variable::ValueType::INTEGER);


    variables.push_back(var1);
    variables.push_back(var2);
    variables.push_back(var3);
    variables.push_back(var4);

    auto variableNodeX = std::make_shared<VariableNode>("variable-node");
    variableNodeX->SetVariable(var1);

    auto variableNodeY = std::make_shared<VariableNode>("variable-node");
    variableNodeY->SetVariable(var2);

    auto variableNodeA = std::make_shared<VariableNode>("variable-node");
    variableNodeA->SetVariable(var3);

    auto variableNodeB = std::make_shared<VariableNode>("variable-node");
    variableNodeB->SetVariable(var4);

    auto testNode = std::make_shared<OperatorNode>();
    testNode->SetOperationType(OperatorNode::OperationType::GREATER_THAN);

    auto addNode = std::make_shared<OperatorNode>();
    addNode->SetOperationType(OperatorNode::OperationType::ADD);

    auto subNode = std::make_shared<OperatorNode>();
    subNode->SetOperationType(OperatorNode::OperationType::SUBTRACT);

    std::vector<std::shared_ptr<BaseNode>> nodes;

    nodes.push_back(functionEntryNode);
    nodes.push_back(printNodeTrue);
    nodes.push_back(printNodeFalse);
    nodes.push_back(branchNode);
    nodes.push_back(variableNodeX);
    nodes.push_back(variableNodeY);
    nodes.push_back(variableNodeA);
    nodes.push_back(variableNodeB);
    nodes.push_back(testNode);
    nodes.push_back(addNode);
    nodes.push_back(subNode);

    auto cn1 = std::make_shared<Connection>();
    auto cn2 = std::make_shared<Connection>();
    auto cn3 = std::make_shared<Connection>();
    auto cn4 = std::make_shared<Connection>();
    auto cn5 = std::make_shared<Connection>();
    auto cn6 = std::make_shared<Connection>();
    auto cn7 = std::make_shared<Connection>();
    auto cn8 = std::make_shared<Connection>();
    auto cn9 = std::make_shared<Connection>();
    auto cn10 = std::make_shared<Connection>();

    std::vector<std::shared_ptr<Connection>> connections;

    connections.push_back(cn1);
    connections.push_back(cn2);
    connections.push_back(cn3);
    connections.push_back(cn4);
    connections.push_back(cn5);
    connections.push_back(cn6);
    connections.push_back(cn7);
    connections.push_back(cn8);
    connections.push_back(cn9);
    connections.push_back(cn10);

    // Branch  True -> print Ok
    //         False -> print Ko  

    // True path
    cn1->inNodeId = printNodeTrue->GetId();
    cn1->inPortIndex = 0;
    cn1->outNodeId = branchNode->GetId();
    cn1->outPortIndex = 0;

    // False path
    cn2->inNodeId = printNodeFalse->GetId();
    cn2->inPortIndex = 0;
    cn2->outNodeId = branchNode->GetId();
    cn2->outPortIndex = 1;
  
    // Function entry -> Branch node
    cn3->inNodeId = branchNode->GetId();
    cn3->inPortIndex = 0;
    cn3->outNodeId = functionEntryNode->GetId();
    cn3->outPortIndex = 0;
    
    // Compare test output -> Branch node (condition input)
    cn4->inNodeId = branchNode->GetId();
    cn4->inPortIndex = 1;
    cn4->outNodeId = testNode->GetId();
    cn4->outPortIndex = 0;
    cn4->type = Connection::DATA_LINK;

    // + output 1 -> Compare test node input 1
    cn5->inNodeId = testNode->GetId();
    cn5->inPortIndex = 0;
    cn5->outNodeId = addNode->GetId();
    cn5->outPortIndex = 0;
    cn5->type = Connection::DATA_LINK;

    // - output  -> Compare test node input 1
    cn6->inNodeId = testNode->GetId();
    cn6->inPortIndex = 1;
    cn6->outNodeId = subNode->GetId();
    cn6->outPortIndex = 0;
    cn6->type = Connection::DATA_LINK;

    // ADD NODE INPUTS

    cn7->inNodeId = addNode->GetId();
    cn7->inPortIndex = 0;
    cn7->outNodeId = variableNodeX->GetId();
    cn7->outPortIndex = 0;
    cn7->type = Connection::DATA_LINK;

    cn8->inNodeId = addNode->GetId();
    cn8->inPortIndex = 1;
    cn8->outNodeId = variableNodeY->GetId();
    cn8->outPortIndex = 0;
    cn8->type = Connection::DATA_LINK;

    // SUBTRACT NODE INPUTS

    cn9->inNodeId = subNode->GetId();
    cn9->inPortIndex = 0;
    cn9->outNodeId = variableNodeA->GetId();
    cn9->outPortIndex = 0;
    cn9->type = Connection::DATA_LINK;

    cn10->inNodeId = subNode->GetId();
    cn10->inPortIndex = 1;
    cn10->outNodeId = variableNodeB->GetId();
    cn10->outPortIndex = 0;
    cn10->type = Connection::DATA_LINK;


     // Create generator context with current time and user
     AssemblyGenerator::GeneratorContext context(
        "2025-04-08 12:09:01",  // Current UTC time
        "arabine",              // Current user
        true,                   // Enable debug output
        true,                   // Enable optimizations
        1024                    // Stack size
    );

    // Create generator
    AssemblyGeneratorChip32 generator(context);


    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();

    // Generate flow in the console
    VisualFlowGenerator flowGenerator(context);
    FlowVisualizer::PrintHeader("arabine", "2025-04-08 12:03:01");
    std::string flow = flowGenerator.GenerateAssembly(nodes, pathTree, variables);

    
    std::cout << "\nGenerated flow:\n" << flow << std::endl;

    // Generate assembly
    std::string assembly = generator.GenerateAssembly(nodes, pathTree, variables);

    std::cout << "\nGenerated assembly:\n" << assembly << std::endl;
    
    Chip32::Machine machine;

    machine.QuickExecute(assembly);


}
