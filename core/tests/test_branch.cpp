// ===================================================================
// test_branch_node.cpp
// Tests complets pour BranchNode avec différents types de conditions
// ===================================================================

#include <catch2/catch_test_macros.hpp>
#include "branch_node.h"
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
// TEST 1 : BranchNode avec condition booléenne directe (variable)
// ===================================================================
TEST_CASE("Branch with direct boolean variable", "[branch][boolean][variable]") {
    std::cout << "\n=== Test: Branch with direct boolean variable ===\n";
    
    // Variables
    std::vector<std::shared_ptr<Variable>> variables;
    
    // Condition = 1 (true)
    auto varCondition = std::make_shared<Variable>("condition");
    varCondition->SetIntegerValue(1);
    variables.push_back(varCondition);
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto varNode = std::make_shared<VariableNode>("var-condition");
    varNode->SetVariable(varCondition);
    
    auto branchNode = std::make_shared<BranchNode>("branch-node");
    branchNode->Initialize();
    
    auto printTrue = std::make_shared<PrintNode>("print-true");
    printTrue->SetText("TRUE branch executed");
    printTrue->Initialize();
    
    auto printFalse = std::make_shared<PrintNode>("print-false");
    printFalse->SetText("FALSE branch executed");
    printFalse->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNode,
        branchNode,
        printTrue,
        printFalse
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → Branch
    auto execConn = std::make_shared<Connection>();
    execConn->outNodeId = functionEntry->GetId();
    execConn->outPortIndex = 0;
    execConn->inNodeId = branchNode->GetId();
    execConn->inPortIndex = 0;
    execConn->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn);
    
    // Data: Variable → Branch.condition (port 1)
    auto dataConn = std::make_shared<Connection>();
    dataConn->outNodeId = varNode->GetId();
    dataConn->outPortIndex = 0;
    dataConn->inNodeId = branchNode->GetId();
    dataConn->inPortIndex = 1;
    dataConn->type = Connection::DATA_LINK;
    connections.push_back(dataConn);
    
    // Execution: Branch.true → PrintTrue
    auto execTrue = std::make_shared<Connection>();
    execTrue->outNodeId = branchNode->GetId();
    execTrue->outPortIndex = 0;
    execTrue->inNodeId = printTrue->GetId();
    execTrue->inPortIndex = 0;
    execTrue->type = Connection::EXECUTION_LINK;
    connections.push_back(execTrue);
    
    // Execution: Branch.false → PrintFalse
    auto execFalse = std::make_shared<Connection>();
    execFalse->outNodeId = branchNode->GetId();
    execFalse->outPortIndex = 1;
    execFalse->inNodeId = printFalse->GetId();
    execFalse->inPortIndex = 0;
    execFalse->type = Connection::EXECUTION_LINK;
    connections.push_back(execFalse);
    
    // Build AST
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    // Generate Assembly using TAC
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-10 10:00:00",
        "test-branch-bool-var",
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
    REQUIRE(assembly.find("DATA") != std::string::npos);
    REQUIRE(assembly.find("CODE") != std::string::npos);
    REQUIRE(assembly.find("branch_true") != std::string::npos);
    REQUIRE(assembly.find("branch_false") != std::string::npos);
    
    // Execute
    std::cout << "\n--- Execution Output ---\n";
    std::cout << "Expected: TRUE branch executed (condition = 1)\n";
    std::cout << "Actual:   ";
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::cout << "\n✓ Test passed\n";
}

// ===================================================================
// TEST 2 : BranchNode avec OperatorNode (comparaison)
// ===================================================================
TEST_CASE("Branch with OperatorNode comparison", "[branch][operator][comparison]") {
    std::cout << "\n=== Test: Branch with OperatorNode (A > B) ===\n";
    
    // Variables
    std::vector<std::shared_ptr<Variable>> variables;
    
    auto varA = std::make_shared<Variable>("A");
    varA->SetIntegerValue(10);
    variables.push_back(varA);
    
    auto varB = std::make_shared<Variable>("B");
    varB->SetIntegerValue(5);
    variables.push_back(varB);
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto varNodeA = std::make_shared<VariableNode>("var-A");
    varNodeA->SetVariable(varA);
    
    auto varNodeB = std::make_shared<VariableNode>("var-B");
    varNodeB->SetVariable(varB);
    
    auto operatorNode = std::make_shared<OperatorNode>("operator-gt");
    operatorNode->SetOperationType(OperatorNode::OperationType::GREATER_THAN);
    operatorNode->Initialize();
    
    auto branchNode = std::make_shared<BranchNode>("branch-node");
    branchNode->Initialize();
    
    auto printTrue = std::make_shared<PrintNode>("print-true");
    printTrue->SetText("A is greater than B");
    printTrue->Initialize();
    
    auto printFalse = std::make_shared<PrintNode>("print-false");
    printFalse->SetText("A is NOT greater than B");
    printFalse->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeA,
        varNodeB,
        operatorNode,
        branchNode,
        printTrue,
        printFalse
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Execution: Entry → Branch
    auto execConn = std::make_shared<Connection>();
    execConn->outNodeId = functionEntry->GetId();
    execConn->outPortIndex = 0;
    execConn->inNodeId = branchNode->GetId();
    execConn->inPortIndex = 0;
    execConn->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn);
    
    // Data: A → Operator.in1
    auto dataConn1 = std::make_shared<Connection>();
    dataConn1->outNodeId = varNodeA->GetId();
    dataConn1->outPortIndex = 0;
    dataConn1->inNodeId = operatorNode->GetId();
    dataConn1->inPortIndex = 0;
    dataConn1->type = Connection::DATA_LINK;
    connections.push_back(dataConn1);
    
    // Data: B → Operator.in2
    auto dataConn2 = std::make_shared<Connection>();
    dataConn2->outNodeId = varNodeB->GetId();
    dataConn2->outPortIndex = 0;
    dataConn2->inNodeId = operatorNode->GetId();
    dataConn2->inPortIndex = 1;
    dataConn2->type = Connection::DATA_LINK;
    connections.push_back(dataConn2);
    
    // Data: Operator.out → Branch.condition
    auto dataConn3 = std::make_shared<Connection>();
    dataConn3->outNodeId = operatorNode->GetId();
    dataConn3->outPortIndex = 0;
    dataConn3->inNodeId = branchNode->GetId();
    dataConn3->inPortIndex = 1;
    dataConn3->type = Connection::DATA_LINK;
    connections.push_back(dataConn3);
    
    // Execution: Branch.true → PrintTrue
    auto execTrue = std::make_shared<Connection>();
    execTrue->outNodeId = branchNode->GetId();
    execTrue->outPortIndex = 0;
    execTrue->inNodeId = printTrue->GetId();
    execTrue->inPortIndex = 0;
    execTrue->type = Connection::EXECUTION_LINK;
    connections.push_back(execTrue);
    
    // Execution: Branch.false → PrintFalse
    auto execFalse = std::make_shared<Connection>();
    execFalse->outNodeId = branchNode->GetId();
    execFalse->outPortIndex = 1;
    execFalse->inNodeId = printFalse->GetId();
    execFalse->inPortIndex = 0;
    execFalse->type = Connection::EXECUTION_LINK;
    connections.push_back(execFalse);
    
    // Build and generate
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    AssemblyGenerator::GeneratorContext context(
        variables, "2025-01-10", "test-branch-operator", true, true, 1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    // Verify TAC structure
    REQUIRE(assembly.find("gt") != std::string::npos);  // Greater Than opcode
    REQUIRE(assembly.find("branch_true") != std::string::npos);
    REQUIRE(assembly.find("branch_false") != std::string::npos);
    
    // Execute
    std::cout << "\n--- Execution Output ---\n";
    std::cout << "Expected: A is greater than B (10 > 5)\n";
    std::cout << "Actual:   ";
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::cout << "\n✓ Test passed\n";
}

// ===================================================================
// TEST 3 : Tester différentes valeurs (0 = false, autres = true)
// ===================================================================
TEST_CASE("Branch with various condition values", "[branch][values][convention]") {
    std::cout << "\n=== Test: Branch with various condition values ===\n";
    
    struct TestCase {
        int conditionValue;
        bool expectedTrue;
        std::string description;
    };
    
    std::vector<TestCase> testCases = {
        {0, false, "0 should be FALSE"},
        {1, true, "1 should be TRUE"},
        {42, true, "42 should be TRUE"},
        {-5, true, "-5 should be TRUE"},
        {100, true, "100 should be TRUE"}
    };
    
    for (const auto& test : testCases) {
        std::cout << "\n--- Testing: " << test.description << " ---\n";
        
        // Variables
        std::vector<std::shared_ptr<Variable>> variables;
        
        auto varCondition = std::make_shared<Variable>("condition");
        varCondition->SetIntegerValue(test.conditionValue);
        variables.push_back(varCondition);
        
        // Nodes
        auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
        functionEntry->SetWeight(100);
        
        auto varNode = std::make_shared<VariableNode>("var-condition");
        varNode->SetVariable(varCondition);
        
        auto branchNode = std::make_shared<BranchNode>("branch-node");
        branchNode->Initialize();
        
        auto printTrue = std::make_shared<PrintNode>("print-true");
        printTrue->SetText("TRUE");
        printTrue->Initialize();
        
        auto printFalse = std::make_shared<PrintNode>("print-false");
        printFalse->SetText("FALSE");
        printFalse->Initialize();
        
        std::vector<std::shared_ptr<BaseNode>> nodes = {
            functionEntry, varNode, branchNode, printTrue, printFalse
        };
        
        // Connections (même pattern que TEST 1)
        std::vector<std::shared_ptr<Connection>> connections;
        
        auto execConn = std::make_shared<Connection>();
        execConn->outNodeId = functionEntry->GetId();
        execConn->outPortIndex = 0;
        execConn->inNodeId = branchNode->GetId();
        execConn->inPortIndex = 0;
        execConn->type = Connection::EXECUTION_LINK;
        connections.push_back(execConn);
        
        auto dataConn = std::make_shared<Connection>();
        dataConn->outNodeId = varNode->GetId();
        dataConn->outPortIndex = 0;
        dataConn->inNodeId = branchNode->GetId();
        dataConn->inPortIndex = 1;
        dataConn->type = Connection::DATA_LINK;
        connections.push_back(dataConn);
        
        auto execTrue = std::make_shared<Connection>();
        execTrue->outNodeId = branchNode->GetId();
        execTrue->outPortIndex = 0;
        execTrue->inNodeId = printTrue->GetId();
        execTrue->inPortIndex = 0;
        execTrue->type = Connection::EXECUTION_LINK;
        connections.push_back(execTrue);
        
        auto execFalse = std::make_shared<Connection>();
        execFalse->outNodeId = branchNode->GetId();
        execFalse->outPortIndex = 1;
        execFalse->inNodeId = printFalse->GetId();
        execFalse->inPortIndex = 0;
        execFalse->type = Connection::EXECUTION_LINK;
        connections.push_back(execFalse);
        
        // Build and generate
        ASTBuilder builder(nodes, connections);
        auto pathTree = builder.BuildAST();
        
        AssemblyGenerator::GeneratorContext context(
            variables, "2025-01-10", "test-branch-values", false, true, 1024
        );
        
        AssemblyGeneratorChip32TAC tacGen(context);
        tacGen.GenerateCompleteProgram(nodes, pathTree);
        
        std::string assembly = tacGen.GetAssembly().str();
        
        // Execute
        Chip32::Machine machine;
        machine.QuickExecute(assembly);
        
        std::string output = machine.printOutput;
        std::string expected = test.expectedTrue ? "TRUE" : "FALSE";
        
        std::cout << "Condition value: " << test.conditionValue << "\n";
        std::cout << "Expected: " << expected << ", Got: " << output << "\n";
        
        REQUIRE(output.find(expected) != std::string::npos);
    }
    
    std::cout << "\n✓ All value tests passed\n";
}

// ===================================================================
// TEST 4 : Tester tous les opérateurs de comparaison
// ===================================================================
TEST_CASE("Branch with all comparison operators", "[branch][operators][all]") {
    std::cout << "\n=== Test: All comparison operators ===\n";
    
    struct TestCase {
        OperatorNode::OperationType op;
        int a, b;
        bool expectedTrue;
        std::string description;
    };
    
    std::vector<TestCase> testCases = {
        {OperatorNode::OperationType::EQUAL, 5, 5, true, "5 == 5"},
        {OperatorNode::OperationType::EQUAL, 5, 10, false, "5 == 10"},
        {OperatorNode::OperationType::NOT_EQUAL, 5, 10, true, "5 != 10"},
        {OperatorNode::OperationType::NOT_EQUAL, 5, 5, false, "5 != 5"},
        {OperatorNode::OperationType::GREATER_THAN, 10, 5, true, "10 > 5"},
        {OperatorNode::OperationType::GREATER_THAN, 5, 10, false, "5 > 10"},
        {OperatorNode::OperationType::LESS_THAN, 5, 10, true, "5 < 10"},
        {OperatorNode::OperationType::LESS_THAN, 10, 5, false, "10 < 5"},
        {OperatorNode::OperationType::GREATER_EQUAL, 10, 5, true, "10 >= 5"},
        {OperatorNode::OperationType::GREATER_EQUAL, 5, 5, true, "5 >= 5"},
        {OperatorNode::OperationType::GREATER_EQUAL, 5, 10, false, "5 >= 10"},
        {OperatorNode::OperationType::LESS_EQUAL, 5, 10, true, "5 <= 10"},
        {OperatorNode::OperationType::LESS_EQUAL, 5, 5, true, "5 <= 5"},
        {OperatorNode::OperationType::LESS_EQUAL, 10, 5, false, "10 <= 5"}
    };
    
    for (const auto& test : testCases) {
        std::cout << "\n--- Testing: " << test.description << " ---\n";
        
        // Variables
        std::vector<std::shared_ptr<Variable>> variables;
        
        auto varA = std::make_shared<Variable>("A");
        varA->SetIntegerValue(test.a);
        variables.push_back(varA);
        
        auto varB = std::make_shared<Variable>("B");
        varB->SetIntegerValue(test.b);
        variables.push_back(varB);
        
        // Nodes (même structure que TEST 2)
        auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
        functionEntry->SetWeight(100);
        
        auto varNodeA = std::make_shared<VariableNode>("var-A");
        varNodeA->SetVariable(varA);
        
        auto varNodeB = std::make_shared<VariableNode>("var-B");
        varNodeB->SetVariable(varB);
        
        auto operatorNode = std::make_shared<OperatorNode>("operator");
        operatorNode->SetOperationType(test.op);
        operatorNode->Initialize();
        
        auto branchNode = std::make_shared<BranchNode>("branch-node");
        branchNode->Initialize();
        
        auto printTrue = std::make_shared<PrintNode>("print-true");
        printTrue->SetText("TRUE");
        printTrue->Initialize();
        
        auto printFalse = std::make_shared<PrintNode>("print-false");
        printFalse->SetText("FALSE");
        printFalse->Initialize();
        
        std::vector<std::shared_ptr<BaseNode>> nodes = {
            functionEntry, varNodeA, varNodeB, operatorNode, branchNode, printTrue, printFalse
        };
        
        // Connections (pattern complet)
        std::vector<std::shared_ptr<Connection>> connections;
        
        auto execConn = std::make_shared<Connection>();
        execConn->outNodeId = functionEntry->GetId();
        execConn->outPortIndex = 0;
        execConn->inNodeId = branchNode->GetId();
        execConn->inPortIndex = 0;
        execConn->type = Connection::EXECUTION_LINK;
        connections.push_back(execConn);
        
        auto dataConn1 = std::make_shared<Connection>();
        dataConn1->outNodeId = varNodeA->GetId();
        dataConn1->outPortIndex = 0;
        dataConn1->inNodeId = operatorNode->GetId();
        dataConn1->inPortIndex = 0;
        dataConn1->type = Connection::DATA_LINK;
        connections.push_back(dataConn1);
        
        auto dataConn2 = std::make_shared<Connection>();
        dataConn2->outNodeId = varNodeB->GetId();
        dataConn2->outPortIndex = 0;
        dataConn2->inNodeId = operatorNode->GetId();
        dataConn2->inPortIndex = 1;
        dataConn2->type = Connection::DATA_LINK;
        connections.push_back(dataConn2);
        
        auto dataConn3 = std::make_shared<Connection>();
        dataConn3->outNodeId = operatorNode->GetId();
        dataConn3->outPortIndex = 0;
        dataConn3->inNodeId = branchNode->GetId();
        dataConn3->inPortIndex = 1;
        dataConn3->type = Connection::DATA_LINK;
        connections.push_back(dataConn3);
        
        auto execTrue = std::make_shared<Connection>();
        execTrue->outNodeId = branchNode->GetId();
        execTrue->outPortIndex = 0;
        execTrue->inNodeId = printTrue->GetId();
        execTrue->inPortIndex = 0;
        execTrue->type = Connection::EXECUTION_LINK;
        connections.push_back(execTrue);
        
        auto execFalse = std::make_shared<Connection>();
        execFalse->outNodeId = branchNode->GetId();
        execFalse->outPortIndex = 1;
        execFalse->inNodeId = printFalse->GetId();
        execFalse->inPortIndex = 0;
        execFalse->type = Connection::EXECUTION_LINK;
        connections.push_back(execFalse);
        
        // Build and execute
        ASTBuilder builder(nodes, connections);
        auto pathTree = builder.BuildAST();
        
        AssemblyGenerator::GeneratorContext context(
            variables, "2025-01-10", "test-branch-op", false, true, 1024
        );
        
        AssemblyGeneratorChip32TAC tacGen(context);
        tacGen.GenerateCompleteProgram(nodes, pathTree);
        
        std::string assembly = tacGen.GetAssembly().str();
        
        Chip32::Machine machine;
        machine.QuickExecute(assembly);
        
        std::string output = machine.printOutput;
        std::string expected = test.expectedTrue ? "TRUE" : "FALSE";
        
        std::cout << "Expected: " << expected << ", Got: " << output << "\n";
        REQUIRE(output.find(expected) != std::string::npos);
    }
    
    std::cout << "\n✓ All operator tests passed\n";
}

// ===================================================================
// TEST 5 : BranchNode avec résultat d'opération arithmétique
// ===================================================================
TEST_CASE("Branch with arithmetic operation result", "[branch][arithmetic]") {
    std::cout << "\n=== Test: Branch with arithmetic result (5 - 5 = 0) ===\n";
    
    // Variables
    std::vector<std::shared_ptr<Variable>> variables;
    
    auto varA = std::make_shared<Variable>("A");
    varA->SetIntegerValue(5);
    variables.push_back(varA);
    
    auto varB = std::make_shared<Variable>("B");
    varB->SetIntegerValue(5);
    variables.push_back(varB);
    
    // Nodes
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry");
    functionEntry->SetWeight(100);
    
    auto varNodeA = std::make_shared<VariableNode>("var-A");
    varNodeA->SetVariable(varA);
    
    auto varNodeB = std::make_shared<VariableNode>("var-B");
    varNodeB->SetVariable(varB);
    
    // Soustraction: 5 - 5 = 0 (false)
    auto subNode = std::make_shared<OperatorNode>("operator-sub");
    subNode->SetOperationType(OperatorNode::OperationType::SUBTRACT);
    subNode->Initialize();
    
    auto branchNode = std::make_shared<BranchNode>("branch-node");
    branchNode->Initialize();
    
    auto printTrue = std::make_shared<PrintNode>("print-true");
    printTrue->SetText("Result is NON-ZERO");
    printTrue->Initialize();
    
    auto printFalse = std::make_shared<PrintNode>("print-false");
    printFalse->SetText("Result is ZERO");
    printFalse->Initialize();
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry, varNodeA, varNodeB, subNode, branchNode, printTrue, printFalse
    };
    
    // Connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    auto execConn = std::make_shared<Connection>();
    execConn->outNodeId = functionEntry->GetId();
    execConn->outPortIndex = 0;
    execConn->inNodeId = branchNode->GetId();
    execConn->inPortIndex = 0;
    execConn->type = Connection::EXECUTION_LINK;
    connections.push_back(execConn);
    
    auto dataConn1 = std::make_shared<Connection>();
    dataConn1->outNodeId = varNodeA->GetId();
    dataConn1->outPortIndex = 0;
    dataConn1->inNodeId = subNode->GetId();
    dataConn1->inPortIndex = 0;
    dataConn1->type = Connection::DATA_LINK;
    connections.push_back(dataConn1);
    
    auto dataConn2 = std::make_shared<Connection>();
    dataConn2->outNodeId = varNodeB->GetId();
    dataConn2->outPortIndex = 0;
    dataConn2->inNodeId = subNode->GetId();
    dataConn2->inPortIndex = 1;
    dataConn2->type = Connection::DATA_LINK;
    connections.push_back(dataConn2);
    
    auto dataConn3 = std::make_shared<Connection>();
    dataConn3->outNodeId = subNode->GetId();
    dataConn3->outPortIndex = 0;
    dataConn3->inNodeId = branchNode->GetId();
    dataConn3->inPortIndex = 1;
    dataConn3->type = Connection::DATA_LINK;
    connections.push_back(dataConn3);
    
    auto execTrue = std::make_shared<Connection>();
    execTrue->outNodeId = branchNode->GetId();
    execTrue->outPortIndex = 0;
    execTrue->inNodeId = printTrue->GetId();
    execTrue->inPortIndex = 0;
    execTrue->type = Connection::EXECUTION_LINK;
    connections.push_back(execTrue);
    
    auto execFalse = std::make_shared<Connection>();
    execFalse->outNodeId = branchNode->GetId();
    execFalse->outPortIndex = 1;
    execFalse->inNodeId = printFalse->GetId();
    execFalse->inPortIndex = 0;
    execFalse->type = Connection::EXECUTION_LINK;
    connections.push_back(execFalse);
    
    // Build and execute
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    AssemblyGenerator::GeneratorContext context(
        variables, "2025-01-10", "test-branch-arithmetic", true, true, 1024
    );
    
    AssemblyGeneratorChip32TAC tacGen(context);
    tacGen.GenerateCompleteProgram(nodes, pathTree);
    
    std::string assembly = tacGen.GetAssembly().str();
    
    std::cout << "\n--- Generated Assembly ---\n";
    std::cout << assembly << std::endl;
    
    // Verify subtraction is present
    REQUIRE(assembly.find("sub") != std::string::npos);
    
    // Execute
    std::cout << "\n--- Execution Output ---\n";
    std::cout << "Expected: Result is ZERO (5 - 5 = 0)\n";
    std::cout << "Actual:   ";
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
    
    std::string output = machine.printOutput;
    REQUIRE(output.find("ZERO") != std::string::npos);
    
    std::cout << "\n✓ Test passed\n";
}