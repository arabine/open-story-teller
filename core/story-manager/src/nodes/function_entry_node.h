#pragma once

#include "base_node.h"


class FunctionEntryNode : public BaseNode
{
public:
    FunctionEntryNode(const std::string &type)
        : BaseNode(type, "Function Entry Node")
    {
        SetWeight(100);
        SetBehavior(BaseNode::BEHAVIOR_EXECUTION);
        SetupExecutionPorts(false, 1, true);
    }

    void Initialize() override {
        // Initialisation spécifique pour FunctionEntryNode
        // Par exemple, préparer les entrées nécessaires pour la fonction
    }

    // Ajoutez des méthodes spécifiques pour gérer l'entrée de la fonction
    void PrepareFunctionEntry() {
        // Logique pour préparer l'entrée de la fonction
    }
};
