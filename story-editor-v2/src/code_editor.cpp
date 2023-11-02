#include "code_editor.h"

#include <fstream>

CodeEditor::CodeEditor()
{

}

void CodeEditor::Initialize()
{
    // error markers
    TextEditor::ErrorMarkers markers;
    markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
    markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
    mEditor.SetErrorMarkers(markers);

    // "breakpoint" markers
        //TextEditor::Breakpoints bpts;
        //bpts.insert(24);
        //bpts.insert(47);
        //editor.SetBreakpoints(bpts);

    mFileToEdit = "test/test_zebra7500.js";

    {
        std::ifstream t(mFileToEdit);
        if (t.good())
        {
            std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
            mEditor.SetText(str);
        }
    }
}

void CodeEditor::Draw(const char* title, bool* p_open)
{
    if (!IsVisible())
    {
        return;
    }

    auto cpos = mEditor.GetCursorPosition();
    ImGui::Begin(title, p_open, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
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

    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, mEditor.GetTotalLines(),
        mEditor.IsOverwrite() ? "Ovr" : "Ins",
        mEditor.CanUndo() ? "*" : " ",
        mEditor.GetLanguageDefinition().mName.c_str(), mFileToEdit.c_str());

    mEditor.Render("TextEditor");
    ImGui::End();
}
