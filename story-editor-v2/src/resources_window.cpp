#include "resources_window.h"
#include "imgui.h"
#include <random>
#include <filesystem>
#include <memory>
#include "resource.h"
#include "ImGuiFileDialog.h"

//static thread_pool pool;

ResourcesWindow::ResourcesWindow(IStoryProject &project)
    : WindowBase("Resources")
    , m_project(project)
{

}

ResourcesWindow::~ResourcesWindow()
{

}

void ResourcesWindow::ChooseFile()
{
    static const char * soundFormats = ".wav,.mp3,.ogg,.flac";
    static const char * imagesFormats = ".bmp *.png *.jpg";

    if (m_showImportDialog)
    {
        m_showImportDialog = false;
        // open Dialog Simple
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", m_soundFile ? soundFormats : imagesFormats, ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            // action


            std::filesystem::path p(filePathName);
            std::filesystem::path p2 = m_project.BuildFullAssetsPath( p.filename());
            std::filesystem::copy(p, p2, std::filesystem::copy_options::overwrite_existing);

            auto res = std::make_shared<Resource>();

            std::string ext = p.extension().string();
            ext.erase(ext.begin()); // remove '.' dot sign
            std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

            res->format = ext;
            res->type = m_soundFile ? "sound" : "image";
            res->file = p.filename().generic_string();
            m_project.AddResource(res);
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }
}


void ResourcesWindow::Draw()
{
    WindowBase::BeginDraw();


    if (ImGui::Button("Add sound"))
    {
        m_showImportDialog = true;
        m_soundFile = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Add image"))
    {
        m_showImportDialog = true;
        m_soundFile = false;
    }

    ChooseFile();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | 
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;

    if (ImGui::BeginTable("table1", 4, tableFlags))
    {
        ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Format", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableHeadersRow();

        auto [b, e] = m_project.Resources();
        for (auto it = b; it != e; ++it)
        {
            ImGui::TableNextColumn();
            ImGui::Text("%s", (*it)->file.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", (*it)->format.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", (*it)->description.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", (*it)->type.c_str());
        }

        ImGui::EndTable();
    }

    WindowBase::EndDraw();
}

