#ifndef WINDOW_BASE_H
#define WINDOW_BASE_H

class WindowBase
{
public:
    WindowBase(bool display = false)
    {
        mDisplay = display;
    }

    void SetVisible(bool enable)
    {
        mDisplay = enable;
    }

    bool IsVisible() const {
        return mDisplay;
    }
private:
    bool mDisplay;
};

#endif // WINDOW_BASE_H

