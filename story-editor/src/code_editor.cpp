#include "code_editor.h"

#include <fstream>
#include <memory>

CodeEditor::CodeEditor()
    : WindowBase("Code editor")
{

}

void CodeEditor::Initialize()
{
    // error markers

//
//    markers.insert(std::make_pair<int, std::string>(41, "Another example error"));


    // "breakpoint" markers
   // m_breakPoints.insert(42);
  //  mEditor.SetBreakpoints(m_breakPoints);

}

void CodeEditor::ClearErrors()
{
    m_markers.clear();
}

void CodeEditor::AddError(int line, const std::string &text)
{
    m_markers.insert(std::make_pair(line, text));
    mEditor.SetErrorMarkers(m_markers);
}


void CodeEditor::SetScript(const std::string &txt)
{
    mEditor.SetText(txt);
}

void CodeEditor::Draw()
{
    WindowBase::BeginDraw();


    auto cpos = mEditor.GetCursorPosition();



    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                auto textToSave = mEditor.GetText();
                /// save text....
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = mEditor.IsReadOnly();
            if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                mEditor.SetReadOnly(ro);
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

            if (ImGui::MenuItem("Select all", nullptr, nullptr))
                mEditor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(mEditor.GetTotalLines(), 0));

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

    ImGui::Text("%6d/%-6d %6d lines  | %s | %s ", cpos.mLine + 1, cpos.mColumn + 1, mEditor.GetTotalLines(),
        mEditor.IsOverwrite() ? "Ovr" : "Ins",
        mEditor.CanUndo() ? "*" : " ");

    mEditor.Render("TextEditor");
    WindowBase::EndDraw();
}
