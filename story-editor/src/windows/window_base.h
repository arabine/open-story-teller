#ifndef WINDOWBASE_H
#define WINDOWBASE_H

#include <string>
#include "imgui.h"

class WindowBase
{
public:
    WindowBase(const std::string &title);
    virtual ~WindowBase() {}

    bool IsDisabled() const {
        return m_disabled;
    }

    virtual void Draw() = 0;

    bool BeginDraw();
    void EndDraw();

    void Disable() {
        m_disabled = true;
    }

    bool IsFocused() const {
        return m_focused;
    }

    void Enable() {
        m_disabled = false;
    }

    void SetFlags(ImGuiWindowFlags flags) {
        m_windowFlags = flags;
    }

private:

    bool m_disabled{false};
    bool m_focused{false};
    std::string m_title;
    ImGuiWindowFlags m_windowFlags = 0;
};

#endif // WINDOWBASE_H
