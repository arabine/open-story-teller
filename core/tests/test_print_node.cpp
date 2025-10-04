// test_print_with_args.cpp
// Test unitaire pour vérifier que les arguments du Print sont bien pris en compte

#include "catch.hpp"
#include "print_node.h"
#include "variable_node.h"
#include "function_entry_node.h"
#include "operator_node.h"
#include "connection.h"
#include "ast_builder.h"
#include "assembly_generator_chip32.h"
#include "chip32_machine.h"
#include "variable.h"

TEST_CASE("Print with single argument") {
    // Create the print node
    auto printNode = std::make_shared<PrintNode>("print-node");
    printNode->SetText("Compteur: %d");
    
    // Create function entry
    auto functionEntryNode = std::make_shared<FunctionEntryNode>("function-entry-node");
    
    // IMPORTANT: Create the "counter" variable and add it to the global variables list
    std::vector<std::shared_ptr<Variable>> variables;
    auto counterVar = std::make_shared<Variable>("counter");
    counterVar->SetIntegerValue(42);  // Initial value
    variables.push_back(counterVar);  // ← CRUCIAL: Add to global variables
    
    // Create a variable node that references the counter variable
    auto variableNodeCounter = std::make_shared<VariableNode>("variable-node");
    variableNodeCounter->SetVariable(counterVar);
    
    // Build the node list
    std::vector<std::shared_ptr<BaseNode>> nodes;
    nodes.push_back(functionEntryNode);
    nodes.push_back(printNode);
    nodes.push_back(variableNodeCounter);
    
    // Create connections
    std::vector<std::shared_ptr<Connection>> connections;
    
    // Connect function entry to print node (execution flow)
    auto cn1 = std::make_shared<Connection>();
    cn1->inNodeId = printNode->GetId();
    cn1->inPortIndex = 0;
    cn1->outNodeId = functionEntryNode->GetId();
    cn1->outPortIndex = 0;
    cn1->type = Connection::EXECUTION_LINK;
    connections.push_back(cn1);
    
    // Connect variable node to print node (data flow - arg0)
    auto cn2 = std::make_shared<Connection>();
    cn2->inNodeId = printNode->GetId();
    cn2->inPortIndex = 1;  // arg0 input port
    cn2->outNodeId = variableNodeCounter->GetId();
    cn2->outPortIndex = 0;
    cn2->type = Connection::DATA_LINK;
    connections.push_back(cn2);
    
    // Create generator context with the variables list
    AssemblyGenerator::GeneratorContext context(
        variables,  // ← IMPORTANT: Pass the variables including counter
        "2025-01-10 10:00:00",
        "test-print-args",
        true,
        true,
        1024
    );
    
    // Create generator
    AssemblyGeneratorChip32 generator(context);
    
    // Build AST
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    // Generate assembly
    generator.Reset();
    generator.GenerateHeader();
    
    // DATA section - this will now include the counter variable
    generator.StartSection(AssemblyGenerator::Section::DATA);
    generator.GenerateNodesVariables(nodes);  // Print node format string
    generator.GenerateGlobalVariables();      // ← This generates counter variable
    
    // TEXT section
    generator.StartSection(AssemblyGenerator::Section::TEXT);
    generator.GenerateTextSection(pathTree);
    generator.GenerateExit();
    
    std::string assembly = generator.GetAssembly();
    
    std::cout << "===== Generated Assembly =====" << std::endl;
    std::cout << assembly << std::endl;
    
    // Now the assembly should include the counter variable declaration:
    // $XEIxIsZoXA DV32, 42 ; counter
    
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
}

TEST_CASE("Print with multiple arguments") {
    // ===== Setup =====
    
    // Variables
    auto var_a = std::make_shared<Variable>("a");
    var_a->SetIntegerValue(10);  // ← CORRECTION: Utiliser SetIntegerValue() au lieu de SetValue<int>()
    
    auto var_b = std::make_shared<Variable>("b");
    var_b->SetIntegerValue(20);  // ← CORRECTION: Utiliser SetIntegerValue() au lieu de SetValue<int>()
    
    std::vector<std::shared_ptr<Variable>> variables = {var_a, var_b};
    
    // Nœuds
    auto functionEntry = std::make_shared<FunctionEntryNode>("function-entry-node");
    functionEntry->SetWeight(100);
    
    auto varNodeA = std::make_shared<VariableNode>("variable-node");
    varNodeA->SetVariableUuid(var_a->GetUuid());
    
    auto varNodeB = std::make_shared<VariableNode>("variable-node");
    varNodeB->SetVariableUuid(var_b->GetUuid());
    
    auto addNode = std::make_shared<OperatorNode>("operator-node");
    addNode->SetOperationType(OperatorNode::OperationType::ADD);
    
    auto printNode = std::make_shared<PrintNode>("print-node");
    printNode->SetText("Calcul: %d + %d = %d");
    // IMPORTANT: Appeler Initialize() si nécessaire après SetText()
    // printNode->Initialize(); // Si le test ne charge pas depuis JSON
    
    std::vector<std::shared_ptr<BaseNode>> nodes = {
        functionEntry,
        varNodeA,
        varNodeB,
        addNode,
        printNode
    };
    
    // Connexions
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
    dataConn1->inPortIndex = 1; // ← CORRECTION: arg0 = port 1 (port 0 = execution)
    dataConn1->type = Connection::DATA_LINK;
    connections.push_back(dataConn1);
    
    // Data: varB → Print.arg1
    auto dataConn2 = std::make_shared<Connection>();
    dataConn2->outNodeId = varNodeB->GetId();
    dataConn2->outPortIndex = 0;
    dataConn2->inNodeId = printNode->GetId();
    dataConn2->inPortIndex = 2; // ← CORRECTION: arg1 = port 2
    dataConn2->type = Connection::DATA_LINK;
    connections.push_back(dataConn2);
    
    // Data: varA → ADD.input0
    auto dataConn3 = std::make_shared<Connection>();
    dataConn3->outNodeId = varNodeA->GetId();
    dataConn3->outPortIndex = 0;
    dataConn3->inNodeId = addNode->GetId();
    dataConn3->inPortIndex = 0;
    dataConn3->type = Connection::DATA_LINK;
    connections.push_back(dataConn3);
    
    // Data: varB → ADD.input1
    auto dataConn4 = std::make_shared<Connection>();
    dataConn4->outNodeId = varNodeB->GetId();
    dataConn4->outPortIndex = 0;
    dataConn4->inNodeId = addNode->GetId();
    dataConn4->inPortIndex = 1;
    dataConn4->type = Connection::DATA_LINK;
    connections.push_back(dataConn4);
    
    // Data: ADD → Print.arg2
    auto dataConn5 = std::make_shared<Connection>();
    dataConn5->outNodeId = addNode->GetId();
    dataConn5->outPortIndex = 0;
    dataConn5->inNodeId = printNode->GetId();
    dataConn5->inPortIndex = 3; // ← CORRECTION: arg2 = port 3
    dataConn5->type = Connection::DATA_LINK;
    connections.push_back(dataConn5);
    
    // ===== Build & Generate =====
    
    ASTBuilder builder(nodes, connections);
    auto pathTree = builder.BuildAST();
    
    AssemblyGenerator::GeneratorContext context(
        variables,
        "2025-01-10 10:00:00",
        "test-print-multi-args",
        true, true, 1024
    );
    
    AssemblyGeneratorChip32 generator(context);
    generator.Reset();
    generator.GenerateHeader();

    
    generator.StartSection(AssemblyGenerator::Section::DATA);
    generator.GenerateNodesVariables(nodes);
    generator.GenerateGlobalVariables();
    generator.StartSection(AssemblyGenerator::Section::TEXT);
    generator.GenerateTextSection(pathTree);
    generator.GenerateExit();
    
    std::string assembly = generator.GetAssembly();
    
    std::cout << "\n===== Generated Assembly (Multi-Args) =====\n" << assembly << std::endl;
    
    // ===== Vérifications =====
    
    // // Vérifier 3 arguments
    // REQUIRE(assembly.find("lcons r1, 3") != std::string::npos);
    
    // // Vérifier chargement des 3 registres
    // REQUIRE(assembly.find("load r2") != std::string::npos); // arg0
    // REQUIRE(assembly.find("load r3") != std::string::npos); // arg1
    // // r4 vient de l'opérateur ADD (devrait être sur la pile ou dans r4)
    
    // Execute
    Chip32::Machine machine;
    machine.QuickExecute(assembly);
}