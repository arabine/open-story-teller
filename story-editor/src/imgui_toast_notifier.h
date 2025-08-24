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

public:
    void addToast(const std::string& title, const std::string& text, ToastType type, float duration = 3.0f) {
        toasts.push_back({title, text, type, std::chrono::steady_clock::now(), duration});
    }

    void render() {
        if (toasts.empty()) {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 viewport_pos = ImGui::GetMainViewport()->Pos;
        ImVec2 viewport_size = ImGui::GetMainViewport()->Size;
        ImVec2 window_pos = ImVec2(viewport_pos.x + viewport_size.x - 10.0f, viewport_pos.y + 10.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        ImGui::SetNextWindowBgAlpha(0.75f);
        if (ImGui::Begin("##ToastWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing)) {
            auto now = std::chrono::steady_clock::now();
            auto it = toasts.begin();
            while (it != toasts.end()) {
                auto elapsed = std::chrono::duration<float>(now - it->startTime).count();
                if (elapsed > it->duration) {
                    it = toasts.erase(it);
                    continue;
                }

                ImGui::PushID(it->title.c_str());

                ImVec4 color;
                const char* icon;
                switch (it->type) {
                    case Success:
                        color = ImVec4(0.18f, 0.80f, 0.44f, 1.0f);
                        icon = ICON_FA_CHECK_CIRCLE; // Font Awesome 5 success icon (check-circle)
                        break;
                    case Warning:
                        color = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
                        icon = ICON_FA_EXCLAMATION_TRIANGLE; // Font Awesome 5 warning icon (exclamation-triangle)
                        break;
                    case Error:
                        color = ImVec4(0.94f, 0.31f, 0.31f, 1.0f);
                        icon = ICON_FA_TIMES_CIRCLE; // Font Awesome 5 error icon (times-circle)
                        break;
                }

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::Text("%s", icon);
                ImGui::PopStyleColor();

                ImGui::SameLine();
                ImGui::TextUnformatted(it->title.c_str());
                ImGui::TextWrapped("%s", it->text.c_str());

                ImGui::Separator();
                ImGui::PopID();
                ++it;
            }

            ImGui::End();
        }
    }
};