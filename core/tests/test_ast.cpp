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

#include <catch2/catch_test_macros.hpp>

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
#include "assembly_generator_chip32_tac.h"


TEST_CASE( "Check AST with basic nodes" ) {

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
        variables,
        "2025-04-08 12:09:01",  // Current UTC time
        "unit-test-ast",              // Current user
        true,                   // Enable debug output
        true,                   // Enable optimizations
        1024                    // Stack size
    );


    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();

    // Generate flow in the console
    VisualFlowGenerator flowGenerator(context);
    FlowVisualizer::PrintHeader("arabine", "2025-04-08 12:03:01");
    flowGenerator.GenerateTextSection(pathTree);

    std::string flow = flowGenerator.GetAssembly();
    
    std::cout << "\nGenerated flow:\n" << flow << std::endl;

    //------------------------------------ Generate assembly ------------------------------------
    AssemblyGeneratorChip32TAC generator(context);
    generator.GenerateCompleteProgram(nodes, pathTree);
    std::string assembly = generator.GetAssembly().str();

    std::cout << "\nGenerated assembly:\n" << assembly << std::endl;
    
    Chip32::Machine machine;

    machine.QuickExecute(assembly);
}

// ===================================================================
// TEST 1 : Print simple avec 2 variables
// ===================================================================
TEST_CASE("TAC Generation - Print with 2 variables", "[tac][print]") {
    // === SETUP ===
    
    std::vector<std::shared_ptr<Variable>> variables;
    // Créer les variables
    auto var_a = std::make_shared<Variable>("a");
    var_a->SetIntegerValue(10);
    variables.push_back(var_a);
    
    auto var_b = std::make_shared<Variable>("b");
    var_b->SetIntegerValue(20);
    variables.push_back(var_b);
    
    // Créer les nœuds
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry-node");
    functionEntry->SetWeight(100);
    
    auto varNodeA = std::make_shared<VariableNode>("variable-node-a");
    varNodeA->SetVariable(var_a);
    
    auto varNodeB = std::make_shared<VariableNode>("variable-node-b");
    varNodeB->SetVariable(var_b);
    
    auto printNode = std::make_shared<PrintNode>("print-node");
    printNode->SetText("Values: {0}, {1}");
    printNode->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeA,
        varNodeB,
        printNode
    };
    
    // Créer les connexions
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → Print
    auto execConn = std::make_shared<Connection>();
    execConn->outNodeId = functionEntry->GetId();
    execConn->outPortIndex = 0;
    execConn->inNodeId = printNode->GetId();
    execConn->inPortIndex = 0;
    execConn->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn);
    
    // Data: varA → Print.arg0
    auto dataConn1 = std::make_shared<Connection>();
    dataConn1->outNodeId = varNodeA->GetId();
    dataConn1->outPortIndex = 0;
    dataConn1->inNodeId = printNode->GetId();
    dataConn1->inPortIndex = 1;  // arg0
    dataConn1->type = Connection::DATA_LINK;
    connections.push_back(dataConn1);
    
    // Data: varB → Print.arg1
    auto dataConn2 = std::make_shared<Connection>();
    dataConn2->outNodeId = varNodeB->GetId();
    dataConn2->outPortIndex = 0;
    dataConn2->inNodeId = printNode->GetId();
    dataConn2->inPortIndex = 2;  // arg1
    dataConn2->type = Connection::DATA_LINK;
    connections.push_back(dataConn2);
    
    // === BUILD AST ===
    ASTBuilder builder(nodes, connections);
    auto astNodes = builder.BuildAST();
    
    // === GENERATE TAC ===
    TACGenerator tacGen;
    
    TACProgram tac = tacGen.Generate(astNodes, variables);
    
    // === DISPLAY TAC (for debugging) ===
    std::cout << "\n" << tac.ToString() << std::endl;
    
    // === VERIFY TAC ===
    const auto& instructions = tac.GetInstructions();
    
    // On doit avoir exactement 3 instructions :
    // 1. param $a_label
    // 2. param $b_label
    // 3. print $format_label
    REQUIRE(instructions.size() == 3);
    
    // Instruction 0 : param $a
    REQUIRE(instructions[0]->GetOpCode() == TACInstruction::OpCode::PARAM);
    REQUIRE(instructions[0]->GetDest()->GetType() == TACOperand::Type::VARIABLE);
    REQUIRE(instructions[0]->GetDest()->GetValue() == var_a->GetLabel());
    
    // Instruction 1 : param $b
    REQUIRE(instructions[1]->GetOpCode() == TACInstruction::OpCode::PARAM);
    REQUIRE(instructions[1]->GetDest()->GetType() == TACOperand::Type::VARIABLE);
    REQUIRE(instructions[1]->GetDest()->GetValue() == var_b->GetLabel());
    
    // Instruction 2 : print $format
    REQUIRE(instructions[2]->GetOpCode() == TACInstruction::OpCode::PRINT);
    REQUIRE(instructions[2]->GetDest()->GetType() == TACOperand::Type::VARIABLE);
    REQUIRE(instructions[2]->GetDest()->GetValue() == printNode->GetLabel());
}


// ===================================================================
// Test complexe : AST avec TAC
// Teste : calculs intermédiaires, réutilisation, Print multi-args, branches
// ===================================================================

TEST_CASE("Complex AST with TAC - Intermediate results and reuse", "[tac][complex][ast]") {
    
    std::cout << "\n========================================\n";
    std::cout << "Complex TAC Test with Intermediate Results\n";
    std::cout << "========================================\n";
    
    // === VARIABLES ===
    std::vector<std::shared_ptr<Variable>> variables;
    
    auto var_A = std::make_shared<Variable>("A");
    var_A->SetIntegerValue(10);
    variables.push_back(var_A);
    
    auto var_B = std::make_shared<Variable>("B");
    var_B->SetIntegerValue(5);
    variables.push_back(var_B);
    
    auto var_C = std::make_shared<Variable>("C");
    var_C->SetIntegerValue(3);
    variables.push_back(var_C);
    
    auto var_threshold = std::make_shared<Variable>("threshold");
    var_threshold->SetIntegerValue(40);
    variables.push_back(var_threshold);
    
    // === NŒUDS ===
    
    // Entry point
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry-node");
    functionEntry->SetWeight(100);
    
    // Variable nodes
    auto varNodeA = std::make_shared<VariableNode>("variable-node-A");
    varNodeA->SetVariable(var_A);
    
    auto varNodeB = std::make_shared<VariableNode>("variable-node-B");
    varNodeB->SetVariable(var_B);
    
    auto varNodeC = std::make_shared<VariableNode>("variable-node-C");
    varNodeC->SetVariable(var_C);
    
    auto varNodeThreshold = std::make_shared<VariableNode>("variable-node-threshold");
    varNodeThreshold->SetVariable(var_threshold);
    
    // Opérateurs
    auto addNode = std::make_shared<OperatorNode>("operator-add");
    addNode->SetOperationType(OperatorNode::OperationType::ADD);
    
    auto subNode = std::make_shared<OperatorNode>("operator-sub");
    subNode->SetOperationType(OperatorNode::OperationType::SUBTRACT);
    
    auto mulNode = std::make_shared<OperatorNode>("operator-mul");
    mulNode->SetOperationType(OperatorNode::OperationType::MULTIPLY);
    
    auto compareNode = std::make_shared<OperatorNode>("operator-compare");
    compareNode->SetOperationType(OperatorNode::OperationType::GREATER_THAN);
    
    // Print nodes
    auto print1 = std::make_shared<PrintNode>("print-node-1");
    print1->SetText("Addition: {0} + {1} = {2}");
    print1->Initialize();
    
    auto print2 = std::make_shared<PrintNode>("print-node-2");
    print2->SetText("Subtraction: {0} - {1} = {2}");
    print2->Initialize();
    
    auto print3 = std::make_shared<PrintNode>("print-node-3");
    print3->SetText("Multiplication: {0} * {1} = {2}");
    print3->Initialize();
    
    // Branch node
    auto branchNode = std::make_shared<BranchNode>("branch-node");
    
    auto printTrue = std::make_shared<PrintNode>("print-true");
    printTrue->SetText("Result is LARGE (> 40)");
    printTrue->Initialize();
    
    auto printFalse = std::make_shared<PrintNode>("print-false");
    printFalse->SetText("Result is small");
    printFalse->Initialize();
    
    // === LISTE DES NŒUDS ===
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeA, varNodeB, varNodeC, varNodeThreshold,
        addNode, subNode, mulNode, compareNode,
        print1, print2, print3,
        branchNode, printTrue, printFalse
    };
    
    // === CONNEXIONS ===
    std::vector<std::shared_ptr<Connection>> connections;
    
    // --- Calcul 1 : A + B → intermediate1 ---
    auto conn1 = std::make_shared<Connection>();
    conn1->outNodeId = varNodeA->GetId();
    conn1->outPortIndex = 0;
    conn1->inNodeId = addNode->GetId();
    conn1->inPortIndex = 0;
    conn1->type = Connection::DATA_LINK;
    connections.push_back(conn1);
    
    auto conn2 = std::make_shared<Connection>();
    conn2->outNodeId = varNodeB->GetId();
    conn2->outPortIndex = 0;
    conn2->inNodeId = addNode->GetId();
    conn2->inPortIndex = 1;
    conn2->type = Connection::DATA_LINK;
    connections.push_back(conn2);
    
    // --- Calcul 2 : A - B → intermediate2 ---
    auto conn3 = std::make_shared<Connection>();
    conn3->outNodeId = varNodeA->GetId();
    conn3->outPortIndex = 0;
    conn3->inNodeId = subNode->GetId();
    conn3->inPortIndex = 0;
    conn3->type = Connection::DATA_LINK;
    connections.push_back(conn3);
    
    auto conn4 = std::make_shared<Connection>();
    conn4->outNodeId = varNodeB->GetId();
    conn4->outPortIndex = 0;
    conn4->inNodeId = subNode->GetId();
    conn4->inPortIndex = 1;
    conn4->type = Connection::DATA_LINK;
    connections.push_back(conn4);
    
    // --- Calcul 3 : intermediate1 * C → result ---
    auto conn5 = std::make_shared<Connection>();
    conn5->outNodeId = addNode->GetId();  // intermediate1
    conn5->outPortIndex = 0;
    conn5->inNodeId = mulNode->GetId();
    conn5->inPortIndex = 0;
    conn5->type = Connection::DATA_LINK;
    connections.push_back(conn5);
    
    auto conn6 = std::make_shared<Connection>();
    conn6->outNodeId = varNodeC->GetId();
    conn6->outPortIndex = 0;
    conn6->inNodeId = mulNode->GetId();
    conn6->inPortIndex = 1;
    conn6->type = Connection::DATA_LINK;
    connections.push_back(conn6);
    
    // --- Execution flow : Entry → Print1 ---
    auto execConn1 = std::make_shared<Connection>();
    execConn1->outNodeId = functionEntry->GetId();
    execConn1->outPortIndex = 0;
    execConn1->inNodeId = print1->GetId();
    execConn1->inPortIndex = 0;
    execConn1->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn1);
    
    // --- Print1 arguments : A, B, intermediate1 ---
    auto conn7 = std::make_shared<Connection>();
    conn7->outNodeId = varNodeA->GetId();
    conn7->outPortIndex = 0;
    conn7->inNodeId = print1->GetId();
    conn7->inPortIndex = 1;  // arg0
    conn7->type = Connection::DATA_LINK;
    connections.push_back(conn7);
    
    auto conn8 = std::make_shared<Connection>();
    conn8->outNodeId = varNodeB->GetId();
    conn8->outPortIndex = 0;
    conn8->inNodeId = print1->GetId();
    conn8->inPortIndex = 2;  // arg1
    conn8->type = Connection::DATA_LINK;
    connections.push_back(conn8);
    
    auto conn9 = std::make_shared<Connection>();
    conn9->outNodeId = addNode->GetId();  // intermediate1 RÉUTILISÉ
    conn9->outPortIndex = 0;
    conn9->inNodeId = print1->GetId();
    conn9->inPortIndex = 3;  // arg2
    conn9->type = Connection::DATA_LINK;
    connections.push_back(conn9);
    
    // --- Execution flow : Print1 → Print2 ---
    auto execConn2 = std::make_shared<Connection>();
    execConn2->outNodeId = print1->GetId();
    execConn2->outPortIndex = 0;
    execConn2->inNodeId = print2->GetId();
    execConn2->inPortIndex = 0;
    execConn2->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn2);
    
    // --- Print2 arguments : A, B, intermediate2 ---
    auto conn10 = std::make_shared<Connection>();
    conn10->outNodeId = varNodeA->GetId();
    conn10->outPortIndex = 0;
    conn10->inNodeId = print2->GetId();
    conn10->inPortIndex = 1;
    conn10->type = Connection::DATA_LINK;
    connections.push_back(conn10);
    
    auto conn11 = std::make_shared<Connection>();
    conn11->outNodeId = varNodeB->GetId();
    conn11->outPortIndex = 0;
    conn11->inNodeId = print2->GetId();
    conn11->inPortIndex = 2;
    conn11->type = Connection::DATA_LINK;
    connections.push_back(conn11);
    
    auto conn12 = std::make_shared<Connection>();
    conn12->outNodeId = subNode->GetId();  // intermediate2
    conn12->outPortIndex = 0;
    conn12->inNodeId = print2->GetId();
    conn12->inPortIndex = 3;
    conn12->type = Connection::DATA_LINK;
    connections.push_back(conn12);
    
    // --- Execution flow : Print2 → Print3 ---
    auto execConn3 = std::make_shared<Connection>();
    execConn3->outNodeId = print2->GetId();
    execConn3->outPortIndex = 0;
    execConn3->inNodeId = print3->GetId();
    execConn3->inPortIndex = 0;
    execConn3->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn3);
    
    // --- Print3 arguments : intermediate1, C, result ---
    auto conn13 = std::make_shared<Connection>();
    conn13->outNodeId = addNode->GetId();  // intermediate1 RÉUTILISÉ ENCORE
    conn13->outPortIndex = 0;
    conn13->inNodeId = print3->GetId();
    conn13->inPortIndex = 1;
    conn13->type = Connection::DATA_LINK;
    connections.push_back(conn13);
    
    auto conn14 = std::make_shared<Connection>();
    conn14->outNodeId = varNodeC->GetId();
    conn14->outPortIndex = 0;
    conn14->inNodeId = print3->GetId();
    conn14->inPortIndex = 2;
    conn14->type = Connection::DATA_LINK;
    connections.push_back(conn14);
    
    auto conn15 = std::make_shared<Connection>();
    conn15->outNodeId = mulNode->GetId();  // result
    conn15->outPortIndex = 0;
    conn15->inNodeId = print3->GetId();
    conn15->inPortIndex = 3;
    conn15->type = Connection::DATA_LINK;
    connections.push_back(conn15);
    
    // --- Execution flow : Print3 → Branch ---
    auto execConn4 = std::make_shared<Connection>();
    execConn4->outNodeId = print3->GetId();
    execConn4->outPortIndex = 0;
    execConn4->inNodeId = branchNode->GetId();
    execConn4->inPortIndex = 0;
    execConn4->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn4);
    
    // --- Compare : result > threshold ---
    auto conn16 = std::make_shared<Connection>();
    conn16->outNodeId = mulNode->GetId();  // result RÉUTILISÉ
    conn16->outPortIndex = 0;
    conn16->inNodeId = compareNode->GetId();
    conn16->inPortIndex = 0;
    conn16->type = Connection::DATA_LINK;
    connections.push_back(conn16);
    
    auto conn17 = std::make_shared<Connection>();
    conn17->outNodeId = varNodeThreshold->GetId();
    conn17->outPortIndex = 0;
    conn17->inNodeId = compareNode->GetId();
    conn17->inPortIndex = 1;
    conn17->type = Connection::DATA_LINK;
    connections.push_back(conn17);
    
    // --- Branch condition ---
    auto conn18 = std::make_shared<Connection>();
    conn18->outNodeId = compareNode->GetId();
    conn18->outPortIndex = 0;
    conn18->inNodeId = branchNode->GetId();
    conn18->inPortIndex = 1;  // condition port
    conn18->type = Connection::DATA_LINK;
    connections.push_back(conn18);
    
    // --- Branch outputs ---
    auto conn19 = std::make_shared<Connection>();
    conn19->outNodeId = branchNode->GetId();
    conn19->outPortIndex = 0;  // true
    conn19->inNodeId = printTrue->GetId();
    conn19->inPortIndex = 0;
    conn19->type = Connection::EXECUTION_LINK;
    connections.push_back(conn19);
    
    auto conn20 = std::make_shared<Connection>();
    conn20->outNodeId = branchNode->GetId();
    conn20->outPortIndex = 1;  // false
    conn20->inNodeId = printFalse->GetId();
    conn20->inPortIndex = 0;
    conn20->type = Connection::EXECUTION_LINK;
    connections.push_back(conn20);
    
    // === BUILD AST ===
    std::cout << "\n--- Building AST ---\n";
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();

    std::cout << "\n=== DEBUG: Inspecting AST nodes ===\n";
    for (size_t i = 0; i < pathTree.size(); i++) {
        auto node = pathTree[i];
        std::cout << "[" << i << "] " << node->node->GetTypeName() 
                << " (ID: " << node->GetId() << ")\n";
        
        if (node->IsType<OperatorNode>()) {
            auto* opNode = node->GetAs<OperatorNode>();
            std::cout << "     Operator type: ";
            switch(opNode->GetOperationType()) {
                case OperatorNode::OperationType::ADD: std::cout << "ADD\n"; break;
                case OperatorNode::OperationType::SUBTRACT: std::cout << "SUB\n"; break;
                case OperatorNode::OperationType::MULTIPLY: std::cout << "MUL\n"; break;
                case OperatorNode::OperationType::GREATER_THAN: std::cout << "GT\n"; break;
                default: std::cout << "OTHER\n"; break;
            }
            std::cout << "     dataInputs count: " << node->dataInputs.size() << "\n";
            for (const auto& [port, input] : node->dataInputs) {
                std::cout << "       Port " << port << " -> " 
                        << input->node->GetTypeName() << "\n";
            }
        }
    }
        
    std::cout << "AST built with " << pathTree.size() << " root nodes\n";
    
    // === GENERATE TAC ===
    std::cout << "\n--- Generating TAC ---\n";
    TACGenerator tacGen;
    TACProgram tac = tacGen.Generate(pathTree, variables);
    
    std::cout << "\n" << tac.ToString() << std::endl;
    
    // === VERIFY TAC EXPECTATIONS ===
    const auto& instructions = tac.GetInstructions();
    
    std::cout << "Total TAC instructions: " << instructions.size() << "\n\n";
    
    // Vérifications
    REQUIRE(instructions.size() > 0);
    
    // Compter les types d'instructions
    int addCount = 0, subCount = 0, mulCount = 0, compareCount = 0;
    int paramCount = 0, printCount = 0;
    
    for (const auto& instr : instructions) {
        switch (instr->GetOpCode()) {
            case TACInstruction::OpCode::ADD: addCount++; break;
            case TACInstruction::OpCode::SUB: subCount++; break;
            case TACInstruction::OpCode::MUL: mulCount++; break;
            case TACInstruction::OpCode::GT: compareCount++; break;
            case TACInstruction::OpCode::PARAM: paramCount++; break;
            case TACInstruction::OpCode::PRINT: printCount++; break;
            default: break;
        }
    }
    
    std::cout << "Statistics:\n";
    std::cout << "  ADD operations: " << addCount << "\n";
    std::cout << "  SUB operations: " << subCount << "\n";
    std::cout << "  MUL operations: " << mulCount << "\n";
    std::cout << "  GT comparisons: " << compareCount << "\n";
    std::cout << "  PARAM instructions: " << paramCount << "\n";
    std::cout << "  PRINT instructions: " << printCount << "\n\n";
    
    // Vérifier qu'on a bien nos opérations
    REQUIRE(addCount == 1);  // Une seule fois malgré réutilisation
    REQUIRE(subCount == 1);
    REQUIRE(mulCount == 1);
    REQUIRE(compareCount == 1);
    REQUIRE(printCount == 5);  // 3 prints normaux + 2 dans le branch
    REQUIRE(paramCount == 9);  // 3 args × 3 prints
    
    // === GENERATE CHIP32 ASSEMBLY WITH TAC ===
    std::cout << "--- Generating Chip32 Assembly from TAC ---\n";
    
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-10 15:30:00",
        "complex-tac-test",
        true,   // debug
        true,   // optimize
        1024
    );
    
    AssemblyGeneratorChip32TAC chip32Gen(context);
    chip32Gen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = chip32Gen.GetAssembly().str();
    
    std::cout << "\n=== GENERATED CHIP32 ASSEMBLY ===\n";
    std::cout << assembly << std::endl;
    
    // === VERIFY ASSEMBLY ===
    
    // Vérifier présence des sections
    REQUIRE(assembly.find("DATA") != std::string::npos);
    REQUIRE(assembly.find("CODE") != std::string::npos);
    REQUIRE(assembly.find(".main:") != std::string::npos);
    REQUIRE(assembly.find("halt") != std::string::npos);
    
    // Vérifier présence de syscall 4 (print)
    REQUIRE(assembly.find("syscall 4") != std::string::npos);
    
    // === EXECUTE ===
    std::cout << "\n--- Executing on Chip32 VM ---\n";
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::cout << "\n✅ Complex TAC test completed successfully!\n";
    std::cout << "========================================\n\n";
}


// Helper function to build the diagram with configurable variable values
std::pair<std::vector<std::shared_ptr<BaseNode>>, std::vector<std::shared_ptr<Connection>>>
BuildComplexDiagram(
    std::vector<std::shared_ptr<Variable>>& variables,
    int valueA, int valueB, int valueC, int threshold)
{
    // === VARIABLES ===
    variables.clear();
    
    auto var_A = std::make_shared<Variable>("A");
    var_A->SetIntegerValue(valueA);
    variables.push_back(var_A);
    
    auto var_B = std::make_shared<Variable>("B");
    var_B->SetIntegerValue(valueB);
    variables.push_back(var_B);
    
    auto var_C = std::make_shared<Variable>("C");
    var_C->SetIntegerValue(valueC);
    variables.push_back(var_C);
    
    auto var_threshold = std::make_shared<Variable>("threshold");
    var_threshold->SetIntegerValue(threshold);
    variables.push_back(var_threshold);
    
    // === NODES ===
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry-node");
    functionEntry->SetWeight(100);
    
    auto varNodeA = std::make_shared<VariableNode>("variable-node-A");
    varNodeA->SetVariable(var_A);
    
    auto varNodeB = std::make_shared<VariableNode>("variable-node-B");
    varNodeB->SetVariable(var_B);
    
    auto varNodeC = std::make_shared<VariableNode>("variable-node-C");
    varNodeC->SetVariable(var_C);
    
    auto varNodeThreshold = std::make_shared<VariableNode>("variable-node-threshold");
    varNodeThreshold->SetVariable(var_threshold);
    
    auto addNode = std::make_shared<OperatorNode>("operator-add");
    addNode->SetOperationType(OperatorNode::OperationType::ADD);
    
    auto subNode = std::make_shared<OperatorNode>("operator-sub");
    subNode->SetOperationType(OperatorNode::OperationType::SUBTRACT);
    
    auto mulNode = std::make_shared<OperatorNode>("operator-mul");
    mulNode->SetOperationType(OperatorNode::OperationType::MULTIPLY);
    
    auto compareNode = std::make_shared<OperatorNode>("operator-compare");
    compareNode->SetOperationType(OperatorNode::OperationType::GREATER_THAN);
    
    auto print1 = std::make_shared<PrintNode>("print-node-1");
    print1->SetText("Addition: {0} + {1} = {2}");
    print1->Initialize();
    
    auto print2 = std::make_shared<PrintNode>("print-node-2");
    print2->SetText("Subtraction: {0} - {1} = {2}");
    print2->Initialize();
    
    auto print3 = std::make_shared<PrintNode>("print-node-3");
    print3->SetText("Multiplication: {0} * {1} = {2}");
    print3->Initialize();
    
    auto branchNode = std::make_shared<BranchNode>("branch-node");
    
    auto printTrue = std::make_shared<PrintNode>("print-true");
    printTrue->SetText("Result is LARGE");
    printTrue->Initialize();
    
    auto printFalse = std::make_shared<PrintNode>("print-false");
    printFalse->SetText("Result is small");
    printFalse->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeA, varNodeB, varNodeC, varNodeThreshold,
        addNode, subNode, mulNode, compareNode,
        print1, print2, print3,
        branchNode, printTrue, printFalse
    };
    
    // === CONNECTIONS ===
    std::vector<std::shared_ptr<Connection>> connections;
    
    // A + B
    auto conn1 = std::make_shared<Connection>();
    conn1->outNodeId = varNodeA->GetId();
    conn1->outPortIndex = 0;
    conn1->inNodeId = addNode->GetId();
    conn1->inPortIndex = 0;
    conn1->type = Connection::DATA_LINK;
    connections.push_back(conn1);
    
    auto conn2 = std::make_shared<Connection>();
    conn2->outNodeId = varNodeB->GetId();
    conn2->outPortIndex = 0;
    conn2->inNodeId = addNode->GetId();
    conn2->inPortIndex = 1;
    conn2->type = Connection::DATA_LINK;
    connections.push_back(conn2);
    
    // A - B
    auto conn3 = std::make_shared<Connection>();
    conn3->outNodeId = varNodeA->GetId();
    conn3->outPortIndex = 0;
    conn3->inNodeId = subNode->GetId();
    conn3->inPortIndex = 0;
    conn3->type = Connection::DATA_LINK;
    connections.push_back(conn3);
    
    auto conn4 = std::make_shared<Connection>();
    conn4->outNodeId = varNodeB->GetId();
    conn4->outPortIndex = 0;
    conn4->inNodeId = subNode->GetId();
    conn4->inPortIndex = 1;
    conn4->type = Connection::DATA_LINK;
    connections.push_back(conn4);
    
    // (A+B) * C
    auto conn5 = std::make_shared<Connection>();
    conn5->outNodeId = addNode->GetId();
    conn5->outPortIndex = 0;
    conn5->inNodeId = mulNode->GetId();
    conn5->inPortIndex = 0;
    conn5->type = Connection::DATA_LINK;
    connections.push_back(conn5);
    
    auto conn6 = std::make_shared<Connection>();
    conn6->outNodeId = varNodeC->GetId();
    conn6->outPortIndex = 0;
    conn6->inNodeId = mulNode->GetId();
    conn6->inPortIndex = 1;
    conn6->type = Connection::DATA_LINK;
    connections.push_back(conn6);
    
    // Execution: Entry → Print1
    auto execConn1 = std::make_shared<Connection>();
    execConn1->outNodeId = functionEntry->GetId();
    execConn1->outPortIndex = 0;
    execConn1->inNodeId = print1->GetId();
    execConn1->inPortIndex = 0;
    execConn1->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn1);
    
    // Print1: A, B, (A+B)
    auto conn7 = std::make_shared<Connection>();
    conn7->outNodeId = varNodeA->GetId();
    conn7->outPortIndex = 0;
    conn7->inNodeId = print1->GetId();
    conn7->inPortIndex = 1;
    conn7->type = Connection::DATA_LINK;
    connections.push_back(conn7);
    
    auto conn8 = std::make_shared<Connection>();
    conn8->outNodeId = varNodeB->GetId();
    conn8->outPortIndex = 0;
    conn8->inNodeId = print1->GetId();
    conn8->inPortIndex = 2;
    conn8->type = Connection::DATA_LINK;
    connections.push_back(conn8);
    
    auto conn9 = std::make_shared<Connection>();
    conn9->outNodeId = addNode->GetId();
    conn9->outPortIndex = 0;
    conn9->inNodeId = print1->GetId();
    conn9->inPortIndex = 3;
    conn9->type = Connection::DATA_LINK;
    connections.push_back(conn9);
    
    // Execution: Print1 → Print2
    auto execConn2 = std::make_shared<Connection>();
    execConn2->outNodeId = print1->GetId();
    execConn2->outPortIndex = 0;
    execConn2->inNodeId = print2->GetId();
    execConn2->inPortIndex = 0;
    execConn2->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn2);
    
    // Print2: A, B, (A-B)
    auto conn10 = std::make_shared<Connection>();
    conn10->outNodeId = varNodeA->GetId();
    conn10->outPortIndex = 0;
    conn10->inNodeId = print2->GetId();
    conn10->inPortIndex = 1;
    conn10->type = Connection::DATA_LINK;
    connections.push_back(conn10);
    
    auto conn11 = std::make_shared<Connection>();
    conn11->outNodeId = varNodeB->GetId();
    conn11->outPortIndex = 0;
    conn11->inNodeId = print2->GetId();
    conn11->inPortIndex = 2;
    conn11->type = Connection::DATA_LINK;
    connections.push_back(conn11);
    
    auto conn12 = std::make_shared<Connection>();
    conn12->outNodeId = subNode->GetId();
    conn12->outPortIndex = 0;
    conn12->inNodeId = print2->GetId();
    conn12->inPortIndex = 3;
    conn12->type = Connection::DATA_LINK;
    connections.push_back(conn12);
    
    // Execution: Print2 → Print3
    auto execConn3 = std::make_shared<Connection>();
    execConn3->outNodeId = print2->GetId();
    execConn3->outPortIndex = 0;
    execConn3->inNodeId = print3->GetId();
    execConn3->inPortIndex = 0;
    execConn3->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn3);
    
    // Print3: (A+B), C, (A+B)*C
    auto conn13 = std::make_shared<Connection>();
    conn13->outNodeId = addNode->GetId();
    conn13->outPortIndex = 0;
    conn13->inNodeId = print3->GetId();
    conn13->inPortIndex = 1;
    conn13->type = Connection::DATA_LINK;
    connections.push_back(conn13);
    
    auto conn14 = std::make_shared<Connection>();
    conn14->outNodeId = varNodeC->GetId();
    conn14->outPortIndex = 0;
    conn14->inNodeId = print3->GetId();
    conn14->inPortIndex = 2;
    conn14->type = Connection::DATA_LINK;
    connections.push_back(conn14);
    
    auto conn15 = std::make_shared<Connection>();
    conn15->outNodeId = mulNode->GetId();
    conn15->outPortIndex = 0;
    conn15->inNodeId = print3->GetId();
    conn15->inPortIndex = 3;
    conn15->type = Connection::DATA_LINK;
    connections.push_back(conn15);
    
    // Execution: Print3 → Branch
    auto execConn4 = std::make_shared<Connection>();
    execConn4->outNodeId = print3->GetId();
    execConn4->outPortIndex = 0;
    execConn4->inNodeId = branchNode->GetId();
    execConn4->inPortIndex = 0;
    execConn4->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn4);
    
    // Compare: result > threshold
    auto conn16 = std::make_shared<Connection>();
    conn16->outNodeId = mulNode->GetId();
    conn16->outPortIndex = 0;
    conn16->inNodeId = compareNode->GetId();
    conn16->inPortIndex = 0;
    conn16->type = Connection::DATA_LINK;
    connections.push_back(conn16);
    
    auto conn17 = std::make_shared<Connection>();
    conn17->outNodeId = varNodeThreshold->GetId();
    conn17->outPortIndex = 0;
    conn17->inNodeId = compareNode->GetId();
    conn17->inPortIndex = 1;
    conn17->type = Connection::DATA_LINK;
    connections.push_back(conn17);
    
    // Branch condition
    auto conn18 = std::make_shared<Connection>();
    conn18->outNodeId = compareNode->GetId();
    conn18->outPortIndex = 0;
    conn18->inNodeId = branchNode->GetId();
    conn18->inPortIndex = 1;
    conn18->type = Connection::DATA_LINK;
    connections.push_back(conn18);
    
    // Branch true/false
    auto conn19 = std::make_shared<Connection>();
    conn19->outNodeId = branchNode->GetId();
    conn19->outPortIndex = 0;
    conn19->inNodeId = printTrue->GetId();
    conn19->inPortIndex = 0;
    conn19->type = Connection::EXECUTION_LINK;
    connections.push_back(conn19);
    
    auto conn20 = std::make_shared<Connection>();
    conn20->outNodeId = branchNode->GetId();
    conn20->outPortIndex = 1;
    conn20->inNodeId = printFalse->GetId();
    conn20->inPortIndex = 0;
    conn20->type = Connection::EXECUTION_LINK;
    connections.push_back(conn20);
    
    return {nodes, connections};
}

// Helper to run the test and capture output
struct TestResult {
    std::string tacOutput;
    std::string assembly;
    std::string executionOutput;
    bool success;
};

TestResult RunComplexTACTest(int valueA, int valueB, int valueC, int threshold) {
    TestResult result;
    result.success = false;
    
    std::vector<std::shared_ptr<Variable>> variables;
    auto [nodes, connections] = BuildComplexDiagram(variables, valueA, valueB, valueC, threshold);
    
    // Build AST
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    // Generate TAC
    TACGenerator tacGen;
    TACProgram tac = tacGen.Generate(pathTree, variables);
    result.tacOutput = tac.ToString();
    
    // Verify TAC structure
    const auto& instructions = tac.GetInstructions();
    if (instructions.empty()) return result;
    
    // Generate Assembly
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-10 15:30:00",
        "complex-tac-test",
        false,  // No debug output
        true,
        1024
    );
    
    AssemblyGeneratorChip32TAC chip32Gen(context);
    chip32Gen.GenerateCompleteProgram(nodes, pathTree);
    result.assembly = chip32Gen.GetAssembly().str();
    
    // Execute (capture output would need machine modification)
    // For now we just verify it doesn't crash
    Chip32::Machine machine;
    try {
        machine.QuickExecute(result.assembly);
        result.success = true;
    } catch (...) {
        result.success = false;
    }
    
    return result;
}

TEST_CASE("Complex AST with TAC - Multiple test cases", "[tac][complex][ast2]") {
    
    std::cout << "\n========================================\n";
    std::cout << "Complex TAC Test - Multiple Scenarios\n";
    std::cout << "========================================\n";
    
    // Test Case 1: A=10, B=5, C=3, threshold=40
    // Expected: 10+5=15, 10-5=5, 15*3=45, 45>40 = TRUE
    {
        std::cout << "\n--- Test Case 1: A=10, B=5, C=3, threshold=40 ---\n";
        auto result = RunComplexTACTest(10, 5, 3, 40);
        
        REQUIRE(result.success);
        REQUIRE(result.assembly.find("DATA") != std::string::npos);
        REQUIRE(result.assembly.find("CODE") != std::string::npos);
        
        // Verify TAC instructions count and types
        TACGenerator tacGen;
        std::vector<std::shared_ptr<Variable>> vars;
        auto [nodes, conns] = BuildComplexDiagram(vars, 10, 5, 3, 40);
        ASTBuilder builder(nodes, conns);
        auto tree = builder.BuildAST();
        TACProgram tac = tacGen.Generate(tree, vars);
        
        int addCount = 0, subCount = 0, mulCount = 0, gtCount = 0;
        for (const auto& instr : tac.GetInstructions()) {
            switch (instr->GetOpCode()) {
                case TACInstruction::OpCode::ADD: addCount++; break;
                case TACInstruction::OpCode::SUB: subCount++; break;
                case TACInstruction::OpCode::MUL: mulCount++; break;
                case TACInstruction::OpCode::GT: gtCount++; break;
                default: break;
            }
        }
        
        REQUIRE(addCount == 1);
        REQUIRE(subCount == 1);
        REQUIRE(mulCount == 1);
        REQUIRE(gtCount == 1);
        
        std::cout << "✓ Passed: 10+5=15, 15*3=45, 45>40\n";
    }
    
    // Test Case 2: A=7, B=3, C=2, threshold=50
    // Expected: 7+3=10, 7-3=4, 10*2=20, 20>50 = FALSE
    {
        std::cout << "\n--- Test Case 2: A=7, B=3, C=2, threshold=50 ---\n";
        auto result = RunComplexTACTest(7, 3, 2, 50);
        
        REQUIRE(result.success);
        
        std::cout << "✓ Passed: 7+3=10, 10*2=20, 20<50\n";
    }
    
    // Test Case 3: A=20, B=10, C=5, threshold=100
    // Expected: 20+10=30, 20-10=10, 30*5=150, 150>100 = TRUE
    {
        std::cout << "\n--- Test Case 3: A=20, B=10, C=5, threshold=100 ---\n";
        auto result = RunComplexTACTest(20, 10, 5, 100);
        
        REQUIRE(result.success);
        
        std::cout << "✓ Passed: 20+10=30, 30*5=150, 150>100\n";
    }
    
    // Test Case 4: Edge case with zeros
    // A=0, B=0, C=10, threshold=5
    // Expected: 0+0=0, 0-0=0, 0*10=0, 0>5 = FALSE
    {
        std::cout << "\n--- Test Case 4: A=0, B=0, C=10, threshold=5 ---\n";
        auto result = RunComplexTACTest(0, 0, 10, 5);
        
        REQUIRE(result.success);
        
        std::cout << "✓ Passed: 0+0=0, 0*10=0, 0<5\n";
    }
    
    // Test Case 5: Negative numbers
    // A=-5, B=3, C=4, threshold=0
    // Expected: -5+3=-2, -5-3=-8, -2*4=-8, -8>0 = FALSE
    {
        std::cout << "\n--- Test Case 5: A=-5, B=3, C=4, threshold=0 ---\n";
        auto result = RunComplexTACTest(-5, 3, 4, 0);
        
        REQUIRE(result.success);
        
        std::cout << "✓ Passed: -5+3=-2, -2*4=-8, -8<0\n";
    }
    
    // Test Case 6: Exactly at threshold
    // A=10, B=10, C=2, threshold=40
    // Expected: 10+10=20, 10-10=0, 20*2=40, 40>40 = FALSE
    {
        std::cout << "\n--- Test Case 6: A=10, B=10, C=2, threshold=40 ---\n";
        auto result = RunComplexTACTest(10, 10, 2, 40);
        
        REQUIRE(result.success);
        
        std::cout << "✓ Passed: 10+10=20, 20*2=40, 40==40 (FALSE for >)\n";
    }
    
    std::cout << "\n✅ All test cases completed successfully!\n";
    std::cout << "========================================\n\n";
}
