#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <vector>
#include <chrono>

#include "IconsMaterialDesignIcons.h"
#include "IconsFontAwesome5_c.h"

enum ToastType {
    Success,
    Warning,
    Error,
    Info
};

struct Toast {
    std::string title;
    std::string text;
    ToastType type;
    std::chrono::steady_clock::time_point startTime;
    float duration;
};

class ImGuiToastNotifier {
private:
    std::vector<Toast> toasts;
    const float TOAST_WIDTH = 350.0f;
    const float TOAST_PADDING = 10.0f;

public:
    void addToast(const std::string& title, const std::string& text, ToastType type, float duration = 3.0f) {
        toasts.push_back({title, text, type, std::chrono::steady_clock::now(), duration});
    }

    // Helper methods pour les cas communs
    void success(const std::string& message) {
        addToast("Success", message, ToastType::Success, 3.0f);
    }
    
    void error(const std::string& message, float duration = 5.0f) {
        addToast("Error", message, ToastType::Error, duration);
    }
    
    void warning(const std::string& message) {
        addToast("Warning", message, ToastType::Warning, 4.0f);
    }
    
    void info(const std::string& message) {
        addToast("Info", message, ToastType::Info, 3.0f);
    }
    
    // Pour les erreurs de compilation
    void compilationFailed(size_t errorCount, size_t warningCount = 0) {
        std::string msg = std::to_string(errorCount) + " error(s) found";
        if (warningCount > 0) {
            msg += ", " + std::to_string(warningCount) + " warning(s)";
        }
        msg += ". Check Error List for details.";
        addToast("Compilation Failed", msg, ToastType::Error, 6.0f);
    }
    
    void compilationSuccess(float duration = 2.5f) {
        addToast("Compilation Success", "Build completed successfully", ToastType::Success, duration);
    }

    void render() {
        if (toasts.empty()) {
            return;
        }

        // Get the main viewport work area
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;
        
        // Position in top-right corner of the work area
        ImVec2 window_pos = ImVec2(
            work_pos.x + work_size.x - TOAST_PADDING,
            work_pos.y + TOAST_PADDING
        );
        
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(TOAST_WIDTH, 0), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.90f);
        
        ImGuiWindowFlags window_flags = 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_AlwaysAutoResize;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        
        if (ImGui::Begin("##ToastWindow", nullptr, window_flags)) {
            auto now = std::chrono::steady_clock::now();
            auto it = toasts.begin();
            
            while (it != toasts.end()) {
                auto elapsed = std::chrono::duration<float>(now - it->startTime).count();
                
                if (elapsed > it->duration) {
                    it = toasts.erase(it);
                    continue;
                }

                // Calculate fade out effect in the last 0.5 seconds
                float alpha = 1.0f;
                if (elapsed > it->duration - 0.5f) {
                    alpha = (it->duration - elapsed) / 0.5f;
                }

                ImGui::PushID(it->title.c_str());
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

                ImVec4 color;
                const char* icon;
                switch (it->type) {
                    case Success:
                        color = ImVec4(0.18f, 0.80f, 0.44f, 1.0f);
                        icon = ICON_FA_CHECK_CIRCLE;
                        break;
                    case Warning:
                        color = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                        icon = ICON_FA_EXCLAMATION_TRIANGLE;
                        break;
                    case Error:
                        color = ImVec4(0.94f, 0.31f, 0.31f, 1.0f);
                        icon = ICON_FA_TIMES_CIRCLE;
                        break;
                    case Info:
                        color = ImVec4(0.3f, 0.7f, 1.0f, 1.0f);
                        icon = ICON_FA_INFO_CIRCLE;
                        break;
                }

                // Draw icon and title on the same line
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::Text("%s", icon);
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
                ImGui::Text("%s", it->title.c_str());
                ImGui::PopFont();

                // Draw message text with wrapping
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + TOAST_WIDTH - 24.0f);
                ImGui::TextWrapped("%s", it->text.c_str());
                ImGui::PopTextWrapPos();

                ImGui::PopStyleVar(); // Alpha

                // Add separator between toasts if not the last one
                if (std::next(it) != toasts.end()) {
                    ImGui::Separator();
                }
                
                ImGui::PopID();
                ++it;
            }

            ImGui::End();
        }
        
        ImGui::PopStyleVar(3); // WindowPadding, ItemSpacing, WindowRounding
    }
};