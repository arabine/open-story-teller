#include "library_window.h"
#include "gui.h"
#include "ImGuiFileDialog.h"
#include <filesystem>

LibraryWindow::LibraryWindow(IStoryManager &project, LibraryManager &library)
    : WindowBase("Library Manager")
    , m_storyManager(project)
    , m_libraryManager(library)
{

}

void LibraryWindow::Initialize()
{



}

static bool canValidateDialog = false;
inline void InfosPane(const char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
    ImGui::Text("Selected Filter : %s", vFilter);
    ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
    if (vCantContinue)
        *vCantContinue = canValidateDialog;
}


void LibraryWindow::Draw()
{
//    if (!IsVisible())
//    {
//        return;
//    }


    WindowBase::BeginDraw();
    ImGui::SetWindowSize(ImVec2(626, 744), ImGuiCond_FirstUseEver);

    if (ImGui::Button("Select directory"))
    {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseLibraryDirDialog", "Choose a library directory", nullptr, ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    ImGui::SameLine();

    if (!m_libraryManager.IsInitialized())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
        ImGui::Text("No any library directory set. Please select one where stories will be located.");
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::Text("Library path: %s", m_libraryManager.LibraryPath().c_str());
    }

    if (ImGui::Button("Scan library"))
    {
        m_libraryManager.Scan();
    }

    ImGui::SameLine();

    if (ImGui::Button("Import story"))
    {
        ImGuiFileDialog::Instance()->OpenDialogWithPane("ImportStoryDlgKey", "Import story", "", "", InfosPane);
    }

    if (m_libraryManager.IsInitialized())
    {
        static ImGuiTableFlags tableFlags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
            | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("library_table", 2, tableFlags))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableHeadersRow();

            for (auto &p : m_libraryManager)
            {
                ImGui::TableNextColumn();
                ImGui::Text("%s", p->GetName().c_str());

                ImGui::TableNextColumn();

                if (ImGui::SmallButton("Load"))
                {
                    auto filename = std::filesystem::path(m_libraryManager.LibraryPath()) / p->GetUuid() / std::filesystem::path("project.json");
                    m_storyManager.OpenProject(filename);
                }

                ImGui::SameLine();

                if (ImGui::SmallButton("Remove"))
                {

                }
            }

            ImGui::EndTable();
        }
    }


    // ---------------- ADD EXISTING STORY TELLER STORY
    if (ImGuiFileDialog::Instance()->Display("ImportStoryDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            std::string filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
            // here convert from string because a string was passed as a userDatas, but it can be what you want
            // std::string userDatas;
            // if (ImGuiFileDialog::Instance()->GetUserDatas())
            //     userDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
            // auto selection = ImGuiFileDialog::Instance()->GetSelection(); // multiselection

            // action
        }
        // close
        ImGuiFileDialog::Instance()->Close();
    }


    // ---------------- CHOOSE LIBRARY DIR
    if (ImGuiFileDialog::Instance()->Display("ChooseLibraryDirDialog"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string projdir = ImGuiFileDialog::Instance()->GetCurrentPath();

            if (std::filesystem::is_directory(projdir))
            {
                m_libraryManager.Initialize(projdir);
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }


    WindowBase::EndDraw();
}

