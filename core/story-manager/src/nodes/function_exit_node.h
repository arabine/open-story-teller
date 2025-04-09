#pragma once

#include "execution_node.h"

class FunctionExitNode : public ExecutionNode
{
public:
    FunctionExitNode(const std::string &type, const std::string &typeName)
        : ExecutionNode(type, typeName) {}

    void Initialize() override {
        // Initialisation spécifique pour FunctionExitNode
        // Par exemple, préparer les sorties nécessaires pour la fonction
    }

    std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override {
        // Logique de construction pour FunctionExitNode
        return "FunctionExitNode Build";
    }

    std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override {
        // Génération des constantes pour FunctionExitNode
        return "FunctionExitNode Constants";
    }

    // Ajoutez des méthodes spécifiques pour gérer la sortie de la fonction
    void FinalizeFunctionExit() {
        // Logique pour finaliser la sortie de la fonction
    }
};
