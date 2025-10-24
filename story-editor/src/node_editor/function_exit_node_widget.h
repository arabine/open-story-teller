#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "function_exit_node.h"
#include "gui.h"
#include "IconsMaterialDesignIcons.h"

class FunctionExitNodeWidget : public BaseNodeWidget
{
public:
    FunctionExitNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_functionExitNode = std::dynamic_pointer_cast<FunctionExitNode>(node);
        SetTitle("Function Exit");
    }

    void Draw() override {
        ImGui::TextUnformatted(m_functionExitNode->GetExitLabel().c_str());
        if (m_functionExitNode->GetReturnValueCount() > 0) {
            ImGui::Text(ICON_MDI_ARROW_LEFT " %zu return value(s)", m_functionExitNode->GetReturnValueCount());
        }
    }

    virtual bool HasSync() const override { 
        return false;
    }

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override {
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), ICON_MDI_EXPORT " Function Exit");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("This node defines an exit point for the function/module.");
        ImGui::Text("It returns values and terminates the function execution.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Exit label
        ImGui::Text("Exit Label:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.0f);
        
        char labelBuf[128];
        strncpy(labelBuf, m_functionExitNode->GetExitLabel().c_str(), sizeof(labelBuf) - 1);
        labelBuf[sizeof(labelBuf) - 1] = '\0';
        
        if (ImGui::InputTextWithHint("##exit_label", "Success / Error / Cancel...", labelBuf, sizeof(labelBuf))) {
            m_functionExitNode->SetExitLabel(labelBuf);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Return values list
        ImGui::Text(ICON_MDI_ARROW_LEFT_BOLD " Return Values:");
        ImGui::Spacing();

        const auto& returnValues = m_functionExitNode->GetReturnValues();
        
        // Table for return values
        if (ImGui::BeginTable("return_values_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            // Display existing return values
            for (size_t i = 0; i < returnValues.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::PushID(static_cast<int>(i));

                // Name
                ImGui::TableNextColumn();
                char nameBuf[128];
                strncpy(nameBuf, returnValues[i].name.c_str(), sizeof(nameBuf) - 1);
                nameBuf[sizeof(nameBuf) - 1] = '\0';
                
                if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf))) {
                    m_functionExitNode->UpdateReturnValue(i, nameBuf, returnValues[i].type);
                }

                // Type
                ImGui::TableNextColumn();
                const char* types[] = {"int", "float", "string", "bool"};
                int currentType = 0;
                for (int t = 0; t < 4; ++t) {
                    if (returnValues[i].type == types[t]) {
                        currentType = t;
                        break;
                    }
                }
                
                if (ImGui::Combo("##type", &currentType, types, 4)) {
                    m_functionExitNode->UpdateReturnValue(i, returnValues[i].name, types[currentType]);
                }

                // Actions
                ImGui::TableNextColumn();
                if (ImGui::Button(ICON_MDI_DELETE "##delete")) {
                    m_functionExitNode->RemoveReturnValue(i);
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();

        // Add new return value
        if (ImGui::Button(ICON_MDI_PLUS " Add Return Value")) {
            m_functionExitNode->AddReturnValue("result", "int");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Info
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_MDI_INFORMATION_OUTLINE " Each return value creates an input port on this node.");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "The exit label will be the name of the execution output port on CallFunctionNode.");
    }

    virtual void Initialize() override {
        BaseNodeWidget::Initialize();
    }

private:
    IStoryManager &m_manager;
    std::shared_ptr<FunctionExitNode> m_functionExitNode;
};