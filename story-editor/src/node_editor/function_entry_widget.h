#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <set>

#include "base_node_widget.h"
#include "i_story_manager.h"
#include "i_story_project.h"
#include "function_entry_node.h"
#include "gui.h"


void DrawBlueprintSyncSocket(ImDrawList* draw_list, const ImVec2& center, float size, ImU32 color, bool filled = true) {
    const float half_size = size * 0.5f;
    const float triangle_size = size * 0.6f; // Triangle légèrement plus petit que le carré
    
    // Coordonnées du carré (partie gauche)
    ImVec2 square_min = ImVec2(center.x - half_size, center.y - half_size);
    ImVec2 square_max = ImVec2(center.x, center.y + half_size);
    
    // Coordonnées du triangle (partie droite, pointant vers la droite)
    ImVec2 triangle_p1 = ImVec2(center.x, center.y - triangle_size * 0.5f);           // Point haut
    ImVec2 triangle_p2 = ImVec2(center.x, center.y + triangle_size * 0.5f);           // Point bas  
    ImVec2 triangle_p3 = ImVec2(center.x + triangle_size * 0.7f, center.y);          // Point de la pointe
    
    if (filled) {
        // Dessiner le carré rempli
        draw_list->AddRectFilled(square_min, square_max, color);
        
        // Dessiner le triangle rempli
        draw_list->AddTriangleFilled(triangle_p1, triangle_p2, triangle_p3, color);
    } else {
        // Dessiner les contours
        const float thickness = 2.0f;
        
        // Contour du carré
        draw_list->AddRect(square_min, square_max, color, 0.0f, 0, thickness);
        
        // Contour du triangle
        draw_list->AddTriangle(triangle_p1, triangle_p2, triangle_p3, color, thickness);
    }
}

// Version avec dégradé pour un effet plus moderne
void DrawBlueprintSyncSocketGradient(ImDrawList* draw_list, const ImVec2& center, float size, ImU32 color_start, ImU32 color_end) {
    const float half_size = size * 0.5f;
    const float triangle_size = size * 0.6f;
    
    // Coordonnées du carré
    ImVec2 square_min = ImVec2(center.x - half_size, center.y - half_size);
    ImVec2 square_max = ImVec2(center.x, center.y + half_size);
    
    // Coordonnées du triangle
    ImVec2 triangle_p1 = ImVec2(center.x, center.y - triangle_size * 0.5f);
    ImVec2 triangle_p2 = ImVec2(center.x, center.y + triangle_size * 0.5f);
    ImVec2 triangle_p3 = ImVec2(center.x + triangle_size * 0.7f, center.y);
    
    // Carré avec dégradé horizontal
    draw_list->AddRectFilledMultiColor(
        square_min, square_max,
        color_start, color_end,
        color_end, color_start
    );
    
    // Triangle uni (couleur de fin du dégradé)
    draw_list->AddTriangleFilled(triangle_p1, triangle_p2, triangle_p3, color_end);
}

// Variante avec animation de pulsation pour indiquer l'activité
void DrawBlueprintSyncSocketAnimated(ImDrawList* draw_list, const ImVec2& center, float size, ImU32 color, float time) {
    // Effet de pulsation basé sur le temps
    float pulse = 0.8f + 0.2f * sinf(time * 3.0f); // Oscille entre 0.8 et 1.0
    float animated_size = size * pulse;
    
    // Socket principal
    DrawBlueprintSyncSocket(draw_list, center, animated_size, color, true);
    
    // Halo subtil autour
    ImU32 halo_color = ImGui::ColorConvertFloat4ToU32(ImVec4(
        ((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
        ((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f, 
        ((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
        0.3f * (pulse - 0.8f) * 5.0f // Alpha qui varie avec la pulsation
    ));
    
    DrawBlueprintSyncSocket(draw_list, center, size * 1.2f, halo_color, false);
}

// Utilisation dans ImNodeFlow
void DrawSyncSocketInNode(ImDrawList* draw_list, const ImVec2& socket_pos, bool is_connected, bool is_hovered) {
    const float socket_size = 16.0f;
    
    // Couleurs selon l'état
    ImU32 base_color = IM_COL32(100, 150, 255, 255);  // Bleu par défaut
    ImU32 connected_color = IM_COL32(50, 255, 100, 255);  // Vert si connecté
    ImU32 hover_color = IM_COL32(255, 200, 50, 255);      // Orange au survol
    
    ImU32 final_color = base_color;
    if (is_connected) final_color = connected_color;
    if (is_hovered) final_color = hover_color;
    
    // Dessiner le socket
    if (is_connected) {
        // Version animée si connecté
        float time = ImGui::GetTime();
        DrawBlueprintSyncSocketAnimated(draw_list, socket_pos, socket_size, final_color, time);
    } else {
        // Version statique
        DrawBlueprintSyncSocket(draw_list, socket_pos, socket_size, final_color, true);
    }
}



class FunctionEntryWidget : public BaseNodeWidget
{
public:
    FunctionEntryWidget(IStoryManager &manager, std::shared_ptr<BaseNode> node)
        : BaseNodeWidget(manager, node)
        , m_manager(manager)
    {
        m_functionEntryNode = std::dynamic_pointer_cast<FunctionEntryNode>(node);
        SetTitle("Function Entry");
    }

    void Draw() override {
        ImGui::SetNextItemWidth(100.f);
    }


    void DrawSocket(const Nw::Pin &pin) override
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


        /*

        // bonne position du socket
         ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
        // 1. Dessiner le socket à pin.pinPoint (pas à pin.pos !)
        if (pin.isConnected) {
            draw_list->AddCircleFilled(pin.pinPoint, 
                                    pin.style.socket_connected_radius, 
                                    pin.style.color);
        } else {
            // Gérer le hover vous-même si nécessaire
            ImVec2 tl = pin.pinPoint - ImVec2(pin.style.socket_radius, pin.style.socket_radius);
            ImVec2 br = pin.pinPoint + ImVec2(pin.style.socket_radius, pin.style.socket_radius);
            bool hovered = ImGui::IsMouseHoveringRect(tl, br);
            
            draw_list->AddCircle(pin.pinPoint, 
                                hovered ? pin.style.socket_hovered_radius 
                                    : pin.style.socket_radius,
                                pin.style.color,
                                pin.style.socket_shape,
                                pin.style.socket_thickness);
        }

        */

/*

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float socket_size = 4;

        // pin.pos.x = pin.pos.x + 10;

        ImVec2 w_pos = ImGui::GetCursorPos();
        std::cout << "x = " << w_pos.x << ", y = " << w_pos.y << std::endl;;

        // Définir les points du polygone pour le symbole de synchronisation
        // C'est un polygone fermé à 5 points
        ImVec2 p1(pin.pos.x - socket_size * 0.5f, pin.pos.y - socket_size);
        ImVec2 p2(pin.pos.x + socket_size * 0.5f, pin.pos.y - socket_size);
        ImVec2 p3(pin.pos.x + socket_size * 0.5f, pin.pos.y + socket_size);
        ImVec2 p4(pin.pos.x - socket_size * 0.5f, pin.pos.y + socket_size);
        ImVec2 p5(pin.pos.x + socket_size * 1.5f, pin.pos.y);

        ImVec2 vertices[] = {p1, p2, p5, p3, p4}; // Ordre des sommets

        // Pour la détection de survol (hover) on peut toujours utiliser le rectangle englobant
        ImVec2 tl = pin.pos - ImVec2(socket_size * 1.5f, socket_size);
        ImVec2 br = pin.pos + ImVec2(socket_size * 1.5f, socket_size);

        if (pin.isConnected)
        {
            draw_list->AddConvexPolyFilled(vertices, IM_ARRAYSIZE(vertices), IM_COL32(255,255,255,255));
        }
        else
        {
            if (ImGui::IsItemHovered() || ImGui::IsMouseHoveringRect(tl, br))
            {
                draw_list->AddPolyline(vertices, IM_ARRAYSIZE(vertices),IM_COL32(255,255,255,255), ImDrawFlags_Closed, 2.0f);
            }
            else
            {
                draw_list->AddPolyline(vertices, IM_ARRAYSIZE(vertices), IM_COL32(255,255,255,255), ImDrawFlags_Closed, 1.3f);
            }
        }

        */
    }

    virtual bool HasSync() const override { 
        return false;
    }

    virtual void DrawProperties(std::shared_ptr<IStoryProject> story) override {

    }
    virtual void Initialize() override {

    }

private:
    IStoryManager &m_manager;

    std::shared_ptr<FunctionEntryNode> m_functionEntryNode;
 
};
