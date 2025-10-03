#include "console_window.h"
#include <stdio.h>

ConsoleWindow::ConsoleWindow()
    : WindowBase("Console")
{
    ClearLog();
    memset(InputBuf, 0, sizeof(InputBuf));

    AutoScroll = true;
    ScrollToBottom = false;
}

ConsoleWindow::~ConsoleWindow()
{
    ClearLog();
}

void ConsoleWindow::ClearLog()
{
    std::scoped_lock<std::mutex> mutex(mLogMutex);
    Items.clear();
}

void ConsoleWindow::Draw()
{
    WindowBase::BeginDraw();

    ImGui::SetWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

    ImGui::TextWrapped("Console view");
    
    if (ImGui::SmallButton("Clear")) { 
        ClearLog(); 
    }
    ImGui::SameLine();
    
    // Option pour activer/désactiver l'auto-scroll
    ImGui::Checkbox("Auto-scroll", &AutoScroll);
    ImGui::SameLine();
    
    bool copy_to_clipboard = ImGui::SmallButton("Copy");
    
    ImGui::Separator();

    // Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Selectable("Clear")) ClearLog();
        ImGui::EndPopup();
    }

    // Afficher les logs avec thread-safety
    {
        std::scoped_lock<std::mutex> mutex(mLogMutex);
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        
        for (const auto& entry : Items)
        {
            const char* item = entry.text.c_str();
            
            // Coloration selon le type
            ImVec4 color;
            bool has_color = false;
            
            if (entry.type == 1) { // Error
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); 
                has_color = true; 
            }
            else if (strstr(item, "[error]")) { 
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); 
                has_color = true; 
            }
            else if (strncmp(item, "# ", 2) == 0) { 
                color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); 
                has_color = true; 
            }
            
            if (has_color)
                ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item);
            if (has_color)
                ImGui::PopStyleColor();
        }
        
        if (copy_to_clipboard)
            ImGui::LogFinish();
        
        ImGui::PopStyleVar();
    }
        
    // Solution simple : ne forcer le scroll QUE quand on ajoute un nouveau message
    // et que l'utilisateur était déjà en bas (ou si ScrollToBottom est explicitement demandé)
    
    float scrollY = ImGui::GetScrollY();
    float scrollMaxY = ImGui::GetScrollMaxY();
    
    // On considère qu'on est "en bas" avec une petite tolérance
    const float BOTTOM_THRESHOLD = 5.0f;
    bool wasAtBottom = (scrollMaxY == 0) || (scrollY >= scrollMaxY - BOTTOM_THRESHOLD);
    
    // Seulement scroller si :
    // 1. ScrollToBottom est explicitement demandé (nouveau message ajouté), OU
    // 2. AutoScroll est activé ET on était déjà en bas
    if (ScrollToBottom || (AutoScroll && wasAtBottom))
    {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ScrollToBottom = false;

    ImGui::EndChild();
    ImGui::Separator();

    WindowBase::EndDraw();
}