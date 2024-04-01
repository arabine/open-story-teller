
#include <sstream>
#include "media_node.h"

namespace ed = ax::NodeEditor;
#include "IconsMaterialDesignIcons.h"
#include "story_project.h"

MediaNode::MediaNode(const std::string &title, IStoryManager &proj)
    : BaseNode(title, proj)
    , m_story(proj)
{
    // Gui::LoadRawImage("fairy.png", m_image);

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
    SetImage(j["image"].get<std::string>());
    SetSound(j["sound"].get<std::string>());
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

    ImGui::Text("%s", m_soundName.c_str());

    ImGui::SameLine();

    if (ImGui::Button(m_buttonUniqueName.c_str()))
    {
        m_story.PlaySoundFile(m_soundPath);
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

        auto [filtreDebut, filtreFin] = isImage ? m_story.Images() : m_story.Sounds();
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

void MediaNode::SetImage(const std::string &f)
{
    m_image.name = f;
    m_image.Load(m_story.BuildFullAssetsPath(f));
}

void MediaNode::SetSound(const std::string &f)
{
    m_soundName = f;
    m_soundPath = m_story.BuildFullAssetsPath(m_soundName);
}




std::string MediaNode::ChoiceLabel() const
{
    std::stringstream ss;
    ss << "mediaChoice" << std::setw(4) << std::setfill('0') << GetId();
    return ss.str();
}

std::string MediaNode::GetEntryLabel()
{
    std::stringstream ss;
    ss << ".mediaEntry" << std::setw(4) << std::setfill('0') << GetId();
    return ss.str();
}


std::string MediaNode::GenerateConstants()
{
    std::string s;

    if (m_image.name.size() > 0)
    {
        s = StoryProject::FileToConstant(m_image.name, ".qoi");  // FIXME: Generate the extension setup in user option of output format
    }
    if (m_soundName.size() > 0)
    {
        s += StoryProject::FileToConstant(m_soundName, ".wav");  // FIXME: Generate the extension setup in user option of output format
    }

    int nb_out_conns = Outputs();
    if (nb_out_conns > 1)
    {
        // Generate choice table if needed (out ports > 1)
        std::stringstream ss;
        std::string label = ChoiceLabel();
        ss << "$" << label
           << " DC32, "
           << nb_out_conns << ", ";

        std::list<std::shared_ptr<Connection>> conns = m_story.GetNodeConnections(GetId());
        int i = 0;
        for (auto & c : conns)
        {
            std::stringstream ssChoice;

            // On va chercher le label d'entrée du noeud connecté à l'autre bout
            ss << m_story.GetNodeEntryLabel(c->inNodeId);
            if (i < (nb_out_conns - 1))
            {
                ss << ", ";
            }
            else
            {
                ss << "\n";
            }
            i++;
        }

        s += ss.str();
    }

    return s;
}

std::string MediaNode::Build()
{
    std::stringstream ss;
    int nb_out_conns = Outputs();

    ss << R"(; ---------------------------- )"
       << GetTitle()
       << " Type: "
       << (nb_out_conns == 0 ? "End" : nb_out_conns == 1 ? "Transition" : "Choice")
       << "\n";
    std::string image = StoryProject::RemoveFileExtension(m_image.name);
    std::string sound = StoryProject::RemoveFileExtension(m_soundName);

    // Le label de ce noeud est généré de la façon suivante :
    // "media" + Node ID + id du noeud parent. Si pas de noeud parent, alors rien
    ss << GetEntryLabel() << ":\n";

    if (image.size() > 0)
    {
        ss << "lcons r0, $" << image  << "\n";
    }
    else
    {
        ss << "lcons r0, 0\n";
    }

    if (sound.size() > 0)
    {
        ss << "lcons r1, $" << sound  << "\n";
    }
    else
    {
        ss << "lcons r1, 0\n";
    }
    // Call the media executor (image, sound)
    ss << "syscall 1\n";

    // Check output connections number
    // == 0: end node        : generate halt
    // == 1: transition node : image + sound on demand, jump directly to the other node when OK
    // > 1 : choice node     : call the node choice manager

    if (nb_out_conns == 0) // End node
    {
        ss << "halt\n";
    }
    else if (nb_out_conns == 1) // it is a transition node
    {
        std::list<std::shared_ptr<Connection>> conns = m_story.GetNodeConnections(GetId());


        for (auto c : conns)
        {
            if (c->outNodeId == GetId())
            {
                // On place dans R0 le prochain noeud à exécuter en cas de OK
                ss << "lcons r0, "
                   << m_story.GetNodeEntryLabel(c->inNodeId) << "\n"
                   << "ret\n";
            }
        }

    }
    else // Choice node
    {
        ss << "lcons r0, $" << ChoiceLabel() << "\n"
           << "jump .media ; no return possible, so a jump is enough";
    }
    return ss.str();
}

