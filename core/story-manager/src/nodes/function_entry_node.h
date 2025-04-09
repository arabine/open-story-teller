#pragma once

#include "execution_node.h"


class FunctionEntryNode : public ExecutionNode
{
public:
    FunctionEntryNode(const std::string &type)
        : ExecutionNode(type, "Function Entry Node") {}

    void Initialize() override {
        // Initialisation spécifique pour FunctionEntryNode
        // Par exemple, préparer les entrées nécessaires pour la fonction
    }

    std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override {
        // Logique de construction pour FunctionEntryNode
        return GetMyEntryLabel() + ":\n";
    }

    std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override {
        // Génération des constantes pour FunctionEntryNode
        return "FunctionEntryNode Constants";
    }

    // Ajoutez des méthodes spécifiques pour gérer l'entrée de la fonction
    void PrepareFunctionEntry() {
        // Logique pour préparer l'entrée de la fonction
    }
};
