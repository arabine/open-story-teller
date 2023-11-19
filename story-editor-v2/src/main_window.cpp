#include "main_window.h"
#include <filesystem>
#include <random>

#ifdef USE_WINDOWS_OS
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#include "IconsMaterialDesignIcons.h"
#include "ImGuiFileDialog.h"

MainWindow::MainWindow()
    : m_resourcesWindow(m_project)
{
    m_project.Clear();
}

MainWindow::~MainWindow()
{

}

void MainWindow::SetupMainMenuBar()
{
    bool showAboutPopup = false;
    bool showParameters = false;
    bool showNewProject = false;
    bool showOpenProject = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New project"))
            {
                showNewProject = true;
            }

            if (ImGui::MenuItem("Open project"))
            {
                showOpenProject = true;
            }

            if (ImGui::MenuItem("Close project"))
            {
                CloseProject();
            }

            if (ImGui::MenuItem("Paramètres"))
            {
                showParameters = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showAboutPopup)
    {
        ImGui::OpenPopup("AboutPopup");
    }

    if (showParameters)
    {
        ImGui::OpenPopup("Options");
    }

    if (showNewProject)
    {
        ImGui::OpenPopup("NewProjectPopup");
    }

    if (showOpenProject)
    {
        ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlgKey", "Choose File", "project.json", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImVec2 parent_pos = ImGui::GetWindowPos();
    //ImVec2 parent_size = ImGui::GetWindowSize();
    //ImVec2 center(parent_pos.x + parent_size.x * 0.5f, parent_pos.y + parent_size.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Story Editor V1");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");
//        ImGui::Text("%s", SDL_GetPlatform());
//        ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
//        ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::SameLine(300);
        if (ImGui::Button("Close", ImVec2(120, 40)))
        {
           ImGui::CloseCurrentPopup();
        }
       ImGui::EndPopup();
    }
}

void MainWindow::Initialize()
{
    // GUI Init
    gui.Initialize();
  //  gui.ApplyTheme();

    editor.Initialize();
    m_emulatorWindow.Initialize();
    m_nodeEditorWindow.Initialize();

    LoadParams();
}


void MainWindow::ShowOptionsWindow()
{
    static int pingState = 0;

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 0.0f));
    if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushItemWidth(-1.0f);

        ImGui::Text("Adresse du serveur");
        ImGui::SameLine();
        ImGui::InputText("##addr",  mBufAddress, sizeof(mBufAddress));

        ImGui::Text("Chemin de récupération");
        ImGui::SameLine();
        ImGui::InputText("##rec_path",  mBufReceivePath, sizeof(mBufReceivePath));

        ImGui::Text("Chemin d'envoi des données");
        ImGui::SameLine();
        ImGui::InputText("##send_path",  mBufSendPath, sizeof(mBufSendPath));

        ImGui::PushItemWidth(100);
        ImGui::Text("Port");
        ImGui::SameLine();
        ImGui::InputText("##port",  mBufPort, sizeof(mBufPort), ImGuiInputTextFlags_CharsDecimal);

        float width = 50;
        ImGui::BeginGroup();
        ImGui::PushID("Zebra7500");
        ImGui::TextUnformatted("Adresse IP Zebra7500");
        ImGui::SameLine();
        for (int i = 0; i < 4; i++) {
            ImGui::PushItemWidth(width);
            ImGui::PushID(i);
            bool invalid_octet = false;
            if (octets[i] > 255) {
                // Make values over 255 red, and when focus is lost reset it to 255.
                octets[i] = 255;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            if (octets[i] < 0) {
                // Make values below 0 yellow, and when focus is lost reset it to 0.
                octets[i] = 0;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            ImGui::InputInt("##v", &octets[i], 0, 0, ImGuiInputTextFlags_CharsDecimal);
            if (invalid_octet) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::PopItemWidth();
        }
        ImGui::PopID();
        ImGui::EndGroup();

        // Example action button and way to build the IP string
        ImGui::SameLine();

        ImGui::SameLine();
        if (pingState == 1)
        {
            ImGui::TextUnformatted("Ping en cours...");
        }
        else if (pingState == 2)
        {
            ImGui::TextUnformatted("Ping succès!");
        }
        else if (pingState == 3)
        {
            ImGui::TextUnformatted("Ping erreur :(");
        }
        else
        {
            ImGui::TextUnformatted("");
        }

        ImGui::Separator();


        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool MainWindow::ShowQuitConfirm()
{
    bool quitRequest = false;
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
   // ImGui::SetNextWindowSize(ImVec2(200, 150));
    if (ImGui::BeginPopupModal("QuitConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Voulez-vous vraiment quitter le logiciel ?");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            quitRequest = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return quitRequest;
}


void MainWindow::OpenProjectDialog()
{
    if (ImGuiFileDialog::Instance()->Display("OpenProjectDlgKey"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            bool success = false;

            m_project.Initialize(filePathName);

            nlohmann::json model;

            if (m_project.Load(filePathName, model))
            {
                m_model.Load(model);
                EnableProject();
            }
            else
            {
                qWarning() << errorMsg;
                QMessageBox::critical(this, tr("Open project error"), errorMsg);
            }

            m_resourceModel.EndChange();
            RefreshProjectInformation();

            /*

            // action


            std::filesystem::path p(filePathName);
            std::filesystem::path p2 = m_project.AssetsPath() /  p.filename().generic_string();
            std::filesystem::copy(p, p2, std::filesystem::copy_options::overwrite_existing);

            Resource res;

            std::string ext = p.extension().string();
            ext.erase(ext.begin()); // remove '.' dot sign
            std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

            res.format = ext;
            res.type = m_soundFile ? "sound" : "image";
            res.file = p.filename().generic_string();
            m_project.AppendResource(res);
*/
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }
}

void MainWindow::EnableProject()
{
    // Add to recent if not exists
    if (!m_recentProjects.contains(m_project.GetProjectFilePath().c_str()))
    {
        m_recentProjects.push_front(m_project.GetProjectFilePath().c_str());
        // Limit to 10 recent projects
        if (m_recentProjects.size() > 10) {
            m_recentProjects.pop_back();
        }
        m_toolbar->GenerateRecentProjectsMenu(m_recentProjects);
    }

    m_ostHmiDock->Open();
    m_resourcesDock->Open();
    m_scriptEditorDock->Open();
    m_vmDock->Open();
    m_ramView->Open();
    m_romView->Open();
    m_logDock->Open();

    m_toolbar->SetActionsActive(true);
    m_view->setEnabled(true);
}


void MainWindow::NewProjectPopup()
{
    static std::string projdir;
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("NewProjectPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("New project parameters (directory must be empty)");
        ImGui::Separator();

        ImGui::Text("Directory: "); ImGui::SameLine();
        static char project_dir[256] = "";
        ImGui::InputTextWithHint("##project_path", "Project path", project_dir, IM_ARRAYSIZE(project_dir));
        if (ImGui::Button( ICON_MDI_FOLDER " ..."))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDialog", "Choose File", nullptr, ".", 1, nullptr, ImGuiFileDialogFlags_Modal);
        }

        // display
        if (ImGuiFileDialog::Instance()->Display("ChooseDirDialog"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                projdir = ImGuiFileDialog::Instance()->GetCurrentPath();

            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }


        ImGui::Text("Project name: "); ImGui::SameLine();
        static char project_name[256] = "";
        ImGui::InputTextWithHint("##project_name", "Project name", project_name, IM_ARRAYSIZE(project_name));

        ImGui::Text("Size of display screen: ");
        ImGui::SameLine();

        static ImGuiComboFlags flags = 0;

        static int display_item_current_idx = 0; // Here we store our selection data as an index.
        static int image_item_current_idx = 0; // Here we store our selection data as an index.
        static int sound_item_current_idx = 0; // Here we store our selection data as an index.

        {
            // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
            // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
            // stored in the object itself, etc.)
            const char* display_items[] = { "320x240", "640x480" };

            const char* combo_preview_value = display_items[display_item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
            if (ImGui::BeginCombo("##ComboDisplay", combo_preview_value, flags))
            {
                for (int n = 0; n < IM_ARRAYSIZE(display_items); n++)
                {
                    const bool is_selected = (display_item_current_idx == n);
                    if (ImGui::Selectable(display_items[n], is_selected))
                        display_item_current_idx = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Text("Image format: ");
        ImGui::SameLine();
        {
            // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
            // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
            // stored in the object itself, etc.)
            const char* image_items[] = { "BMP (compressed 4-bit palette)", "QOIF (Quite Ok Image Format" };
            const char* image_combo_preview_value = image_items[image_item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
            if (ImGui::BeginCombo("##ComboImage", image_combo_preview_value, flags))
            {
                for (int n = 0; n < IM_ARRAYSIZE(image_items); n++)
                {
                    const bool is_selected = (image_item_current_idx == n);
                    if (ImGui::Selectable(image_items[n], is_selected))
                        image_item_current_idx = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }


        ImGui::Text("Sound format: ");
        ImGui::SameLine();
        {
            // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
            // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
            // stored in the object itself, etc.)
            const char* sound_items[] = { "WAV (16-bit stereo)", "QOAF (Quite Ok Audio Format" };
            const char* sound_combo_preview_value = sound_items[sound_item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
            if (ImGui::BeginCombo("##ComboSound", sound_combo_preview_value, flags))
            {
                for (int n = 0; n < IM_ARRAYSIZE(sound_items); n++)
                {
                    const bool is_selected = (sound_item_current_idx == n);
                    if (ImGui::Selectable(sound_items[n], is_selected))
                        sound_item_current_idx = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }


        auto GetImageFormat = [](int idx) -> StoryProject::ImageFormat
        {
            StoryProject::ImageFormat img{StoryProject::IMG_FORMAT_BMP_4BITS};
            if (idx < StoryProject::IMG_FORMAT_COUNT) {
                img = static_cast<StoryProject::ImageFormat>(idx);
            }
            return img;
        };

        auto GetSoundFormat = [](int idx) -> StoryProject::SoundFormat {

            StoryProject::SoundFormat img{StoryProject::SND_FORMAT_WAV};
            if (idx < StoryProject::IMG_FORMAT_COUNT) {
                img = static_cast<StoryProject::SoundFormat>(idx);
            }

            return img;
        };



        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            bool valid{true};

            if (!std::filesystem::is_directory(projdir))
            {
                valid = false;
            }

            if (valid)
            {
                m_project.Initialize(std::filesystem::path(projdir) / "project.json");

                if (display_item_current_idx == 0)
                {
                    m_project.SetDisplayFormat(320, 240);
                }
                else
                {
                    m_project.SetDisplayFormat(640, 480);
                }

                m_project.SetImageFormat(GetImageFormat(image_item_current_idx));
                m_project.SetSoundFormat(GetSoundFormat(sound_item_current_idx));
                m_project.SetName(project_name);
                m_project.SetUuid(UUID().String());

                SaveProject();
                EnableProject();

                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    else
    {
        projdir = "";
    }
}

void MainWindow::SaveProject()
{
    nlohmann::json model; // = m_model.Save();
    m_project.Save(model);
}

void MainWindow::CloseProject()
{
    m_project.Clear();
    m_nodeEditorWindow.Clear();

//    m_model.Clear();

//    m_ostHmiDock->Close();
//    m_resourcesDock->Close();
//    m_scriptEditorDock->Close();
//    m_vmDock->Close();
//    m_ramView->Close();
//    m_romView->Close();
//    m_logDock->Close();

//    m_toolbar->SetActionsActive(false);
//    m_view->setEnabled(false);
}



void MainWindow::Loop()
{
    // Main loop
    bool done = false;

    while (!done)
    {
        bool aboutToClose = gui.PollEvent();

        gui.StartFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        SetupMainMenuBar();

        console.Draw("Console", nullptr);
        m_emulatorWindow.Draw("Emulator", nullptr);
        editor.Draw("Code Editor", nullptr);

        m_resourcesWindow.Draw("Resources", nullptr);
        m_nodeEditorWindow.Draw("Blueprint", nullptr);
        ShowOptionsWindow();

        NewProjectPopup();
        OpenProjectDialog();

        if (aboutToClose)
        {
             ImGui::OpenPopup("QuitConfirm");
        }
        if (ShowQuitConfirm())
        {
            done = true;
        }

        gui.EndFrame();


    }

    gui.Destroy();
}

void MainWindow::SaveParams()
{

}

void MainWindow::LoadParams()
{

}