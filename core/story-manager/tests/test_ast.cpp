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
#include "chip32_machine.h"

#include <stdarg.h>
#include <string.h>


TEST_CASE( "Check various indentations and typos" ) {

    Compiler compiler;

    auto printNode = std::make_shared<PrintNode>("print-node");

    printNode->SetText("Hello from OST");

  //  auto branchNode = std::make_shared<BranchNode>("branch-node");


    std::vector<std::shared_ptr<BaseNode>> nodes;

    nodes.push_back(printNode);
  //  nodes.push_back(branchNode);

    std::vector<std::shared_ptr<Connection>> connections;

    auto cn1 = std::make_shared<Connection>();
  

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



    // Construction de l'AST
    compiler.buildAST(nodes, connections);
    compiler.displayNodeSchema();

    compiler.generateAssembly();

    std::cout << compiler.GetCode() << std::endl;


    Chip32::Machine machine;

    machine.QuickExecute(compiler.GetCode());

  //  REQUIRE( parseResult == true );

}
