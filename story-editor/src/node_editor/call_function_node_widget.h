// call_function_node_widget.h
#pragma once

#include <vector>
#include <map>
#include <string>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "call_function_node.h"
#include "gui.h"

class CallFunctionNodeWidget : public BaseNodeWidget
{
public:
    CallFunctionNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_callFunctionNode = std::dynamic_pointer_cast<CallFunctionNode>(node);
        SetTitle("Call Function");
    }

    void Initialize() override {
        BaseNodeWidget::Initialize();
        m_functionName = m_callFunctionNode->GetFunctionName();
        m_functionUuid = m_callFunctionNode->GetFunctionUuid();
    }

    void DrawProperties(std::shared_ptr<IStoryProject> story) override {
        ImGui::AlignTextToFramePadding();
        
        // Liste déroulante des fonctions disponibles
        if (ImGui::BeginCombo("Function", m_functionName.empty() ? "<Select function>" : m_functionName.c_str())) {

            // Récupérer la liste des fonctions du projet
            auto functions = story->GetFunctionsList();  // À implémenter dans IStoryProject
            
            for (size_t i = 0; i < functions.size(); ++i) {
                const bool is_selected = (m_functionUuid == functions[i].uuid);
                
                if (ImGui::Selectable(functions[i].name.c_str(), is_selected)) {
                    m_functionUuid = functions[i].uuid;
                    m_functionName = functions[i].name;
                    m_callFunctionNode->SetFunction(m_functionUuid, m_functionName);
                }
                
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Bouton pour ouvrir la fonction dans l'éditeur
        ImGui::Spacing();
        
        // Désactiver le bouton si aucune fonction n'est sélectionnée
        if (m_functionUuid.empty()) {
            ImGui::BeginDisabled();
        }
        
        if (ImGui::Button("> Open function")) {
            m_manager.OpenFunction(m_functionUuid, m_functionName);
        }
        
        if (m_functionUuid.empty()) {
            ImGui::EndDisabled();
        }
    }

    void Draw() override {
        // Afficher le nom de la fonction dans le noeud
        ImGui::TextUnformatted(m_functionName.empty() 
            ? "<No function>" 
            : m_functionName.c_str());
    }

private:
    IStoryManager &m_manager;
    std::shared_ptr<CallFunctionNode> m_callFunctionNode;
    std::string m_functionName;
    std::string m_functionUuid;
};