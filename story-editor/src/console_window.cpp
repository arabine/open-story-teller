#include "console_window.h"

#include <stdio.h>


ConsoleWindow::ConsoleWindow()
    : WindowBase("Console")
{
    ClearLog();
    memset(InputBuf, 0, sizeof(InputBuf));
    HistoryPos = -1;

    // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
    Commands.push_back("HELP");
    Commands.push_back("HISTORY");
    Commands.push_back("CLEAR");
    Commands.push_back("CLASSIFY");
    AutoScroll = true;
    ScrollToBottom = false;
}

ConsoleWindow::~ConsoleWindow()
{
    ClearLog();
    for (int i = 0; i < History.Size; i++)
        free(History[i]);
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
//    ImGui::TextWrapped("Enter 'HELP' for help.");

    // TODO: display items starting from the bottom

//    if (ImGui::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
//    ImGui::SameLine();
//    if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
//    ImGui::SameLine();
    if (ImGui::SmallButton("Clear"))           { ClearLog(); }
    ImGui::SameLine();
    bool copy_to_clipboard = ImGui::SmallButton("Copy");
    //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

//    ImGui::Separator();

//    // Options menu
//    if (ImGui::BeginPopup("Options"))
//    {
//        ImGui::Checkbox("Auto-scroll", &AutoScroll);
//        ImGui::EndPopup();
//    }

//    // Options, Filter
//    if (ImGui::Button("Options"))
//        ImGui::OpenPopup("Options");
//    ImGui::SameLine();
//    Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    ImGui::Separator();

    // Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Selectable("Clear")) ClearLog();
        ImGui::EndPopup();
    }

    // Display every line as a separate entry so we can change their color or add custom widgets.
    // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
    // to only process visible items. The clipper will automatically measure the height of your first item and then
    // "seek" to display only items in the visible area.
    // To use the clipper we can replace your standard loop:
    //      for (int i = 0; i < Items.Size; i++)
    //   With:
    //      ImGuiListClipper clipper;
    //      clipper.Begin(Items.Size);
    //      while (clipper.Step())
    //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    // - That your items are evenly spaced (same height)
    // - That you have cheap random access to your elements (you can access them given their index,
    //   without processing all the ones before)
    // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
    // We would need random-access on the post-filtered list.
    // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
    // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
    // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
    // to improve this example code!
    // If your items are of variable height:
    // - Split them into same height items would be simpler and facilitate random-seeking into your list.
    // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
     if (copy_to_clipboard)
         ImGui::LogToClipboard();

    ImGuiListClipper clipper;
    std::scoped_lock<std::mutex> mutex(mLogMutex);
    clipper.Begin(Items.size());
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            ImGui::TextUnformatted(Items[i].c_str());
        }
    }
            /*
    for (int i = 0; i < Items.Size; i++)
    {
        const char* item = Items[i];
        if (!Filter.PassFilter(item))
            continue;

        // Normally you would store more information in your item than just a string.
        // (e.g. make Items[] an array of structure, store color/type etc.)
        ImVec4 color;
        bool has_color = false;
        if (strstr(item, "[error]"))          { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
        else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
        if (has_color)
            ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(item);
        if (has_color)
            ImGui::PopStyleColor();
    }
    */
     if (copy_to_clipboard)
         ImGui::LogFinish();

    if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
        ImGui::SetScrollHereY(1.0f);
    ScrollToBottom = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();
/*
    // Command-line
    bool reclaim_focus = false;
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
    if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
    {
        char* s = InputBuf;
        Strtrim(s);
        if (s[0])
            ExecCommand(s);
        strcpy(s, "");
        reclaim_focus = true;
    }

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
    if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
*/
   WindowBase::EndDraw();
}

void ConsoleWindow::ExecCommand(const char *command_line)
{
    AddLog("# %s\n", command_line);

    // Insert into history. First find match and delete it so it can be pushed to the back.
    // This isn't trying to be smart or optimal.
    HistoryPos = -1;
    for (int i = History.Size - 1; i >= 0; i--)
        if (Stricmp(History[i], command_line) == 0)
        {
            free(History[i]);
            History.erase(History.begin() + i);
            break;
        }
    History.push_back(Strdup(command_line));

    // Process command
    if (Stricmp(command_line, "CLEAR") == 0)
    {
        ClearLog();
    }
    else if (Stricmp(command_line, "HELP") == 0)
    {
        AddLog("Commands:");
        for (int i = 0; i < Commands.Size; i++)
            AddLog("- %s", Commands[i]);
    }
    else if (Stricmp(command_line, "HISTORY") == 0)
    {
        int first = History.Size - 10;
        for (int i = first > 0 ? first : 0; i < History.Size; i++)
            AddLog("%3d: %s\n", i, History[i]);
    }
    else
    {
        AddLog("Unknown command: '%s'\n", command_line);
    }

    // On command input, we scroll to bottom even if AutoScroll==false
    ScrollToBottom = true;
}

int ConsoleWindow::TextEditCallbackStub(ImGuiInputTextCallbackData *data)
{
    ConsoleWindow* console = (ConsoleWindow*)data->UserData;
    return console->TextEditCallback(data);
}

int ConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData *data)
{
    //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
    {
        // Example of TEXT COMPLETION

        // Locate beginning of current word
        const char* word_end = data->Buf + data->CursorPos;
        const char* word_start = word_end;
        while (word_start > data->Buf)
        {
            const char c = word_start[-1];
            if (c == ' ' || c == '\t' || c == ',' || c == ';')
                break;
            word_start--;
        }

        // Build a list of candidates
        ImVector<const char*> candidates;
        for (int i = 0; i < Commands.Size; i++)
            if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                candidates.push_back(Commands[i]);

        if (candidates.Size == 0)
        {
            // No match
            AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
        }
        else if (candidates.Size == 1)
        {
            // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, candidates[0]);
            data->InsertChars(data->CursorPos, " ");
        }
        else
        {
            // Multiple matches. Complete as much as we can..
            // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
            int match_len = (int)(word_end - word_start);
            for (;;)
            {
                int c = 0;
                bool all_candidates_matches = true;
                for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                    if (i == 0)
                        c = toupper(candidates[i][match_len]);
                    else if (c == 0 || c != toupper(candidates[i][match_len]))
                        all_candidates_matches = false;
                if (!all_candidates_matches)
                    break;
                match_len++;
            }

            if (match_len > 0)
            {
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
            }

            // List matches
            AddLog("Possible matches:\n");
            for (int i = 0; i < candidates.Size; i++)
                AddLog("- %s\n", candidates[i]);
        }

        break;
    }
    case ImGuiInputTextFlags_CallbackHistory:
    {
        // Example of HISTORY
        const int prev_history_pos = HistoryPos;
        if (data->EventKey == ImGuiKey_UpArrow)
        {
            if (HistoryPos == -1)
                HistoryPos = History.Size - 1;
            else if (HistoryPos > 0)
                HistoryPos--;
        }
        else if (data->EventKey == ImGuiKey_DownArrow)
        {
            if (HistoryPos != -1)
                if (++HistoryPos >= History.Size)
                    HistoryPos = -1;
        }

        // A better implementation would preserve the data on the current input line along with cursor position.
        if (prev_history_pos != HistoryPos)
        {
            const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, history_str);
        }
    }
    }
    return 0;
}
