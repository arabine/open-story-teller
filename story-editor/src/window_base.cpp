#include "window_base.h"
#include "imgui.h"

WindowBase::WindowBase(const std::string &title)
    : m_title(title)
{

}

bool WindowBase::BeginDraw()
{
    bool ok  = ImGui::Begin(m_title.c_str(), nullptr, m_windowFlags);

    if (m_disabled)
    {
        ImGui::BeginDisabled();
    }
    return ok;
}

void WindowBase::EndDraw()
{
    if (m_disabled)
    {
        ImGui::EndDisabled();
    }
    ImGui::End();

}

