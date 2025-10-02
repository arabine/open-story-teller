#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <filesystem>
#include <algorithm>
#include "json.hpp"
#include "sys_lib.h"

using json = nlohmann::json;

struct LanguageInfo {
    std::string code;        // "en", "fr", etc.
    std::string displayName; // "English", "Français", etc.
    std::string flagEmoji;   // "🇬🇧", "🇫🇷", etc. (optionnel)
};

class Localization {
public:
    static Localization& Instance() {
        static Localization instance;
        return instance;
    }

    // Scanner le dossier locales/ pour détecter les langues disponibles
    void ScanAvailableLanguages() {
        m_availableLanguages.clear();
        
        std::string localesPath = "locales";
        
        if (!std::filesystem::exists(localesPath)) {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(localesPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string langCode = entry.path().stem().string();
                
                try {
                    std::string content = SysLib::ReadFile(entry.path().string());
                    json j = json::parse(content);
                    
                    LanguageInfo info;
                    info.code = langCode;
                    
                    // Lire le nom d'affichage depuis le fichier de langue
                    if (j.contains("language.name")) {
                        info.displayName = j["language.name"].get<std::string>();
                    } else {
                        info.displayName = langCode; // Fallback sur le code
                    }
                    
                    // Lire l'emoji de drapeau (optionnel)
                    if (j.contains("language.flag")) {
                        info.flagEmoji = j["language.flag"].get<std::string>();
                    }
                    
                    m_availableLanguages.push_back(info);
                }
                catch (const std::exception& e) {
                    // Ignorer les fichiers JSON invalides
                    continue;
                }
            }
        }
        
        // Trier par code de langue
        std::sort(m_availableLanguages.begin(), m_availableLanguages.end(),
            [](const LanguageInfo& a, const LanguageInfo& b) {
                return a.code < b.code;
            });
    }
    
    const std::vector<LanguageInfo>& GetAvailableLanguages() const {
        return m_availableLanguages;
    }

    // Charger un fichier de langue
    bool LoadLanguage(const std::string& langCode) {
        std::string filePath = "locales/" + langCode + ".json";
        
        if (!SysLib::FileExists(filePath)) {
            return false;
        }

        try {
            std::string content = SysLib::ReadFile(filePath);
            json j = json::parse(content);
            
            m_translations.clear();
            for (auto& [key, value] : j.items()) {
                m_translations[key] = value.get<std::string>();
            }
            
            m_currentLang = langCode;
            return true;
        }
        catch (const std::exception& e) {
            return false;
        }
    }

    // Obtenir une traduction
    const char* Get(const std::string& key) const {
        auto it = m_translations.find(key);
        if (it != m_translations.end()) {
            return it->second.c_str();
        }
        // Retourne la clé si non trouvée (utile pour le debug)
        return key.c_str();
    }

    // Obtenir avec des paramètres de format
    std::string GetF(const std::string& key, const std::vector<std::string>& args) const {
        std::string text = Get(key);
        
        for (size_t i = 0; i < args.size(); ++i) {
            std::string placeholder = "{" + std::to_string(i) + "}";
            size_t pos = text.find(placeholder);
            if (pos != std::string::npos) {
                text.replace(pos, placeholder.length(), args[i]);
            }
        }
        
        return text;
    }

    const std::string& GetCurrentLang() const {
        return m_currentLang;
    }

private:
    Localization() : m_currentLang("en") {
        // Scanner les langues disponibles au démarrage
        ScanAvailableLanguages();
        // Charger la langue par défaut
        LoadLanguage("en");
    }

    // Empêcher la copie
    Localization(const Localization&) = delete;
    Localization& operator=(const Localization&) = delete;

    std::unordered_map<std::string, std::string> m_translations;
    std::string m_currentLang;
    std::vector<LanguageInfo> m_availableLanguages;
};

// Macro pour simplifier l'usage
#define TR(key) Localization::Instance().Get(key)
#define TRF(key, ...) Localization::Instance().GetF(key, {__VA_ARGS__})

#endif // LOCALIZATION_H