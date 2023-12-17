#include "media_node.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"


MediaNode::MediaNode(const std::string &title, IStoryProject &proj)
    : BaseNode(title, proj)
    , m_project(proj)
{
    Gui::LoadRawImage("fairy.png", m_image);

    // Create defaut one input and one output
    AddInput();
    AddOutputs(1);


    std::string widgetId =  std::to_string(GetInternalId()); // Make widget unique by using the node ID

    m_buttonUniqueName = "Play " ICON_MDI_PLAY "##id" + widgetId;

}

void MediaNode::Draw()
{
    BaseNode::FrameStart();


    static ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                   ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 10.0f));
    if (ImGui::BeginTable("table1", 1, flags))
    {
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



    // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements
    // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)


    ImGui::AlignTextToFramePadding();
    ImGui::Text("Outputs:");
    ImGui::SameLine();

    // Arrow buttons with Repeater

    uint32_t counter = Outputs();
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    ImGui::PushButtonRepeat(true);
    std::string leftSingle = "##left" + std::to_string(GetId());
    if (ImGui::ArrowButton(leftSingle.c_str(), ImGuiDir_Left)) { if (counter > 1) counter--; }
    ImGui::SameLine(0.0f, spacing);

    std::string rightSingle = "##right" + std::to_string(GetId());
    if (ImGui::ArrowButton(rightSingle.c_str(), ImGuiDir_Right))
    {
        counter++;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", counter);

    SetOutputs(counter);

    DrawPins();



    BaseNode::FrameEnd();

}

/*

"internal-data": {
                    "image": "fairy.png",
                    "sound": "la_fee_luminelle.mp3"
                },

*/
void MediaNode::FromJson(const nlohmann::json &j)
{
    m_image.name = j["image"].get<std::string>();

    Gui::LoadRawImage(m_project.BuildFullAssetsPath(m_image.name), m_image);

    m_soundName = j["sound"].get<std::string>();
    m_soundPath = m_project.BuildFullAssetsPath(m_soundName);
}

void MediaNode::ToJson(nlohmann::json &j)
{
    j["image"] = m_image.name;
    j["sound"] = m_soundName;
}

void MediaNode::DrawProperties()
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Image");
    ImGui::SameLine();

    ImGui::Text("%s", m_image.name.c_str());

    ImGui::SameLine();

    std::string type = "sound";
    if (ImGui::Button("Select...##image")) {
        type = "image";
        ImGui::OpenPopup("popup_button");
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Sound");
    ImGui::SameLine();

    ImGui::Text("%s", m_soundName.c_str());

    ImGui::SameLine();

    if (ImGui::Button(m_buttonUniqueName.c_str()))
    {
        m_project.PlaySoundFile(m_soundPath);
    }

    if (ImGui::Button("Select...##sound")) {
        ImGui::OpenPopup("popup_button");
    }


    // This is the actual popup Gui drawing section.
    if (ImGui::BeginPopup("popup_button")) {
        ImGui::SeparatorText("Sounds");

        static int item_current_idx = 0; // Here we store our selection data as an index.


        auto [filtreDebut, filtreFin] = m_project.Sounds();
        int n = 0;
        for (auto it = filtreDebut; it != filtreFin; ++it, n++)
        {
            if (ImGui::Selectable((*it)->file.c_str()), n == item_current_idx)
                item_current_idx = n;
        }

        ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
    }

}
