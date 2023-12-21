#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include "gui.h"
#include <string>
#include <mutex>
#include <vector>

#include "window_base.h"


// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
struct ConsoleWindow : public WindowBase
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

    void AddLog(const std::string &text, uint32_t type)
    {


        // FIXME-OPT
        Entry e{text, type};
        std::scoped_lock<std::mutex> mutex(mLogMutex);
        Items.push_back(e);
        if (Items.size() > 100)
        {
            Items.erase(Items.begin());
        }
    }

    virtual void Draw() override;


private:

    struct Entry {
        std::string text;
        uint32_t type;
    };

    std::mutex              mLogMutex;
    char                  InputBuf[256];
    std::vector<Entry>       Items;

    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

};

#endif // CONSOLEWINDOW_H
