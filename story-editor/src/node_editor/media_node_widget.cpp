
#include <sstream>
#include "media_node_widget.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"

MediaNodeWidget::MediaNodeWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
    : BaseNodeWidget(manager, node)
    , m_manager(manager)
{
    m_mediaNode = std::dynamic_pointer_cast<MediaNode>(node);

    // Create defaut one input and one output
    AddInput();
    AddOutputs(1);

    std::string widgetId =  std::to_string(GetInternalId()); // Make widget unique by using the node ID

    m_buttonUniqueName = "Play " ICON_MDI_PLAY "##id" + widgetId;
}

void MediaNodeWidget::Draw()
{
    BaseNodeWidget::FrameStart();


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


    if (m_image.Valid())
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
    std::string leftSingle = "##left" + m_mediaNode->GetId();
    if (ImGui::ArrowButton(leftSingle.c_str(), ImGuiDir_Left)) { if (counter > 1) counter--; }
    ImGui::SameLine(0.0f, spacing);

    std::string rightSingle = "##right" + m_mediaNode->GetId();
    if (ImGui::ArrowButton(rightSingle.c_str(), ImGuiDir_Right))
    {
        counter++;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", counter);

    SetOutputs(counter);

    DrawPins();

    BaseNodeWidget::FrameEnd();

}

/*

"internal-data": {
                    "image": "fairy.png",
                    "sound": "la_fee_luminelle.mp3"
                },
*/
void MediaNodeWidget::Initialize()
{
    BaseNodeWidget::Initialize();
    nlohmann::json j = m_mediaNode->GetInternalData();
    SetImage(j["image"].get<std::string>());
    SetSound(j["sound"].get<std::string>());
}


void MediaNodeWidget::StoreInternalData()
{
    nlohmann::json j;
    j["image"] = m_mediaNode->image;
    j["sound"] = m_mediaNode->sound;

    m_mediaNode->SetInternalData(j);
}

void MediaNodeWidget::DrawProperties()
{
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Image");
    ImGui::SameLine();

    ImGui::Text("%s", m_mediaNode->image.c_str());

    ImGui::SameLine();

    static bool isImage = true;
    if (ImGui::Button("Select...##image")) {
        ImGui::OpenPopup("popup_button");
        isImage = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_CLOSE_BOX_OUTLINE "##delimage")) {
        SetImage("");
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Sound");
    ImGui::SameLine();

    ImGui::Text("%s", m_mediaNode->sound.c_str());

    ImGui::SameLine();

    if (ImGui::Button(m_buttonUniqueName.c_str()))
    {
        m_manager.PlaySoundFile(m_soundPath);
    }

    ImGui::SameLine();

    if (ImGui::Button("Select...##sound")) {
        ImGui::OpenPopup("popup_button");
        isImage = false;
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_CLOSE_BOX_OUTLINE "##delsound")) {
        SetSound("");
    }

    // This is the actual popup Gui drawing section.
    if (ImGui::BeginPopup("popup_button")) {
        ImGui::SeparatorText(isImage ? "Images" : "Sounds");

        auto [filtreDebut, filtreFin] = isImage ? m_manager.Images() : m_manager.Sounds();
        int n = 0;
        for (auto it = filtreDebut; it != filtreFin; ++it, n++)
        {
            if (ImGui::Selectable((*it)->file.c_str()))
            {
                if (isImage)
                {
                    SetImage((*it)->file);
                }
                else
                {
                    SetSound((*it)->file);
                }
            }
        }

        ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
    }

}

void MediaNodeWidget::SetImage(const std::string &f)
{
    m_mediaNode->image = f;
    m_image.Load(m_manager.BuildFullAssetsPath(f));
    StoreInternalData();
}

void MediaNodeWidget::SetSound(const std::string &f)
{
    m_mediaNode->sound = f;
    m_soundPath = m_manager.BuildFullAssetsPath(m_mediaNode->sound);
    StoreInternalData();
}


