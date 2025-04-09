#pragma once

#include "base_node.h"

class ExecutionNode : public BaseNode
{
public:
    ExecutionNode(const std::string &type, const std::string &typeName)
        : BaseNode(type, typeName) {}

    void Initialize() override {
        // Initialisation spécifique pour ExecutionNode
    }

    std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override {
        return GetMyEntryLabel() + ":\n";
    }

    std::string GenerateAssembly() const override {
        return GetMyEntryLabel() + ":\n";
    }

    std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override {
        // Génération des constantes pour ExecutionNode
        return "ExecutionNode Constants";
    }

    // Ajoutez des méthodes spécifiques pour gérer les entrées et sorties d'exécution
    void AddExecutionInput() {
        // Logique pour ajouter une entrée d'exécution
    }

    void AddExecutionOutput() {
        // Logique pour ajouter une sortie d'exécution
    }
};
