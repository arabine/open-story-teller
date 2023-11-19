#include "media_node.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "IconsFontAwesome5_c.h"

MediaNode::MediaNode(const std::string &title)
    : BaseNode(title)
{
    Gui::LoadRawImage("fairy.png", m_image);

    // Create defaut one input and one output
    AddInput();
    AddOutput();

}

void MediaNode::Draw()
{
    BaseNode::FrameStart();


    static ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                   ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 10.0f));
    if (ImGui::BeginTable("table1", 1, flags)) {
        ImGui::TableNextRow();
        ImU32 bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 1.0f));
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color);
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Media node");

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();


    if (m_image.valid())
    {
        ImGui::Image(m_image.texture, ImVec2(320, 240));
    }
    else
    {
        ImGui::Dummy(ImVec2(320, 240));
    }

    // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements
    // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Image");
    ImGui::SameLine();

    ImGui::Text("image.png");

    ImGui::SameLine();

    bool do_select = false;
    if (ImGui::Button("Select")) {
        do_select = true;	// Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
    }

    // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements
    // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Sound");
    ImGui::SameLine();

    ImGui::Text("sound.mp3");

    ImGui::SameLine();

    bool do_select_sound = false;
    if (ImGui::Button("Select")) {
        do_select_sound = true;	// Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
    }


    ImGui::AlignTextToFramePadding();
    ImGui::Text("Hold to repeat:");
    ImGui::SameLine();

    // Arrow buttons with Repeater

    uint32_t counter = Outputs();
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    ImGui::PushButtonRepeat(true);
    std::string leftSingle = "##left" + GetId();
    if (ImGui::ArrowButton(leftSingle.c_str(), ImGuiDir_Left)) { if (counter > 1) counter--; }
    ImGui::SameLine(0.0f, spacing);

    std::string rightSingle = "##right" + GetId();
    if (ImGui::ArrowButton(rightSingle.c_str(), ImGuiDir_Right))
    {
        counter++;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", counter);

    if (counter > Outputs())
    {
        for (int i = 0; i < (counter - Outputs()); i++)
        {
            AddOutput();
        }
    }
    else if (counter < Outputs())
    {
        for (int i = 0; i < (Outputs() - counter); i++)
        {
            DeleteOutput();
        }
    }


    DrawPins();

    BaseNode::FrameEnd();
}
