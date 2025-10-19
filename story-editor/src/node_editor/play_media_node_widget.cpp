// story-editor/src/node_editor/play_media_node_widget.cpp
#include "play_media_node_widget.h"
#include "IconsMaterialDesignIcons.h"

PlayMediaNodeWidget::PlayMediaNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
{
    m_playMediaNode = std::dynamic_pointer_cast<PlayMediaNode>(node);
}

void PlayMediaNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
}

void PlayMediaNodeWidget::DrawProperties(std::shared_ptr<IStoryProject> story)
{
    ImGui::AlignTextToFramePadding();
    
    ImGui::TextWrapped("This node plays media (image and/or sound) at runtime.");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Data Inputs:");
    ImGui::BulletText("Port 1: Image name (string)");
    ImGui::Indent();
    ImGui::TextDisabled("Variable or constant with filename");
    ImGui::TextDisabled("Pass 0 or null for no image");
    ImGui::Unindent();
    
    ImGui::BulletText("Port 2: Sound name (string)");
    ImGui::Indent();
    ImGui::TextDisabled("Variable or constant with filename");
    ImGui::TextDisabled("Pass 0 or null for no sound");
    ImGui::Unindent();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Note:");
    ImGui::TextWrapped("Connect string variables/constants to the data input ports. "
                      "The syscall expects filenames relative to the assets folder.");
}

void PlayMediaNodeWidget::Draw()
{
    ImGui::Text(ICON_MDI_PLAY_CIRCLE " Play Media");
    ImGui::TextDisabled("Data-driven");
    ImGui::TextColored(ImVec4(0.6f, 0.9f, 1.0f, 1.0f), "Image: IN 1");
    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.9f, 1.0f), "Sound: IN 2");
}
