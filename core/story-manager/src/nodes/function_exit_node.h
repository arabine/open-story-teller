#pragma once

#include "base_node.h"

class FunctionExitNode : public BaseNode
{
public:
    FunctionExitNode(const std::string &type)
        : BaseNode(type, "Function Exit Node")
    {

        SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
    }

    void Initialize() override {
        // Initialisation spécifique pour FunctionExitNode
        // Par exemple, préparer les sorties nécessaires pour la fonction
    }



    // Ajoutez des méthodes spécifiques pour gérer la sortie de la fonction
    void FinalizeFunctionExit() {
        // Logique pour finaliser la sortie de la fonction
    }
};
