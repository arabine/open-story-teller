#ifndef WINDOWBASE_H
#define WINDOWBASE_H

#include <string>

class WindowBase
{
public:
    WindowBase(const std::string &title);

    bool IsDisabled() const {
        return m_disabled;
    }

    virtual void Draw() = 0;

    bool BeginDraw();
    void EndDraw();

private:

    bool m_disabled{false};
    std::string m_title;
};

#endif // WINDOWBASE_H
