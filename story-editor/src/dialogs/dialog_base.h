// dialog_base.h
#ifndef DIALOG_BASE_H
#define DIALOG_BASE_H

#include "imgui.h" // Nécessaire pour ImGui::BeginPopupModal

class DialogBase
{
public:
    virtual ~DialogBase() = default;

    // Affiche le contenu de la fenêtre de dialogue.
    // Cette méthode est virtuelle pure et doit être implémentée par les classes dérivées.
    virtual void Draw() = 0;

    // Ouvre le popup de la fenêtre de dialogue.
    // Note: Ceci ne rend pas le dialogue visible tant que Draw() n'est pas appelé.
    // Cette méthode est utilisée pour les popups modaux ImGui.
    void Open()
    {
        if (m_show)
        {
            ImGui::OpenPopup(GetTitle());
        }
    }

    // Rend le dialogue visible au prochain appel de Draw().
    void Show()
    {
        m_show = true;
        m_firstOpen = true; 
    }

    // Cache le dialogue au prochain appel de Draw().
    void Reset()
    {
        m_show = false;
        m_firstOpen = false;
    }

protected:
    // Permet d'accéder au titre pour l'ouverture du popup.
    virtual const char* GetTitle() const = 0;

    bool m_show = false;
    bool m_firstOpen = false;
};

#endif // DIALOG_BASE_H