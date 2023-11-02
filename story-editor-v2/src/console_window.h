#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include "gui.h"
#include <string>
#include <mutex>
#include <vector>


// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
struct ConsoleWindow
{
public:
    ConsoleWindow();
    ~ConsoleWindow();

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog();

    void AddMessage(const std::string &message) { AddLog("%s", message.c_str()); }

    void Draw(const char* title, bool* p_open);

    void    ExecCommand(const char* command_line);

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

    int     TextEditCallback(ImGuiInputTextCallbackData* data);

private:

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf)-1] = 0;
        va_end(args);
        std::scoped_lock<std::mutex> mutex(mLogMutex);
        Items.push_back(Strdup(buf));
        if (Items.size() > 100) 
        {
            Items.erase(Items.begin());
        }
    }

    std::mutex              mLogMutex;
    char                  InputBuf[256];
    std::vector<std::string>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

};

#endif // CONSOLEWINDOW_H
