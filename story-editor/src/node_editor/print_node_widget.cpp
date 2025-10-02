#include <sstream>
#include "print_node_widget.h"

#include "IconsMaterialDesignIcons.h"
#include "story_project.h"
#include "uuid.h"

char PrintNodeWidget::m_buffer[PrintNodeWidget::MAX_PRINT_SIZE] = {0};

PrintNodeWidget::PrintNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_printNode = std::dynamic_pointer_cast<PrintNode>(node);
    SetTitle("Print");
}

void PrintNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
    
    // Copy current text to buffer
    auto text = m_printNode->GetText();
    if (text.size() < MAX_PRINT_SIZE) {
        text.copy(m_buffer, text.size());
        m_buffer[text.size()] = '\0';
    }
}

void PrintNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Format string:");
    
    ImGui::PushItemWidth(200.0f);

    // Edit the format string
    if (ImGui::InputText("##format", m_buffer, sizeof(m_buffer))) {
        m_printNode->SetText(m_buffer);
    }
    
    ImGui::PopItemWidth();
    
    // Show help text
    ImGui::TextDisabled("Use {0}, {1}, {2}, {3} for arguments");
    
    // Display current text
    ImGui::Separator();
    ImGui::Text("Preview: %s", m_printNode->GetText().c_str());
}

void PrintNodeWidget::Draw()
{
    // Display format string in the node body
    std::string displayText = m_printNode->GetText();
    if (displayText.empty()) {
        displayText = "<empty>";
    }
    
    // Truncate if too long
    if (displayText.length() > 30) {
        displayText = displayText.substr(0, 27) + "...";
    }
    
    ImGui::TextUnformatted(displayText.c_str());
}