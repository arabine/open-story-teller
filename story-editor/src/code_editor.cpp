#include "code_editor.h"
#include "IconsMaterialDesignIcons.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include "imgui.h"


void CodeEditor::TextViewDraw()
{

    ImGui::BeginChild("TextView", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    static ImGuiTableFlags flags1 = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
    static ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));

    if (ImGui::BeginTable("AssemblyCode", 3, flags1))
    {            
        int i = 0;
        std::string line;
        std::istringstream is(m_text);
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
            }
            
            if (m_breakpoints.contains(i)) {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.85f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
            ImGui::TableNextColumn();

            if (m_highlights.count(i) > 0)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }

            // Colonne des numéros de ligne
            ImGui::Text("%d", static_cast<int>(i + 1));
            ImGui::TableNextColumn();

            if (m_highlights.count(i) > 0)
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


CodeEditor::CodeEditor(IStoryManager &project)
    : WindowBase("Code viewer")
    , m_storyManager(project)
{
    // mEditor.SetReadOnly(false);
    SetFlags(ImGuiWindowFlags_MenuBar);
}

void CodeEditor::Initialize()
{
    // error markers

//
//    markers.insert(std::make_pair<int, std::string>(41, "Another example error"));


    // "breakpoint" markers
    // m_breakPoints.insert(42);
    // mEditor.SetBreakpoints(m_breakPoints);

}

void CodeEditor::ClearErrors()
{
    // m_markers.clear();
}

void CodeEditor::AddError(int line, const std::string &text)
{
    // m_markers.insert(std::make_pair(line, text));
    // mEditor.SetErrorMarkers(m_markers);
}


void CodeEditor::SetScript(const std::string &txt)
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

std::string CodeEditor::GetScript() const
{
    // return mEditor.GetText();
    return "";
}


void CodeEditor::Draw()
{
    WindowBase::BeginDraw();


    // auto cpos = mEditor.GetCursorPosition();

    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    /*
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                // auto textToSave = mEditor.GetText();
                /// save text....
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            // bool ro = mEditor.IsReadOnly();
            // if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
            //     mEditor.SetReadOnly(ro);
            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && mEditor.CanUndo()))
                mEditor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && mEditor.CanRedo()))
                mEditor.Redo();

            ImGui::Separator();

            if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, mEditor.HasSelection()))
                mEditor.Copy();
            if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && mEditor.HasSelection()))
                mEditor.Cut();
            if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && mEditor.HasSelection()))
                mEditor.Delete();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                mEditor.Paste();
                

            ImGui::Separator();

            // if (ImGui::MenuItem("Select all", nullptr, nullptr))
            //     mEditor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(mEditor.GetTotalLines(), 0));

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
                mEditor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                mEditor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Retro blue palette"))
                mEditor.SetPalette(TextEditor::GetRetroBluePalette());
            ImGui::EndMenu();
        }
    
        ImGui::EndMenuBar();
    }
    */

    // ImGui::Text("%6d/%-6d %6d lines  | %s | %s ", cpos.mLine + 1, cpos.mColumn + 1, mEditor.GetTotalLines(),
    //     mEditor.IsOverwrite() ? "Ovr" : "Ins",
    //     mEditor.CanUndo() ? "*" : " ");

    ImGui::SameLine();
    if (ImGui::SmallButton("Toggle breakpoint")) {
        /*
        if (m_breakPoints.contains(cpos.mLine + 1))
        {
            m_breakPoints.erase(cpos.mLine + 1);
        }
        else
        {
            m_breakPoints.insert(cpos.mLine + 1);
        }
        mEditor.SetBreakpoints(m_breakPoints);

        m_storyManager.ToggleBreakpoint(cpos.mLine + 1);
        */
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_MDI_SKIP_NEXT "##step_instruction")) {
        m_storyManager.Step();
    }

    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_MDI_PLAY "##run")) {
        m_storyManager.Run();
    }

    TextViewDraw();

    // mEditor.Render("TextEditor");
    WindowBase::EndDraw();
}
