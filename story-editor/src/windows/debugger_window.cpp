#include "debugger_window.h"
#include "IconsMaterialDesignIcons.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include <gui.h>
#include "imgui.h"


void DebuggerWindow::TextViewDraw()
{
    const ImVec2 childSize = ImVec2(0, 0);
    // Début de la scrollview (Child)
    ImGui::BeginChild("Table ScrollView", childSize, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit
     | ImGuiTableFlags_BordersV 
     | ImGuiTableFlags_BordersOuterH 
     | ImGuiTableFlags_RowBg 
     | ImGuiTableFlags_ContextMenuInBody;
        
    static ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));


    // Hauteur de chaque ligne
    float lineHeight = ImGui::GetTextLineHeightWithSpacing(); // Hauteur d'une ligne avec l'espacement

    if (ImGui::BeginTable("AssemblyCode", 3, tableFlags))
    {            
        int i = 1;
        std::string line;
        std::istringstream is(m_text);

        if ((m_focusOnLine != -1) && (m_focusOnLine != m_currentLine))
        {
            m_currentLine = m_focusOnLine;
            // Calcul de la position Y pour centrer la ligne cible
            float scrollY = m_currentLine * lineHeight - ImGui::GetWindowHeight() * 0.5f + lineHeight * 0.5f;
            // Début de la scrollview
            ImGui::SetScrollY(scrollY);
        }

        while (std::getline(is, line)) 
        {
            ImGui::TableNextRow();
            ImGui::PushID(static_cast<int>(i)); // Identifiant unique pour chaque ligne

            ImGui::TableNextColumn();

            // Colonne des breakpoints            
            if (ImGui::Selectable("", m_breakpoints.contains(i), 0, ImVec2(20, 20))) {
                if (m_breakpoints.contains(i))
                {
                    m_breakpoints.erase(i);
                }
                else
                {
                    m_breakpoints.insert(i);
                }
                m_storyManager.ToggleBreakpoint(i);
            }
            
            if (m_breakpoints.contains(i)) {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.85f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
            ImGui::TableNextColumn();

            if (m_currentLine == i)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }

            // Colonne des numéros de ligne
            ImGui::Text("%d", static_cast<int>(i));
            ImGui::TableNextColumn();

            if (m_currentLine == i)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }

            ImGui::TextUnformatted(line.c_str());
            ImGui::PopID();
            i++;
        }
        ImGui::EndTable();
    }

   ImGui::EndChild();
}


DebuggerWindow::DebuggerWindow(IStoryManager &project)
    : WindowBase("Code viewer")
    , m_storyManager(project)
{
    // mEditor.SetReadOnly(false);
    // SetFlags(ImGuiWindowFlags_MenuBar);
}

void DebuggerWindow::Initialize()
{
    // error markers

//
//    markers.insert(std::make_pair<int, std::string>(41, "Another example error"));


    // "breakpoint" markers
    // m_breakPoints.insert(42);
    // mEditor.SetBreakpoints(m_breakPoints);

}

void DebuggerWindow::ClearErrors()
{
    // m_markers.clear();
}

void DebuggerWindow::AddError(int line, const std::string &text)
{
    // m_markers.insert(std::make_pair(line, text));
    // mEditor.SetErrorMarkers(m_markers);
}


void DebuggerWindow::SetScript(const std::string &txt)
{
    m_text = txt;

//         m_text = R"(
//     fdsfds
//     fds
// ffffffffffffffffffffff
//     ff
//     fd
// feeeee
//     21234f
// e
//     fdsfs
//     )";

//     HighlightLine(2);
}

std::string DebuggerWindow::GetScript() const
{
    // return mEditor.GetText();
    return "";
}


void DebuggerWindow::Draw()
{
    WindowBase::BeginDraw();


    // auto cpos = mEditor.GetCursorPosition();

    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);


    // if (ImGui::SmallButton("Toggle breakpoint")) {
        
    //     if (m_breakPoints.contains(cpos.mLine + 1))
    //     {
    //         m_breakPoints.erase(cpos.mLine + 1);
    //     }
    //     else
    //     {
    //         m_breakPoints.insert(cpos.mLine + 1);
    //     }
    //     mEditor.SetBreakpoints(m_breakPoints);

    //     m_storyManager.ToggleBreakpoint(cpos.mLine + 1);
        
    // }
    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_MDI_SKIP_NEXT "##step_instruction")) {
        m_storyManager.Step();
    }

    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_MDI_PLAY "##run")) {
        m_storyManager.Run();
    }

    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_MDI_CONTENT_COPY "##copytocplipboard")) {
      Gui::CopyToClipboard(m_text);
    }

    TextViewDraw();

    // mEditor.Render("TextEditor");
    WindowBase::EndDraw();
}
