// story-editor/src/node_editor/wait_event_node_widget.cpp
#include "wait_event_node_widget.h"
#include "IconsMaterialDesignIcons.h"

WaitEventNodeWidget::WaitEventNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
{
    m_waitEventNode = std::dynamic_pointer_cast<WaitEventNode>(node);
}

void WaitEventNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}

void WaitEventNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    
    // Event Mask Editor
    ImGui::Text("Event Mask:");
    DrawEventMaskEditor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Timeout
    ImGui::Text("Timeout:");
    int timeout = static_cast<int>(m_waitEventNode->GetTimeout());
    if (ImGui::InputInt("ms##timeout", &timeout, 100, 1000))
    {
        if (timeout < 0) timeout = 0;
        m_waitEventNode->SetTimeout(static_cast<uint32_t>(timeout));
    }
    
    ImGui::SameLine();
    ImGui::TextDisabled("(0 = infinite)");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Result Variable
    ImGui::Text("Result Variable:");
    std::string resultVarUuid = m_waitEventNode->GetResultVariableUuid();
    std::string selectedVarName = "(none - use R0 directly)";
    
    // Trouver le nom de la variable sélectionnée
    story->ScanVariable([this, &resultVarUuid, &selectedVarName](std::shared_ptr<Variable> var) {
        if (var->GetUuid() == resultVarUuid) {
            selectedVarName = var->GetVariableName();
            return false;
        }
        return true;
    });

    
    if (ImGui::BeginCombo("##resultvar", selectedVarName.c_str())) {
        if (ImGui::Selectable("(none - use R0 directly)", resultVarUuid.empty())) {
            m_waitEventNode->SetResultVariable("");
        }

        int i = 0;
        story->ScanVariable([&i, &resultVarUuid, this](std::shared_ptr<Variable> var) {
            if (var->GetValueType() == Variable::ValueType::INTEGER) {
                bool isSelected = (var->GetUuid() == resultVarUuid);
                if (ImGui::Selectable(var->GetVariableName().c_str(), isSelected)) {
                    m_waitEventNode->SetResultVariable(var->GetUuid());
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            i++;
            return true;
        });

        ImGui::EndCombo();
    }

    
    ImGui::Spacing();
    ImGui::TextDisabled("The event code will be stored in this variable");
    ImGui::TextDisabled("or in R0 if no variable is selected");
}

void WaitEventNodeWidget::DrawEventMaskEditor()
{
    uint32_t mask = m_waitEventNode->GetEventMask();
    bool changed = false;
    
    // Affichage hexadécimal
    ImGui::Text("Hex: 0x%08X", mask);
    
    ImGui::Spacing();
    
    // Bits individuels (exemple de masque d'événements communs)
    // Bit 0-7: Boutons numériques
    // Bit 8: OK button
    // Bit 9: Home button  
    // Bit 10: End of audio
    
    ImGui::BeginGroup();
    
    bool bit0 = (mask & (1 << 0)) != 0;
    if (ImGui::Checkbox("Bit 0 (Button 0)", &bit0)) {
        mask = bit0 ? (mask | (1 << 0)) : (mask & ~(1 << 0));
        changed = true;
    }
    
    bool bit1 = (mask & (1 << 1)) != 0;
    if (ImGui::Checkbox("Bit 1 (Button 1)", &bit1)) {
        mask = bit1 ? (mask | (1 << 1)) : (mask & ~(1 << 1));
        changed = true;
    }
    
    bool bit2 = (mask & (1 << 2)) != 0;
    if (ImGui::Checkbox("Bit 2 (Button 2)", &bit2)) {
        mask = bit2 ? (mask | (1 << 2)) : (mask & ~(1 << 2));
        changed = true;
    }
    
    bool bit3 = (mask & (1 << 3)) != 0;
    if (ImGui::Checkbox("Bit 3 (Button 3)", &bit3)) {
        mask = bit3 ? (mask | (1 << 3)) : (mask & ~(1 << 3));
        changed = true;
    }
    
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    
    bool bit8 = (mask & (1 << 8)) != 0;
    if (ImGui::Checkbox("Bit 8 (OK Button)", &bit8)) {
        mask = bit8 ? (mask | (1 << 8)) : (mask & ~(1 << 8));
        changed = true;
    }
    
    bool bit9 = (mask & (1 << 9)) != 0;
    if (ImGui::Checkbox("Bit 9 (Home Button)", &bit9)) {
        mask = bit9 ? (mask | (1 << 9)) : (mask & ~(1 << 9));
        changed = true;
    }
    
    bool bit10 = (mask & (1 << 10)) != 0;
    if (ImGui::Checkbox("Bit 10 (End Audio)", &bit10)) {
        mask = bit10 ? (mask | (1 << 10)) : (mask & ~(1 << 10));
        changed = true;
    }
    
    ImGui::EndGroup();
    
    ImGui::Spacing();
    
    // Presets communs
    if (ImGui::Button("Preset: All Events")) {
        mask = 0xFFFFFFFF;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Preset: Media End")) {
        mask = 0b10000000000;  // OK + Home + EndAudio
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        mask = 0;
        changed = true;
    }
    
    if (changed) {
        m_waitEventNode->SetEventMask(mask);
    }
}

void WaitEventNodeWidget::Draw()
{
    uint32_t mask = m_waitEventNode->GetEventMask();
    uint32_t timeout = m_waitEventNode->GetTimeout();
    
    ImGui::Text(ICON_MDI_TIMER_SAND " Wait Event");
    
    // Description du masque
    std::string desc = GetEventMaskDescription(mask);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "%s", desc.c_str());
    
    // Timeout si défini
    if (timeout > 0) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                          "Timeout: %u ms", timeout);
    }
}

std::string WaitEventNodeWidget::GetEventMaskDescription(uint32_t mask)
{
    if (mask == 0) {
        return "No events";
    }
    if (mask == 0xFFFFFFFF) {
        return "All events";
    }
    
    // Compter les bits actifs
    int bitCount = 0;
    for (int i = 0; i < 32; i++) {
        if (mask & (1 << i)) bitCount++;
    }
    
    if (bitCount == 1) {
        // Un seul événement
        for (int i = 0; i < 32; i++) {
            if (mask & (1 << i)) {
                if (i == 8) return "OK button";
                if (i == 9) return "Home button";
                if (i == 10) return "End audio";
                return "Event bit " + std::to_string(i);
            }
        }
    }
    
    // Plusieurs événements
    return std::to_string(bitCount) + " events";
}