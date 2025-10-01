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
    Error
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
    const float TOAST_WIDTH = 300.0f;
    const float TOAST_PADDING = 10.0f;

public:
    void addToast(const std::string& title, const std::string& text, ToastType type, float duration = 3.0f) {
        toasts.push_back({title, text, type, std::chrono::steady_clock::now(), duration});
    }

    void render() {
        if (toasts.empty()) {
            return;
        }

        // Get the main viewport work area (excludes menu bar, status bar, etc.)
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
        
        // Add NoNav to prevent interfering with other windows
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
                }

                // Draw icon and title on the same line
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::Text("%s", icon);
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font for title
                ImGui::TextUnformatted(it->title.c_str());
                ImGui::PopFont();

                // Draw message text with wrapping
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + TOAST_WIDTH - 24.0f);
                ImGui::TextUnformatted(it->text.c_str());
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