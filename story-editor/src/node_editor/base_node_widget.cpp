#include "base_node_widget.h"
#include "uuid.h"
#include "imgui.h"
#include "IconsMaterialDesignIcons.h"

unsigned long BaseNodeWidget::s_nextId = 1;

BaseNodeWidget::BaseNodeWidget(IStoryManager &manager,  std::shared_ptr<BaseNode> base)
    : m_manager(manager)
    , m_base(base)
{
    // A node is specific to the Node Gfx library
    std::cout << " --> Created node widget: " << m_base->GetId() << std::endl;
}

BaseNodeWidget::~BaseNodeWidget()
{
    
}


void BaseNodeWidget::Initialize()
{
    m_firstFrame = true;
    
}

void BaseNodeWidget::DrawSocket(const Nw::Pin &pin)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Taille du socket
    float socket_size = 4.0f;
    
    // Définir les 5 points du polygone (flèche pointant vers la droite pour Output)
    // Pour Input, la flèche pointerait vers la gauche
    ImVec2 p1, p2, p3, p4, p5;
    
    if (pin.pinKind == Nw::PinKind::Output) {
        // Flèche pointant vers la droite (→)
        p1 = ImVec2(pin.pinPoint.x - socket_size * 1.5f, pin.pinPoint.y - socket_size);
        p2 = ImVec2(pin.pinPoint.x - socket_size * 0.5f, pin.pinPoint.y - socket_size);
        p3 = ImVec2(pin.pinPoint.x + socket_size * 0.5f, pin.pinPoint.y);  // Pointe
        p4 = ImVec2(pin.pinPoint.x - socket_size * 0.5f, pin.pinPoint.y + socket_size);
        p5 = ImVec2(pin.pinPoint.x - socket_size * 1.5f, pin.pinPoint.y + socket_size);
    } else {
        // Flèche pointant vers la gauche (←)
        p1 = ImVec2(pin.pinPoint.x + socket_size * 1.5f, pin.pinPoint.y - socket_size);
        p2 = ImVec2(pin.pinPoint.x + socket_size * 0.5f, pin.pinPoint.y - socket_size);
        p3 = ImVec2(pin.pinPoint.x - socket_size * 0.5f, pin.pinPoint.y);  // Pointe
        p4 = ImVec2(pin.pinPoint.x + socket_size * 0.5f, pin.pinPoint.y + socket_size);
        p5 = ImVec2(pin.pinPoint.x + socket_size * 1.5f, pin.pinPoint.y + socket_size);
    }
    
    ImVec2 vertices[] = {p1, p2, p3, p4, p5};
    
    // Rectangle pour la détection de hover
    ImVec2 tl = pin.pinPoint - ImVec2(socket_size * 1.5f, socket_size);
    ImVec2 br = pin.pinPoint + ImVec2(socket_size * 1.5f, socket_size);
    
    bool hovered = ImGui::IsItemHovered() || ImGui::IsMouseHoveringRect(tl, br);
    
    // Dessin du socket
    if (pin.isConnected) {
        // Rempli quand connecté
        draw_list->AddConvexPolyFilled(vertices, IM_ARRAYSIZE(vertices), 
                                    pin.style.color);
    } else {
        // Contour seulement quand non connecté
        if (hovered) {
            draw_list->AddPolyline(vertices, IM_ARRAYSIZE(vertices), 
                                pin.style.color, 
                                ImDrawFlags_Closed, 
                                pin.style.socket_hovered_radius);  // Épaisseur au hover
        } else {
            draw_list->AddPolyline(vertices, IM_ARRAYSIZE(vertices), 
                                pin.style.color, 
                                ImDrawFlags_Closed, 
                                pin.style.socket_thickness);  // Épaisseur normale
        }
    }
    
    // Optionnel : dessiner la décoration (fond hover) si nécessaire
    if (hovered) {
        draw_list->AddRectFilled(pin.pos - pin.style.padding, 
                                pin.pos + pin.size + pin.style.padding, 
                                pin.style.bg_hover_color, 
                                pin.style.bg_radius);
    }

}

