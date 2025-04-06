#pragma once

#include "base_node.h"

class OperatorNode : public BaseNode
{
public:
    OperatorNode(const std::string &type, const std::string &typeName)
        : BaseNode(type, typeName) {}

    void Initialize() override {
        // Initialisation spécifique pour DataNode
    }

    std::string Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns) override {
        // Logique de construction pour DataNode
        return "DataNode Build";
    }

    std::string GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns) override {
        // Génération des constantes pour DataNode
        return "DataNode Constants";
    }

    // Ajoutez des méthodes spécifiques pour gérer les données directement
    void ProcessData() {
        // Logique pour traiter les données directement
    }
};


