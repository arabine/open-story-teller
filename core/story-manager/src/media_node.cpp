#include "media_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"

static std::string ChoiceLabel(const std::string &id)
{
    std::stringstream ss;
    ss << "mediaChoice" << std::setw(4) << std::setfill('0') << id;
    return ss.str();
}

MediaNode::MediaNode(const std::string &type)
    : BaseNode(type, "Media Node")
{
    nlohmann::json j{ {"image", ""}, {"sound", ""}};
    SetInternalData(j);
}

void MediaNode::StoreInternalData()
{
    nlohmann::json j;
    j["image"] = m_image;
    j["sound"] = m_sound;

    SetInternalData(j);
}

void MediaNode::Initialize()
{
    nlohmann::json j = GetInternalData();
    m_image = j["image"].get<std::string>();
    m_sound = j["sound"].get<std::string>();
}

std::string MediaNode::GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns)
{
    std::string s;

    if (m_image.size() > 0)
    {
        s = Compiler::FileToConstant(m_image, Resource::ImageExtension(m_image, project.GetOptions().image_format), project);
    }
    if (m_sound.size() > 0)
    {
        s += Compiler::FileToConstant(m_sound, Resource::SoundExtension(m_sound, project.GetOptions().sound_format), project);  // FIXME: Generate the extension setup in user option of output format
    }

    // Generate choice table if needed (out ports > 1)
    std::stringstream ss;
    std::string label = ChoiceLabel(GetId());
    ss << "$" << label
        << " DC32, "
        << nb_out_conns << ", ";

    std::list<std::shared_ptr<Connection>> conns;
    page.GetNodeConnections(conns, GetId());
    int i = 0;
    for (auto & c : conns)
    {
        std::stringstream ssChoice;

        // On va chercher le label d'entrée du noeud connecté à l'autre bout
        ss << BaseNode::GetEntryLabel(c->inNodeId);
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


    return s;
}

void MediaNode::SetImage(const std::string &image)
{
    m_image = image;
    StoreInternalData();
}

std::string_view MediaNode::GetImage() const
{
    return m_image;
}

void MediaNode::SetSound(const std::string &sound)
{
    m_sound = sound;
    StoreInternalData();
}

std::string_view MediaNode::GetSound() const
{
    return m_sound;
}

std::string MediaNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    std::stringstream ss;

    ss << R"(; ---------------------------- )"
       << GetTitle()
       << " Type: "
       << (nb_out_conns == 0 ? "End" : nb_out_conns == 1 ? "Transition" : "Choice")
       << "\n";

    std::string img = SysLib::RemoveFileExtension(m_image);
    std::string snd = SysLib::RemoveFileExtension(m_sound);

    // Le label de ce noeud est généré de la façon suivante :
    // "media" + Node ID + id du noeud parent. Si pas de noeud parent, alors rien
    ss << BaseNode::GetEntryLabel(GetId()) << ":\n";

    if (img.size() > 0)
    {
        ss << "lcons r0, $" << m_image  << "\n";
    }
    else
    {
        ss << "lcons r0, 0\n";
    }

    if (snd.size() > 0)
    {
        ss << "lcons r1, $" << m_sound  << "\n";
    }
    else
    {
        ss << "lcons r1, 0\n";
    }
    // Call the media executor (image, sound)
    ss << "syscall 1\n";

    // Check output connections number
    // == 0: end node        : wait for home button or return du choice node at the end of the audio
    // == 1: transition node : image + sound on demand, jump directly to the other node when OK
    // > 1 : choice node     : call the node choice manager

    if (nb_out_conns == 0) // End node
    {
        ss << "ret\n";
    }
    else if (nb_out_conns == 1) // it is a transition node
    {
        std::list<std::shared_ptr<Connection>> conns;
        page.GetNodeConnections(conns, GetId());

        for (auto c : conns)
        {
            if (c->outNodeId == GetId())
            {
                // On place dans R0 le prochain noeud à exécuter en cas de OK
                ss << "lcons r0, "
                   << BaseNode::GetEntryLabel(c->inNodeId) << "\n"
                   << "ret\n";
            }
        }

    }
    else // Choice node
    {
        ss << "lcons r0, 0b10000000000\n" // ; mask for end of audio, Ok, and home buttons
           << "syscall 2\n"  // ; wait for event (OK, home or end of audio), return to choice loop"
           << "lcons r0, $" << ChoiceLabel(GetId()) << "\n"
           << "jump .media ; no return possible, so a jump is enough";
    }
    return ss.str();
}

