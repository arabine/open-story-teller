// ===================================================================
// core/tests/test_loops.cpp
// Tests unitaires pour ForLoop, WhileLoop, Break et Continue
// ===================================================================

#include <catch2/catch_test_macros.hpp>
#include "for_loop_node.h"
#include "while_loop_node.h"
#include "break_node.h"
#include "continue_node.h"
#include "variable_node.h"
#include "function_entry_node.h"
#include "operator_node.h"
#include "print_node.h"
#include "connection.h"
#include "ast_builder.h"
#include "assembly_generator_chip32_tac.h"
#include "chip32_machine.h"
#include "variable.h"

// ===================================================================
// TEST 1 : ForLoopNode simple (0 à 5)
// ===================================================================
TEST_CASE("Simple ForLoop counting 0 to 5", "[loop][for]") {
    std::cout << "\n=== Test: Simple ForLoop 0 to 5 ===\n";
    
    std::vector<std::shared_ptr<Variable>> variables;
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto forLoop = std::make_shared<ForLoopNode>("for-loop");
    forLoop->SetStartIndex(0);
    forLoop->SetEndIndex(5);
    forLoop->SetStep(1);
    forLoop->Initialize();
    
    auto printNode = std::make_shared<PrintNode>("print-loop");
    printNode->SetText("Iteration");
    printNode->Initialize();
    
    auto printEnd = std::make_shared<PrintNode>("print-end");
    printEnd->SetText("Loop completed!");
    printEnd->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        forLoop,
        printNode,
        printEnd
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → ForLoop
    auto execConn1 = std::make_shared<Connection>();
    execConn1->outNodeId = functionEntry->GetId();
    execConn1->outPortIndex = 0;
    execConn1->inNodeId = forLoop->GetId();
    execConn1->inPortIndex = 0;
    execConn1->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn1);
    
    // Execution: ForLoop.body → Print
    auto execConn2 = std::make_shared<Connection>();
    execConn2->outNodeId = forLoop->GetId();
    execConn2->outPortIndex = 0;  // Loop Body
    execConn2->inNodeId = printNode->GetId();
    execConn2->inPortIndex = 0;
    execConn2->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn2);
    
    // Execution: ForLoop.completed → PrintEnd
    auto execConn3 = std::make_shared<Connection>();
    execConn3->outNodeId = forLoop->GetId();
    execConn3->outPortIndex = 1;  // Completed
    execConn3->inNodeId = printEnd->GetId();
    execConn3->inPortIndex = 0;
    execConn3->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn3);
    
    // Build AST
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    // Generate Assembly
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-20 10:00:00",
        "test-for-loop",
        true,
        true,
        1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    // Verify
    REQUIRE(assembly.find("for_start") != std::string::npos);
    REQUIRE(assembly.find("for_end") != std::string::npos);
    
    // Execute
    std::cout << "\n--- Execution Output ---\n";
    std::cout << "Expected: 5 iterations + completion message\n";
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::cout << "\n✓ Test passed\n";
}

// ===================================================================
// TEST 2 : WhileLoop avec condition variable
// ===================================================================
TEST_CASE("WhileLoop with variable condition", "[loop][while]") {
    std::cout << "\n=== Test: WhileLoop with condition ===\n";
    
    std::vector<std::shared_ptr<Variable>> variables;
    
    auto varCounter = std::make_shared<Variable>("counter");
    varCounter->SetIntegerValue(0);
    variables.push_back(varCounter);
    
    auto varLimit = std::make_shared<Variable>("limit");
    varLimit->SetIntegerValue(3);
    variables.push_back(varLimit);
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto varNodeCounter = std::make_shared<VariableNode>("var-counter");
    varNodeCounter->SetVariable(varCounter);
    
    auto varNodeLimit = std::make_shared<VariableNode>("var-limit");
    varNodeLimit->SetVariable(varLimit);
    
    // Condition: counter < limit
    auto compareNode = std::make_shared<OperatorNode>("compare-lt");
    compareNode->SetOperationType(OperatorNode::OperationType::LESS_THAN);
    compareNode->Initialize();
    
    auto whileLoop = std::make_shared<WhileLoopNode>("while-loop");
    whileLoop->Initialize();
    
    auto printNode = std::make_shared<PrintNode>("print-loop");
    printNode->SetText("Counter value");
    printNode->Initialize();
    
    auto printEnd = std::make_shared<PrintNode>("print-end");
    printEnd->SetText("While loop completed!");
    printEnd->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeCounter,
        varNodeLimit,
        compareNode,
        whileLoop,
        printNode,
        printEnd
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → While
    auto execConn1 = std::make_shared<Connection>();
    execConn1->outNodeId = functionEntry->GetId();
    execConn1->outPortIndex = 0;
    execConn1->inNodeId = whileLoop->GetId();
    execConn1->inPortIndex = 0;
    execConn1->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn1);
    
    // Data: counter → compare.in1
    auto dataConn1 = std::make_shared<Connection>();
    dataConn1->outNodeId = varNodeCounter->GetId();
    dataConn1->outPortIndex = 0;
    dataConn1->inNodeId = compareNode->GetId();
    dataConn1->inPortIndex = 0;
    dataConn1->type = Connection::DATA_LINK;
    connections.push_back(dataConn1);
    
    // Data: limit → compare.in2
    auto dataConn2 = std::make_shared<Connection>();
    dataConn2->outNodeId = varNodeLimit->GetId();
    dataConn2->outPortIndex = 0;
    dataConn2->inNodeId = compareNode->GetId();
    dataConn2->inPortIndex = 1;
    dataConn2->type = Connection::DATA_LINK;
    connections.push_back(dataConn2);
    
    // Data: compare.out → while.condition
    auto dataConn3 = std::make_shared<Connection>();
    dataConn3->outNodeId = compareNode->GetId();
    dataConn3->outPortIndex = 0;
    dataConn3->inNodeId = whileLoop->GetId();
    dataConn3->inPortIndex = 1;
    dataConn3->type = Connection::DATA_LINK;
    connections.push_back(dataConn3);
    
    // Execution: While.body → Print
    auto execConn2 = std::make_shared<Connection>();
    execConn2->outNodeId = whileLoop->GetId();
    execConn2->outPortIndex = 0;
    execConn2->inNodeId = printNode->GetId();
    execConn2->inPortIndex = 0;
    execConn2->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn2);
    
    // Execution: While.completed → PrintEnd
    auto execConn3 = std::make_shared<Connection>();
    execConn3->outNodeId = whileLoop->GetId();
    execConn3->outPortIndex = 1;
    execConn3->inNodeId = printEnd->GetId();
    execConn3->inPortIndex = 0;
    execConn3->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn3);
    
    // Build and test
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    AssemblyGenerator::GeneratorContext context(
        variables, "2025-01-20", "test-while-loop", true, true, 1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    REQUIRE(assembly.find("while_start") != std::string::npos);
    REQUIRE(assembly.find("while_end") != std::string::npos);
    
    std::cout << "\n✓ Test passed\n";
}

// ===================================================================
// TEST 3 : ForLoop avec Break
// ===================================================================
TEST_CASE("ForLoop with Break at first iteration", "[loop][for][break]") {
    std::cout << "\n=== Test: ForLoop with Break ===\n";
    
    std::vector<std::shared_ptr<Variable>> variables;
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto forLoop = std::make_shared<ForLoopNode>("for-loop");
    forLoop->SetStartIndex(0);
    forLoop->SetEndIndex(10);
    forLoop->SetStep(1);
    forLoop->Initialize();
    
    auto printNode = std::make_shared<PrintNode>("print-iter");
    printNode->SetText("Iteration");
    printNode->Initialize();
    
    auto breakNode = std::make_shared<BreakNode>("break-node");
    breakNode->Initialize();
    
    auto printEnd = std::make_shared<PrintNode>("print-end");
    printEnd->SetText("Broke out of loop");
    printEnd->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        forLoop,
        printNode,
        breakNode,
        printEnd
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Entry → ForLoop
    auto execConn1 = std::make_shared<Connection>();
    execConn1->outNodeId = functionEntry->GetId();
    execConn1->outPortIndex = 0;
    execConn1->inNodeId = forLoop->GetId();
    execConn1->inPortIndex = 0;
    execConn1->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn1);
    
    // ForLoop.body → Print
    auto execConn2 = std::make_shared<Connection>();
    execConn2->outNodeId = forLoop->GetId();
    execConn2->outPortIndex = 0;
    execConn2->inNodeId = printNode->GetId();
    execConn2->inPortIndex = 0;
    execConn2->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn2);
    
    // Print → Break
    auto execConn3 = std::make_shared<Connection>();
    execConn3->outNodeId = printNode->GetId();
    execConn3->outPortIndex = 0;
    execConn3->inNodeId = breakNode->GetId();
    execConn3->inPortIndex = 0;
    execConn3->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn3);
    
    // ForLoop.completed → PrintEnd
    auto execConn4 = std::make_shared<Connection>();
    execConn4->outNodeId = forLoop->GetId();
    execConn4->outPortIndex = 1;
    execConn4->inNodeId = printEnd->GetId();
    execConn4->inPortIndex = 0;
    execConn4->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn4);
    
    // Build and generate
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    AssemblyGenerator::GeneratorContext context(
        variables, "2025-01-20", "test-break", true, true, 1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    REQUIRE(assembly.find("for_end") != std::string::npos);
    REQUIRE(assembly.find("jump") != std::string::npos);
    
    std::cout << "\n✓ Test passed\n";
}

// ===================================================================
// TEST 4 : ForLoop avec Continue
// ===================================================================
TEST_CASE("ForLoop with Continue", "[loop][for][continue]") {
    std::cout << "\n=== Test: ForLoop with Continue ===\n";
    
    std::vector<std::shared_ptr<Variable>> variables;
    
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto forLoop = std::make_shared<ForLoopNode>("for-loop");
    forLoop->SetStartIndex(0);
    forLoop->SetEndIndex(5);
    forLoop->Initialize();
    
    auto continueNode = std::make_shared<ContinueNode>("continue-node");
    continueNode->Initialize();
    
    auto printNode = std::make_shared<PrintNode>("print-odd");
    printNode->SetText("Should not print");
    printNode->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        forLoop,
        continueNode,
        printNode
    };
    
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Entry → ForLoop
    auto execConn1 = std::make_shared<Connection>();
    execConn1->outNodeId = functionEntry->GetId();
    execConn1->outPortIndex = 0;
    execConn1->inNodeId = forLoop->GetId();
    execConn1->inPortIndex = 0;
    execConn1->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn1);
    
    // ForLoop.body → Continue
    auto execConn2 = std::make_shared<Connection>();
    execConn2->outNodeId = forLoop->GetId();
    execConn2->outPortIndex = 0;
    execConn2->inNodeId = continueNode->GetId();
    execConn2->inPortIndex = 0;
    execConn2->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn2);
    
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    AssemblyGenerator::GeneratorContext context(
        variables, "2025-01-20", "test-continue", true, true, 1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    REQUIRE(assembly.find("for_continue") != std::string::npos);
    
    std::cout << "\n✓ Test passed\n";
}