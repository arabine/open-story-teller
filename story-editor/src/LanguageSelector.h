#ifndef LANGUAGE_SELECTOR_H
#define LANGUAGE_SELECTOR_H

#include <functional>
#include <string>
#include "imgui.h"
#include "Localization.h"

class LanguageSelector {
public:
    LanguageSelector() = default;
    ~LanguageSelector() = default;
    
    // Callback appelé quand la langue change
    using OnLanguageChangedCallback = std::function<void(const std::string& langCode)>;
    
    void SetOnLanguageChanged(OnLanguageChangedCallback callback) {
        m_onLanguageChanged = callback;
    }
    
    // Dessiner le menu de sélection de langue
    void DrawMenu() {
        // Nom du menu qui se met à jour avec la langue
        const char* menuLabel = TR("menu.language");
        
        if (ImGui::BeginMenu(menuLabel)) {
            DrawLanguageOptions();
            ImGui::EndMenu();
        }
    }
    
private:
    void DrawLanguageOptions() {
        auto& localization = Localization::Instance();
        const auto& languages = localization.GetAvailableLanguages();
        const std::string& currentLang = localization.GetCurrentLang();
        
        if (languages.empty()) {
            ImGui::TextDisabled("No languages available");
            return;
        }
        
        for (const auto& lang : languages) {
            // Vérifier si c'est la langue actuelle
            bool isSelected = (lang.code == currentLang);
            
            // Construire le label avec emoji si disponible
            std::string label;
            if (!lang.flagEmoji.empty()) {
                // FIXME: ImGui ne supporte pas en natif les emojis ; chercher sur le net
                // label = lang.flagEmoji + " " + lang.displayName;
                label = lang.displayName;
            } else {
                label = lang.displayName;
            }
            
            // MenuItem avec état de sélection (affiche une coche)
            if (ImGui::MenuItem(label.c_str(), nullptr, isSelected)) {
                if (!isSelected) { // Seulement si ce n'est pas déjà la langue active
                    if (localization.LoadLanguage(lang.code)) {
                        // Notifier le changement
                        if (m_onLanguageChanged) {
                            m_onLanguageChanged(lang.code);
                        }
                    }
                }
            }
        }
    }
    
    OnLanguageChangedCallback m_onLanguageChanged;
};

#endif // LANGUAGE_SELECTOR_H