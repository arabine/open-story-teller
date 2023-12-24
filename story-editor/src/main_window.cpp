#include "main_window.h"
#include <filesystem>
#include <random>

#include "platform_folders.h"
#include "uuid.h"

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
    : m_emulatorWindow(*this)
    , m_resourcesWindow(*this)
    , m_nodeEditorWindow(*this)

{
    m_project.Clear();
}

MainWindow::~MainWindow()
{

}

void MainWindow::DrawStatusBar()
{
    float statusWindowHeight = ImGui::GetFrameHeight() * 1.4f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusWindowHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, statusWindowHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("StatusBar", nullptr, windowFlags);



    if (true)
    {
        float dy = ImGui::GetFontSize() * 0.15f;

        ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 14.f * ImGui::GetFontSize());

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - dy);
        ImGui::Text("FPS: %.1f", 1000.0f / ImGui::GetIO().Framerate);
    }

    ImGui::End();
}

void MainWindow::DrawMainMenuBar()
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

            if (ImGui::MenuItem("Save project"))
            {
                SaveProject();
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
        std::string home = pf::getUserHome() + "/";

#ifdef DEBUG
        home = "/home/anthony/ostproj/ba869e4b-03d6-4249-9202-85b4cec767a7/";
#endif

        ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlgKey", "Choose File", ".json", home, 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImVec2 parent_pos = ImGui::GetWindowPos();
    //ImVec2 parent_size = ImGui::GetWindowSize();
    //ImVec2 center(parent_pos.x + parent_size.x * 0.5f, parent_pos.y + parent_size.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Story Editor V2");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");
        ImGui::Text("http://www.openstoryteller.org");
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

    m_editor.Initialize();
    m_emulatorWindow.Initialize();
    m_nodeEditorWindow.Initialize();
    m_PropertiesWindow.Initialize();

    LoadParams();
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


            m_project.Initialize(filePathName);

            nlohmann::json model;

            if (m_project.Load(filePathName, model, m_resources))
            {
                Log("Open project success");
                m_nodeEditorWindow.Load(model);
                EnableProject();
            }
            else
            {
                Log("Open project error");
            }


     //       RefreshProjectInformation();  // FIXME
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }
}

void MainWindow::EnableProject()
{
    auto proj = m_project.GetProjectFilePath();
    // Add to recent if not exists
    if (std::find(m_recentProjects.begin(), m_recentProjects.end(), proj) != m_recentProjects.end())
    {
        m_recentProjects.push_back(proj);
        // Limit to 10 recent projects
        if (m_recentProjects.size() > 10) {
            m_recentProjects.pop_back();
        }
    }
/*  // FIXME
    m_ostHmiDock->Open();
    m_resourcesDock->Open();
    m_scriptEditorDock->Open();
    m_vmDock->Open();
    m_ramView->Open();
    m_romView->Open();
    m_logDock->Open();

    m_toolbar->SetActionsActive(true);
    m_view->setEnabled(true);
*/
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
                auto p = std::filesystem::path(projdir) / std::filesystem::path("project.json");
                m_project.Initialize(p.generic_string());

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
    nlohmann::json model;
    m_nodeEditorWindow.Save(model);
    m_project.Save(model, m_resources);
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
        DrawMainMenuBar();
       // DrawStatusBar();

        // ------------  Draw all windows
        m_consoleWindow.Draw();
        m_emulatorWindow.Draw();
        m_editor.Draw();
        m_resourcesWindow.Draw();
        m_nodeEditorWindow.Draw();

        m_PropertiesWindow.SetSelectedNode(m_nodeEditorWindow.GetSelectedNode());
        m_PropertiesWindow.Draw();

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

void MainWindow::Log(const std::string &txt, bool critical)
{
    m_consoleWindow.AddLog(txt, critical ? 1 : 0);
}

void MainWindow::PlaySoundFile(const std::string &fileName)
{
    Log("Play sound file: " + fileName);
    m_project.PlaySoundFile(fileName);
}

std::string MainWindow::BuildFullAssetsPath(const std::string &fileName) const
{
    return m_project.BuildFullAssetsPath(fileName);
}

std::pair<FilterIterator, FilterIterator> MainWindow::Images()
{
    return m_resources.Images();
}

std::pair<FilterIterator, FilterIterator> MainWindow::Sounds()
{
    return m_resources.Sounds();
}

void MainWindow::AddResource(std::shared_ptr<Resource> res)
{
    m_resources.Add(res);
}

void MainWindow::ClearResources()
{
    m_resources.Clear();
}

std::pair<FilterIterator, FilterIterator> MainWindow::Resources()
{
    return m_resources.Items();
}

void MainWindow::DeleteResource(FilterIterator &it)
{
    return m_resources.Delete(it);
}

void MainWindow::Build()
{
    // 1. First compile nodes to assembly
    CompileToAssembler();

    // 2. Compile the assembly to machine binary
    GenerateBinary();

    // 3. Convert all media to desired type format
    ConvertResources();
}

std::string MainWindow::GetNodeEntryLabel(unsigned long nodeId)
{
    return m_nodeEditorWindow.GetNodeEntryLabel(nodeId);
}

std::list<std::shared_ptr<Connection>> MainWindow::GetNodeConnections(unsigned long nodeId)
{
    return m_nodeEditorWindow.GetNodeConnections(nodeId);
}

bool MainWindow::CompileToAssembler()
{
    // 1. Check if the model can be compiled, check for errors and report
    // FIXME

    // 2. Generate the assembly code from the model
    m_currentCode = m_nodeEditorWindow.Build();

    // Add global functions
    {
        std::string buffer;

        std::ifstream f("scripts/media.asm");
        f.seekg(0, std::ios::end);
        buffer.resize(f.tellg());
        f.seekg(0);
        f.read(buffer.data(), buffer.size());
        m_currentCode += buffer;
    }

    m_editor.SetScript(m_currentCode);

    return true;
}

void MainWindow::GenerateBinary()
{
    m_dbg.run_result = VM_FINISHED;
    m_dbg.free_run = false;

    if (m_assembler.Parse(m_currentCode) == true )
    {
        if (m_assembler.BuildBinary(m_program, m_result) == true)
        {
            m_result.Print();

            Log("Binary successfully generated.");

            // Update ROM memory
            std::copy(m_program.begin(), m_program.end(), m_rom_data);

            // FIXME
//            m_ramView->SetMemory(m_ram_data, sizeof(m_ram_data));
//            m_romView->SetMemory(m_rom_data, m_program.size());
            m_project.SaveStory(m_program);
            chip32_initialize(&m_chip32_ctx);
            m_dbg.run_result = VM_OK;
            UpdateVmView();
            //            DebugContext::DumpCodeAssembler(m_assembler);
        }
        else
        {
            Chip32::Assembler::Error err = m_assembler.GetLastError();
            Log(err.ToString(), true);
            m_editor.AddError(err.line, err.message); // show also the error in the code editor
        }
    }
    else
    {
        Chip32::Assembler::Error err = m_assembler.GetLastError();
        Log(err.ToString(), true);
        m_editor.AddError(err.line, err.message); // show also the error in the code editor
    }
}

void MainWindow::UpdateVmView()
{
    // FIXME
//    m_vmDock->updateRegistersView(m_chip32_ctx);
//    highlightNextLine();
    // Refresh RAM content
//    m_ramView->SetMemory(m_ram_data, m_chip32_ctx.ram.size);
}

void MainWindow::ConvertResources()
{

}

void MainWindow::SaveParams()
{

}

void MainWindow::LoadParams()
{

}
