// call_function_node_widget.h
#pragma once

#include <vector>
#include <map>
#include <string>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "call_function_node.h"
#include "function_entry_node.h"
#include "function_exit_node.h"
#include "gui.h"
#include "IconsMaterialDesignIcons.h"

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
        
        // If a function is selected, rebuild ports
        if (!m_functionUuid.empty()) {
            RebuildPortsFromModule();
        }
    }

    void DrawProperties(std::shared_ptr<IStoryProject> story) override {
        ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), ICON_MDI_FUNCTION " Call Function");
        ImGui::Separator();
        ImGui::Spacing();

        // Function selection
        ImGui::Text("Select function/module:");
        ImGui::Spacing();
        
        if (ImGui::BeginCombo("##function", m_functionName.empty() ? "<Select function>" : m_functionName.c_str())) {
            // Get list of available functions/modules
            auto functions = story->GetFunctionsList();
            
            for (size_t i = 0; i < functions.size(); ++i) {
                const bool is_selected = (m_functionUuid == functions[i].uuid);
                
                if (ImGui::Selectable(functions[i].name.c_str(), is_selected)) {
                    m_functionUuid = functions[i].uuid;
                    m_functionName = functions[i].name;
                    m_callFunctionNode->SetFunction(m_functionUuid, m_functionName);
                    
                    // Rebuild ports based on the selected module
                    RebuildPortsFromModule();
                }
                
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();

        // Open function button
        if (m_functionUuid.empty()) {
            ImGui::BeginDisabled();
        }
        
        if (ImGui::Button(ICON_MDI_OPEN_IN_NEW " Open function")) {
            m_manager.OpenFunction(m_functionUuid, m_functionName);
        }
        
        if (m_functionUuid.empty()) {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // If a function is selected, show input/output configuration
        if (!m_functionUuid.empty() && !m_moduleParameters.empty()) {
            DrawInputBindingsUI();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            DrawOutputMappingsUI(story);
        }
    }

    void Draw() override {
        ImGui::TextUnformatted(m_functionName.empty() 
            ? "<No function>" 
            : m_functionName.c_str());
    }

private:
    IStoryManager &m_manager;
    std::shared_ptr<CallFunctionNode> m_callFunctionNode;
    std::string m_functionName;
    std::string m_functionUuid;
    
    // Cache of module interface
    std::vector<std::pair<std::string, std::string>> m_moduleParameters;  // name, type
    std::vector<std::string> m_exitLabels;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> m_returnValuesByExit;
    
    void RebuildPortsFromModule() {
        // Get the module from NodesFactory
        auto module = m_manager.GetNodesFactory().GetModule(m_functionUuid);
        if (!module) {
            return;
        }
        
        // Find FunctionEntryNode in the module
        m_moduleParameters.clear();
        std::shared_ptr<FunctionEntryNode> entryNode = nullptr;
        
        module->ScanNodes([&](std::shared_ptr<BaseNode> node) {
            if (node->GetType() == "function-entry-node") {
                entryNode = std::dynamic_pointer_cast<FunctionEntryNode>(node);
                return false;
            }
            return true;
        });
        
        if (entryNode) {
            for (const auto& param : entryNode->GetParameters()) {
                m_moduleParameters.push_back({param.name, param.type});
            }
        }
        
        // Find all FunctionExitNodes in the module
        m_exitLabels.clear();
        m_returnValuesByExit.clear();
        std::vector<std::shared_ptr<FunctionExitNode>> exitNodes;
        
        module->ScanNodes([&](std::shared_ptr<BaseNode> node) {
            if (node->GetType() == "function-exit-node") {
                auto exitNode = std::dynamic_pointer_cast<FunctionExitNode>(node);
                if (exitNode) {
                    exitNodes.push_back(exitNode);
                    m_exitLabels.push_back(exitNode->GetExitLabel());
                    
                    std::vector<std::pair<std::string, std::string>> returnValues;
                    for (const auto& rv : exitNode->GetReturnValues()) {
                        returnValues.push_back({rv.name, rv.type});
                    }
                    m_returnValuesByExit[exitNode->GetExitLabel()] = returnValues;
                }
            }
            return true;
        });
        
        // Rebuild the node's ports
        std::vector<std::pair<std::string, std::string>> exitLabelsWithType;
        for (const auto& label : m_exitLabels) {
            exitLabelsWithType.push_back({label, "execution"});
        }
        
        m_callFunctionNode->RebuildPortsFromModule(m_moduleParameters, exitLabelsWithType, m_returnValuesByExit);
    }
    
    void DrawInputBindingsUI() {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), ICON_MDI_ARROW_RIGHT " Input Parameters");
        ImGui::Spacing();
        ImGui::Text("Configure how each parameter receives its value:");
        ImGui::Spacing();
        
        if (ImGui::BeginTable("input_bindings_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Constant Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableHeadersRow();
            
            for (const auto& param : m_moduleParameters) {
                ImGui::TableNextRow();
                ImGui::PushID(param.first.c_str());
                
                // Parameter name
                ImGui::TableNextColumn();
                ImGui::Text("%s (%s)", param.first.c_str(), param.second.c_str());
                
                // Mode selection
                ImGui::TableNextColumn();
                auto binding = m_callFunctionNode->GetInputBinding(param.first);
                int currentMode = (binding && binding->mode == CallFunctionNode::MODE_CONSTANT) ? 1 : 0;
                
                const char* modes[] = {"Connected", "Constant"};
                if (ImGui::Combo("##mode", &currentMode, modes, 2)) {
                    auto mode = (currentMode == 1) ? CallFunctionNode::MODE_CONSTANT : CallFunctionNode::MODE_CONNECTED;
                    std::string value = binding ? binding->constantValue : "";
                    m_callFunctionNode->SetInputBindingMode(param.first, mode, value);
                }
                
                // Constant value (only if mode is constant)
                ImGui::TableNextColumn();
                if (currentMode == 1) {
                    char valueBuf[256] = "";
                    if (binding) {
                        strncpy(valueBuf, binding->constantValue.c_str(), sizeof(valueBuf) - 1);
                    }
                    
                    if (ImGui::InputText("##value", valueBuf, sizeof(valueBuf))) {
                        m_callFunctionNode->SetInputBindingMode(param.first, CallFunctionNode::MODE_CONSTANT, valueBuf);
                    }
                } else {
                    ImGui::TextDisabled("(from pin)");
                }
                
                ImGui::PopID();
            }
            
            ImGui::EndTable();
        }
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_MDI_INFORMATION_OUTLINE " Use 'Connected' to get values from connected pins, 'Constant' to set a fixed value.");
    }
    
    void DrawOutputMappingsUI(std::shared_ptr<IStoryProject> story) {
        ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.2f, 1.0f), ICON_MDI_ARROW_LEFT " Return Values");
        ImGui::Spacing();
        
        // Show all return values grouped by exit
        for (const auto& exitPair : m_returnValuesByExit) {
            ImGui::Text("Exit: %s", exitPair.first.c_str());
            ImGui::Indent();
            
            for (const auto& rv : exitPair.second) {
                ImGui::BulletText("%s (%s)", rv.first.c_str(), rv.second.c_str());
            }
            
            ImGui::Unindent();
            ImGui::Spacing();
        }
        
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_MDI_INFORMATION_OUTLINE " Return values are available on output pins.");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Connect them to use the returned data.");
    }
};