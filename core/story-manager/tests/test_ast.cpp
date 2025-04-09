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
#include "chip32_machine.h"

#include <stdarg.h>
#include <string.h>

#include "ast_builder.h"
#include "assembly_generator.h"
#include "flow_generator.h"

/*
void ProcessASTTree(const ASTBuilder::PathTree& tree, int depth = 0) {
  std::queue<std::pair<std::shared_ptr<ASTNode>, int>> queue;
  queue.push({tree.root, depth});
  std::unordered_set<std::string> visited;

  while (!queue.empty()) {
      auto [node, currentDepth] = queue.front();
      queue.pop();

      if (visited.find(node->node->GetId()) != visited.end()) {
          continue;
      }
      visited.insert(node->node->GetId());

      std::string indent(currentDepth * 2, ' ');
      std::cout << indent << "Node: " << node->node->GetTypeName() 
               << " (ID: " << node->node->GetId() << ")" << std::endl;

      // Print data inputs
      for (const auto& [portIndex, inputNode] : node->dataInputs) {
          std::cout << indent << "  Input at port " << portIndex 
                   << " from: " << inputNode->node->GetTypeName() << std::endl;
      }

      // Print data outputs
      for (const auto& [portIndex, outputs] : node->dataOutputs) {
          for (const auto& [targetNode, targetPort] : outputs) {
              std::cout << indent << "  Output from port " << portIndex 
                       << " to: " << targetNode->node->GetTypeName() 
                       << " port " << targetPort << std::endl;
          }
      }

      // Add children to queue
      for (const auto& child : node->children) {
          queue.push({child, currentDepth + 1});
      }
  }
}
*/
TEST_CASE( "Check various indentations and typos" ) {

    Compiler compiler;

    auto printNodeTrue = std::make_shared<PrintNode>("print-node");
    printNodeTrue->SetText("Good!");

    auto printNodeFalse = std::make_shared<PrintNode>("print-node");
    printNodeFalse->SetText("Bad :(");

    auto branchNode = std::make_shared<BranchNode>("branch-node");

    auto functionEntryNode = std::make_shared<FunctionEntryNode>("function-entry-node");

    auto variableNode = std::make_shared<VariableNode>("variable-node");


    std::vector<std::shared_ptr<BaseNode>> nodes;

    nodes.push_back(functionEntryNode);
    nodes.push_back(printNodeTrue);
    nodes.push_back(printNodeFalse);
    nodes.push_back(branchNode);
    nodes.push_back(variableNode);

    auto cn1 = std::make_shared<Connection>();
    auto cn2 = std::make_shared<Connection>();
    auto cn3 = std::make_shared<Connection>();
    auto cn4 = std::make_shared<Connection>();

    std::vector<std::shared_ptr<Connection>> connections;

    connections.push_back(cn1);
    connections.push_back(cn2);
    connections.push_back(cn3);
    connections.push_back(cn4);
  

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
    
    // Variable branch -> Branch node (condition input)
    cn4->inNodeId = branchNode->GetId();
    cn4->inPortIndex = 1;
    cn4->outNodeId = variableNode->GetId();
    cn4->outPortIndex = 0;
    cn4->type = Connection::DATA_LINK;


    // // Création des nœuds
    // std::vector<Node> nodes = {
    //     Node(Node::Type::VARIABLE, "i", "node_i"),
    //     Node(Node::Type::CONSTANT, 10, "node_10"),
    //     Node(Node::Type::SUBTRACT, "node_subtract"),
    //     Node(Node::Type::CONSTANT, 1, "node_1"),
    //     Node(Node::Type::ADD, "node_add"),
    //     Node(Node::Type::ASSIGN, "i", "node_assign"),
    //     Node(Node::Type::VARIABLE, "conditionVar", "node_condVar"),
    //     Node(Node::Type::CONSTANT, 2, "node_2"),
    //     Node(Node::Type::MULTIPLY, "node_multiply"),
    //     Node(Node::Type::ASSIGN, "i", "node_assign_multiply"),
    //     Node(Node::Type::BRANCH, "node_branch"),
    //     Node(Node::Type::LOOP, "node_loop")
    // };

    // try 
    // {
    //   // Construction de l'AST
    //   compiler.buildAST(nodes, connections);
    //   compiler.printAST();

    // } catch(const std::exception &e)
    // {
    //   std::cout << e.what() << std::endl;
    // }


     // Create generator context with current time and user
     AssemblyGenerator::GeneratorContext context(
        "2025-04-08 12:09:01",  // Current UTC time
        "arabine",              // Current user
        true,                   // Enable debug output
        true,                   // Enable optimizations
        1024                    // Stack size
    );

    // Create generator
    AssemblyGenerator generator(context);


    ASTBuilder builder(nodes, connections);
    auto pathTrees = builder.BuildAST();

    /*
    // Process each path tree
    for (const auto& tree : pathTrees) {
        std::cout << (tree.isExecutionPath ? "Execution" : "Data") 
                 << " Path Tree:" << std::endl;
        ProcessASTTree(tree);
        std::cout << std::endl;
    }
        */

    // Generate flow in the console
    VisualFlowGenerator flowGenerator(context);
    std::string flow = flowGenerator.GenerateAssembly(pathTrees);

    FlowVisualizer::PrintHeader("arabine", "2025-04-08 12:03:01");
    std::cout << "\nGenerated flow:\n" << flow << std::endl;

    // Generate assembly
    std::string assembly = generator.GenerateAssembly(pathTrees);


    

    
    

    
    // compiler.generateAssembly();

    // std::cout << compiler.GetCode() << std::endl;


    // Chip32::Machine machine;

    // machine.QuickExecute(compiler.GetCode());

  //  REQUIRE( parseResult == true );

}
