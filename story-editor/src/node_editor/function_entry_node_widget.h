#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "function_entry_node.h"
#include "gui.h"
#include "IconsMaterialDesignIcons.h"

class FunctionEntryNodeWidget : public BaseNodeWidget
{
public:
    FunctionEntryNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_functionEntryNode = std::dynamic_pointer_cast<FunctionEntryNode>(node);
        SetTitle("Function Entry");
    }

    void Draw() override {
        ImGui::TextUnformatted("Entry Point");
        if (m_functionEntryNode->GetParameterCount() > 0) {
            ImGui::Text(ICON_MDI_ARROW_RIGHT " %zu parameter(s)", m_functionEntryNode->GetParameterCount());
        }
    }

    virtual bool HasSync() const override { 
        return false;
    }

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), ICON_MDI_IMPORT " Function Entry");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("This node defines the entry point of the function/module.");
        ImGui::Text("Add parameters that will be passed when this function is called.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Parameters list
        ImGui::Text(ICON_MDI_FORMAT_LIST_BULLETED " Parameters:");
        ImGui::Spacing();

        const auto& parameters = m_functionEntryNode->GetParameters();
        
        // Table for parameters
        if (ImGui::BeginTable("parameters_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Default", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            // Display existing parameters
            for (size_t i = 0; i < parameters.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::PushID(static_cast<int>(i));

                // Name
                ImGui::TableNextColumn();
                char nameBuf[128];
                strncpy(nameBuf, parameters[i].name.c_str(), sizeof(nameBuf) - 1);
                nameBuf[sizeof(nameBuf) - 1] = '\0';
                
                if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf))) {
                    m_functionEntryNode->UpdateParameter(i, nameBuf, parameters[i].type, parameters[i].defaultValue);
                }

                // Type
                ImGui::TableNextColumn();
                const char* types[] = {"int", "float", "string", "bool"};
                int currentType = 0;
                for (int t = 0; t < 4; ++t) {
                    if (parameters[i].type == types[t]) {
                        currentType = t;
                        break;
                    }
                }
                
                if (ImGui::Combo("##type", &currentType, types, 4)) {
                    m_functionEntryNode->UpdateParameter(i, parameters[i].name, types[currentType], parameters[i].defaultValue);
                }

                // Default value
                ImGui::TableNextColumn();
                char defaultBuf[128];
                strncpy(defaultBuf, parameters[i].defaultValue.c_str(), sizeof(defaultBuf) - 1);
                defaultBuf[sizeof(defaultBuf) - 1] = '\0';
                
                if (ImGui::InputText("##default", defaultBuf, sizeof(defaultBuf))) {
                    m_functionEntryNode->UpdateParameter(i, parameters[i].name, parameters[i].type, defaultBuf);
                }

                // Actions
                ImGui::TableNextColumn();
                if (ImGui::Button(ICON_MDI_DELETE "##delete")) {
                    m_functionEntryNode->RemoveParameter(i);
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();

        // Add new parameter
        if (ImGui::Button(ICON_MDI_PLUS " Add Parameter")) {
            m_functionEntryNode->AddParameter("newParam", "int", "0");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Info
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_MDI_INFORMATION_OUTLINE " Each parameter creates an output port on this node.");
    }

    virtual void Initialize() override {
        BaseNodeWidget::Initialize();
    }

private:
    IStoryManager &m_manager;
    std::shared_ptr<FunctionEntryNode> m_functionEntryNode;
};
