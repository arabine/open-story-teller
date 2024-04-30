#ifndef WINDOWBASE_H
#define WINDOWBASE_H

#include <string>

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

    void Enable() {
        m_disabled = false;
    }

private:

    bool m_disabled{false};
    std::string m_title;
};

#endif // WINDOWBASE_H
