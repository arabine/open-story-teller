// test_print_node.cpp
#include "catch.hpp"
#include "print_node.h"
#include "variable_node.h"
#include "function_entry_node.h"
#include "operator_node.h"
#include "connection.h"
#include "ast_builder.h"
#include "assembly_generator_chip32_tac.h"
#include "chip32_machine.h"
#include "variable.h"

// ===================================================================
// TEST 1 : Print simple sans argument
// ===================================================================
TEST_CASE("Print without arguments - TAC", "[print][simple][tac]") {
    std::cout << "\n=== Test: Print without arguments (TAC) ===\n";
    
    // Variables (vide pour ce test)
    std::vector<std::shared_ptr<Variable>> variables;
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto printNode = std::make_shared<PrintNode>("print-node");
    printNode->SetText("Hello World!");
    printNode->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        printNode
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → Print
    auto execConn = std::make_shared<Connection>();
    execConn->outNodeId = functionEntry->GetId();
    execConn->outPortIndex = 0;
    execConn->inNodeId = printNode->GetId();
    execConn->inPortIndex = 0;
    execConn->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn);
    
    // Build AST
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    // Generate Assembly using TAC
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-10 10:00:00",
        "test-print-no-args",
        true,  // debug (will show TAC)
        true,  // optimize
        1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    // Verify
    REQUIRE(assembly.find("DATA") != std::string::npos);
    REQUIRE(assembly.find("CODE") != std::string::npos);
    REQUIRE(assembly.find("Hello World!") != std::string::npos);
    REQUIRE(assembly.find("syscall 4") != std::string::npos);
    REQUIRE(assembly.find("halt") != std::string::npos);
    
    // Execute
    std::cout << "\n--- Execution Output ---\n";
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::cout << "\n✓ Test passed\n";
}

// ===================================================================
// TEST 2 : Print avec 4 arguments (maximum supporté)
// ===================================================================
TEST_CASE("Print with 4 arguments - TAC", "[print][args][tac]") {
    std::cout << "\n=== Test: Print with 4 arguments (TAC) ===\n";
    
    // Variables: A=10, B=5, C=3, D=2
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
    
    auto var_D = std::make_shared<Variable>("D");
    var_D->SetIntegerValue(2);
    variables.push_back(var_D);
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto varNodeA = std::make_shared<VariableNode>("var-node-A");
    varNodeA->SetVariable(var_A);
    
    auto varNodeB = std::make_shared<VariableNode>("var-node-B");
    varNodeB->SetVariable(var_B);
    
    auto varNodeC = std::make_shared<VariableNode>("var-node-C");
    varNodeC->SetVariable(var_C);
    
    auto varNodeD = std::make_shared<VariableNode>("var-node-D");
    varNodeD->SetVariable(var_D);
    
    auto printNode = std::make_shared<PrintNode>("print-node");
    printNode->SetText("Values: A={0}, B={1}, C={2}, D={3}");
    printNode->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeA, varNodeB, varNodeC, varNodeD,
        printNode
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → Print
    auto execConn = std::make_shared<Connection>();
    execConn->outNodeId = functionEntry->GetId();
    execConn->outPortIndex = 0;
    execConn->inNodeId = printNode->GetId();
    execConn->inPortIndex = 0;
    execConn->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn);
    
    // Data: A → Print.arg0 (port 1)
    auto dataConn1 = std::make_shared<Connection>();
    dataConn1->outNodeId = varNodeA->GetId();
    dataConn1->outPortIndex = 0;
    dataConn1->inNodeId = printNode->GetId();
    dataConn1->inPortIndex = 1;
    dataConn1->type = Connection::DATA_LINK;
    connections.push_back(dataConn1);
    
    // Data: B → Print.arg1 (port 2)
    auto dataConn2 = std::make_shared<Connection>();
    dataConn2->outNodeId = varNodeB->GetId();
    dataConn2->outPortIndex = 0;
    dataConn2->inNodeId = printNode->GetId();
    dataConn2->inPortIndex = 2;
    dataConn2->type = Connection::DATA_LINK;
    connections.push_back(dataConn2);
    
    // Data: C → Print.arg2 (port 3)
    auto dataConn3 = std::make_shared<Connection>();
    dataConn3->outNodeId = varNodeC->GetId();
    dataConn3->outPortIndex = 0;
    dataConn3->inNodeId = printNode->GetId();
    dataConn3->inPortIndex = 3;
    dataConn3->type = Connection::DATA_LINK;
    connections.push_back(dataConn3);
    
    // Data: D → Print.arg3 (port 4)
    auto dataConn4 = std::make_shared<Connection>();
    dataConn4->outNodeId = varNodeD->GetId();
    dataConn4->outPortIndex = 0;
    dataConn4->inNodeId = printNode->GetId();
    dataConn4->inPortIndex = 4;
    dataConn4->type = Connection::DATA_LINK;
    connections.push_back(dataConn4);
    
    // Build AST
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    // Generate Assembly using TAC
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-10 10:00:00",
        "test-print-4-args",
        true,  // debug (will show TAC)
        true,  // optimize
        1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    // Verify
    REQUIRE(assembly.find("DATA") != std::string::npos);
    REQUIRE(assembly.find("CODE") != std::string::npos);
    
    // Verify variable declarations
    REQUIRE(assembly.find("; A") != std::string::npos);
    REQUIRE(assembly.find("; B") != std::string::npos);
    REQUIRE(assembly.find("; C") != std::string::npos);
    REQUIRE(assembly.find("; D") != std::string::npos);
    
    // Verify format string conversion {0} → %d
    REQUIRE(assembly.find("{0}") == std::string::npos);
    REQUIRE(assembly.find("{1}") == std::string::npos);
    REQUIRE(assembly.find("{2}") == std::string::npos);
    REQUIRE(assembly.find("{3}") == std::string::npos);
    
    // Verify syscall with 4 arguments
    REQUIRE(assembly.find("lcons r1, 4") != std::string::npos);
    REQUIRE(assembly.find("syscall 4") != std::string::npos);
    
    // Execute
    std::cout << "\n--- Execution Output ---\n";
    std::cout << "Expected: Values: A=10, B=5, C=3, D=2\n";
    std::cout << "Actual:   ";
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::cout << "\n✓ Test passed\n";
}