#include "base_node_widget.h"
#include "uuid.h"

#include "IconsMaterialDesignIcons.h"

unsigned long BaseNodeWidget::s_nextId = 1;

BaseNodeWidget::BaseNodeWidget(IStoryManager &manager,  std::shared_ptr<BaseNode> base)
    : m_manager(manager)
    , m_base(base)
{
    // A node is specific to the Node Gfx library
    m_node = std::make_unique<Node>(GetNextId(), ""); // ImGui internal ID
    std::cout << " --> Created node widget: " << (int)m_node->ID.Get() << std::endl;
}

BaseNodeWidget::~BaseNodeWidget()
{
    std::cout << " <-- Deleted node widget: " << (int)m_node->ID.Get() << std::endl;
}

void BaseNodeWidget::AddInputs(int num)
{
    for (int i = 0; i < num; i++)
    {
        m_node->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    }
}

void BaseNodeWidget::AddOutputs(int num)
{
   for (int i = 0; i < num; i++)
   {
        m_node->Outputs.emplace_back(GetNextId(), "", PinType::Flow);
   }
}

void BaseNodeWidget::SetInputs(uint32_t num)
{
   if (num > Inputs())
   {
        AddInputs(num - Inputs());
   }
   else if (num < Inputs())
   {
        for (unsigned int i = 0; i < (Inputs() - num); i++)
        {
            m_node->Inputs.pop_back();
        }
   }
}

void BaseNodeWidget::SetOutputs(uint32_t num)
{
   if (num > Outputs())
   {
        AddOutputs(num - Outputs());
   }
   else if (num < Outputs())
   {
        for (unsigned int i = 0; i < (Outputs() - num); i++)
        {
            DeleteOutput();
        }
   }
}

void BaseNodeWidget::DeleteOutput()
{
   m_node->Outputs.pop_back();
}


void BaseNodeWidget::Initialize()
{
    m_firstFrame = true;
    
}


void BaseNodeWidget::SetInputPinName(int pinIndex, const std::string &name)
{
    if (pinIndex < m_node->Inputs.size())
    {
        m_node->Inputs[pinIndex].Name = name;
    }
}

void BaseNodeWidget::SetOutPinName(int pinIndex, const std::string &name)
{
    if (pinIndex < m_node->Outputs.size())
    {
        m_node->Outputs[pinIndex].Name = name;
    }
}

void BaseNodeWidget::FrameStart()
{
    ed::BeginNode(m_node->ID);

    if (m_firstFrame)
    {
        if (m_base)
        {
            // Use the parent node position, the one saved in the JSON project
            // FIXME: find a better way to do that?
            ed::SetNodePosition(m_node->ID, ImVec2(m_base->GetX(), m_base->GetY()));
        }
    }
    else
    {
        // Si ce n'est pas la première frame, on synchronise la position du noeud avec l'objet
        if (m_base)
        {
            m_base->SetPosition(GetX(), GetY());
        }
    }
    m_firstFrame = false;


    // Title

    const char * text = m_base->GetTypeName().c_str();
    // Obtenir la position courante du curseur
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Définir les dimensions du texte
    ImVec2 text_size = ImGui::CalcTextSize(text);

    // Ajouter un padding autour du texte
    float padding = 5.0f;
    ImVec2 rect_min = ImVec2(pos.x - padding, pos.y - padding);
    ImVec2 rect_max = ImVec2(pos.x + text_size.x + padding, pos.y + text_size.y + padding);

    // Définir la couleur du rectangle (bleu avec transparence)
    ImU32 bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 1.0f));

    // Dessiner le rectangle de fond
    ImGui::GetWindowDrawList()->AddRectFilled(rect_min, rect_max, bg_color);

    // Afficher le texte
    ImGui::TextUnformatted(text);
}

void BaseNodeWidget::FrameEnd()
{
    ed::EndNode();
}

void BaseNodeWidget::DrawPins()
{
    static const char *str = "#1 >";
    static float textWidth = ImGui::CalcTextSize(str).x;

    for (auto& input : m_node->Inputs)
    {
        ed::BeginPin(input.ID, ed::PinKind::Input);

        ImGui::Text( ICON_MDI_OCTAGON_OUTLINE " In" );
        ed::EndPin();
    }

    int i = 1;
    for (auto& output : m_node->Outputs)
    {
        ImGui::Dummy(ImVec2(320 - textWidth * 2, 0)); // Hacky magic number to space out the output pin.
        ImGui::SameLine();
        ed::BeginPin(output.ID, ed::PinKind::Output);
        if (output.Name.empty())
        {
            ImGui::Text( "#%d " ICON_MDI_OCTAGON_OUTLINE, i++);
        }
        else
        {
            ImGui::Text( "%s" ICON_MDI_OCTAGON_OUTLINE, output.Name.c_str()  );
        }
        ed::EndPin();
    }
}

float BaseNodeWidget::GetX() const
{
    auto pos = GetNodePosition(m_node->ID);
    return pos.x;
}

float BaseNodeWidget::GetY() const
{
    auto pos = GetNodePosition(m_node->ID);
    return pos.y;
}
