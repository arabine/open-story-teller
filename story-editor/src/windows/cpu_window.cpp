#include "cpu_window.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include "IconsMaterialDesignIcons.h"
#include "chip32_vm.h"

static const char* CpuRegNames[] = {
    "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
    "R8", "R9", "T0", "T1", "T2", "T3", "T4", "T5",
    "T6", "T7", "T8", "T9", "PC", "SP", "RA"
};

CpuWindow::CpuWindow(IStoryManager &proj)
    : WindowBase("CPU")
    , m_story(proj)
{

}

void CpuWindow::Initialize()
{



}

void CpuWindow::Draw()
{
    static ImVec2 size(320.0f, 240.0f);

    
    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);
    
    static ImGuiTableFlags tableFlags =
                 ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
                 | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
                 | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
                 | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
                 | ImGuiTableFlags_SizingFixedFit;


    if (ImGui::BeginTable("cpu_table", 2, tableFlags))
    {
        ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);

        for (int i = 0; i < REGISTER_COUNT; i++)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",CpuRegNames[i]);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%04X", m_story.GetRegister(i));
        }

        ImGui::EndTable();
    }

    WindowBase::EndDraw();
}
