#pragma once

#include "base_node.h"


class FunctionEntryNode : public BaseNode
{
public:
    FunctionEntryNode(const std::string &type)
        : BaseNode(type, "Function Entry Node")
    {
        SetWeight(100);
    }

    void Initialize() override {
        // Initialisation spécifique pour FunctionEntryNode
        // Par exemple, préparer les entrées nécessaires pour la fonction
    }

    std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override {
        // Logique de construction pour FunctionEntryNode
        return GetMyEntryLabel() + ":\n";
    }

    // Ajoutez des méthodes spécifiques pour gérer l'entrée de la fonction
    void PrepareFunctionEntry() {
        // Logique pour préparer l'entrée de la fonction
    }
};
