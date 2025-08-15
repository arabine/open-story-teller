#include "main_window.h"
#include <filesystem>
#include <SDL3/SDL.h>
#include "platform_folders.h"

#include "media_converter.h"

#include "pack_archive.h"
#include "uuid.h"
#include "sys_lib.h"

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
#include "imgui_memory_editor.h"

#include "app_controller.h"
#include "all_events.h"

MainWindow::MainWindow(ILogger& logger, EventBus& eventBus, AppController& appController)
    : m_logger(logger)
    , m_eventBus(eventBus)
    , m_appController(appController)
     , m_emulatorDock(appController)
    , m_debuggerWindow(appController)
    , m_cpuWindow(appController)
    , m_resourcesDock(appController.GetResourceManager(), appController)
    , m_nodeEditorWindow(appController, appController.GetNodesFactory(), IStoryProject::PROJECT_TYPE_STORY)
    , m_moduleEditorWindow(appController, appController.GetNodesFactory(), IStoryProject::PROJECT_TYPE_MODULE)
    , m_libraryWindow(appController, appController.GetLibraryManager(), appController.GetNodesFactory())
    , m_variablesWindow(appController)
    , m_projectPropertiesDialog(appController, appController.GetResourceManager())
{

    logger.RegisterSubject(shared_from_this());

    CloseProject();
    CloseModule();

    // define style for all directories
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_MDI_FOLDER);
    // define style for all files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ICON_MDI_FILE);


    m_eventBus.Subscribe<OpenProjectEvent>([this](const OpenProjectEvent &event) {
        OpenProject(event.GetUuid());
    });

    m_eventBus.Subscribe<OpenFunctionEvent>([this](const OpenFunctionEvent &event) {
        OpenFunction(event.GetUuid(), event.GetName());
    });
}

MainWindow::~MainWindow()
{
    m_appController.SaveParams();
}


float MainWindow::DrawMainMenuBar()
{
    bool showNewProject = false;
    bool showOpenProject = false;

    float height = 60; 

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {

            if (ImGui::MenuItem("New story project"))
            {
                CloseProject();

                m_story = m_appController.NewProject();

                if (m_story)
                {
                    m_appController.SaveProject();
                    OpenProject(m_story->GetUuid());
                }
            }

            if (ImGui::MenuItem("New module"))
            {
                // Current module project
                CloseModule();
                showNewProject = true;
                NewModule();
            }

            if (ImGui::MenuItem("Save module"))
            {
                SaveModule();
            }

/*
            if (ImGui::BeginMenu("Open Recent"))
            {
                for (auto &e : m_recentProjects)
                {
                    if (e.size() > 0)
                    {
                        if (ImGui::MenuItem(e.c_str()))
                        {
                            OpenProject(e);
                        }
                    }
                }

                ImGui::EndMenu();
            }
*/

            bool init = m_story ? true : false; // local copy because CloseProject() changes the status between BeginDisabled/EndDisabled
            if (!init)
                ImGui::BeginDisabled();

            ImGui::Separator();
            if (ImGui::MenuItem("Save project"))
            {
                m_appController.SaveProject();
            }

            if (ImGui::MenuItem("Close project"))
            {
                CloseProject();
            }

            if (ImGui::MenuItem("Project settings"))
            {
                m_projectPropertiesDialog.Show();
            }

            if (ImGui::MenuItem("Close module"))
            {
                CloseModule();
            }

            if (!init)
                ImGui::EndDisabled();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                m_aboutDialog.Show();
            }
            ImGui::EndMenu();
        }


        height = ImGui::GetFrameHeight();
        ImGui::EndMainMenuBar();
    }

    m_aboutDialog.Open();
    if (m_story)
    {
        m_projectPropertiesDialog.Open();
    }

    return height;
}

bool MainWindow::Initialize()
{
    bool success = false;

    // GUI Init
    if (m_gui.Initialize())
    {
    //  gui.ApplyTheme();
        m_debuggerWindow.Initialize();
        m_emulatorDock.Initialize();
        m_nodeEditorWindow.Initialize();
        m_moduleEditorWindow.Initialize();
        m_PropertiesWindow.Initialize();
        m_libraryWindow.Initialize();

        success = true;
    }

    return success;
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
        ImGui::Text("Really qui without saving?");
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



void MainWindow::OpenProject(const std::string &uuid)
{
    m_nodeEditorWindow.Load(m_story);
    auto proj = m_story->GetProjectFilePath();
    // Add to recent if not exists
    if (std::find(m_recentProjects.begin(), m_recentProjects.end(), proj) == m_recentProjects.end())
    {
        m_recentProjects.push_back(proj);
        // Limit to 10 recent projects
        if (m_recentProjects.size() > 10) {
            m_recentProjects.pop_back();
        }

        // Save recent projects on disk
        m_appController.SaveParams();
    }

    m_nodeEditorWindow.Enable();
    
    m_emulatorDock.Enable();
    m_consoleWindow.Enable();
    m_debuggerWindow.Enable();
    m_resourcesDock.Enable();
    m_PropertiesWindow.Enable();
    m_variablesWindow.Enable();
    m_cpuWindow.Enable();

    RefreshProjectInformation();
}

void MainWindow::NewModule()
{
    auto module = m_appController.NewModule();
    m_moduleEditorWindow.Load(module);

    m_moduleEditorWindow.Enable();
}


void MainWindow::SaveModule()
{
    m_appController.SaveAllModules(m_resources);;
    m_logger.Log("Modules saved");
}

void MainWindow::OpenModule(const std::string &uuid)
{
    m_module = m_nodesFactory.GetModule(uuid);
    if (!m_module)
    {
        m_logger.Log("Cannot find module: " + uuid);
    }
    else if (m_module->Load(m_resources, m_nodesFactory))
    {
        m_logger.Log("Open module success");
        m_moduleEditorWindow.Load(m_module);
        m_moduleEditorWindow.Enable();
    }
    else
    {
        m_logger.Log("Open module error");
    }
}


void MainWindow::CloseProject()
{
    // FIXME: not sure but if present, we lost some information in the library manager

    // if (m_story)
    // {
    //     m_story->Clear();
    //     m_story.reset();
    // }

    m_appController.CloseProject();
    m_nodeEditorWindow.Clear();
    m_nodeEditorWindow.Disable();

    m_emulatorDock.ClearImage();
    m_consoleWindow.ClearLog();
    m_debuggerWindow.ClearErrors();
    m_debuggerWindow.SetScript("");   
    
    m_emulatorDock.Disable();
    m_debuggerWindow.Disable();
    m_resourcesDock.Disable();
    m_PropertiesWindow.Disable();
    m_variablesWindow.Disable();
    m_cpuWindow.Disable();

    RefreshProjectInformation();
}


void MainWindow::CloseModule()
{
    m_moduleEditorWindow.Clear();
    m_moduleEditorWindow.Disable();

}


void MainWindow::ImportProject(const std::string &filePathName, int format)
{
    (void) format;
    PackArchive archive(*this, m_nodesFactory);

    // On va déterminer le type de fichier selon l'extension
    auto ext = SysLib::GetFileExtension(filePathName);
    auto filename = SysLib::GetFileName(filePathName);

    if ((ext == "pk") || (filename == "ni"))
    {
        archive.ImportCommercialFormat(filePathName, m_libraryManager.LibraryPath(), m_libraryManager.CommercialDbView());
    }
    else if ((ext == "json") || (ext == "zip"))
    {
        archive.ImportStudioFormat(filePathName, m_libraryManager.LibraryPath());
    }
    else
    {
        m_logger.Log("Unknown file format: " + filePathName);
    }
}
 
void MainWindow::RefreshProjectInformation()
{
    std::string fullText = "Story Editor " + LibraryManager::GetVersion();

    if (m_story)
    {
        fullText += " - " + m_story->GetProjectFilePath();
    }
    m_gui.SetWindowTitle(fullText);
}


void MainWindow::DrawToolBar(float topPadding)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoDocking;

    // Définit la taille et la position de la barre d'outils
    ImVec2 size = ImVec2(60, ImGui::GetIO().DisplaySize.y - topPadding);  // Largeur de 60 pixels et hauteur égale à celle de l'écran
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(ImVec2(0, topPadding));  // Positionné à gauche et en haut

    // Création de la fenêtre pour la barre d'outils
    ImGui::Begin("ToolBar", nullptr, window_flags);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // rouge
    float old_size = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= 2.5;

    ImGui::PushFont(ImGui::GetFont());

    // Ajouter des boutons à la barre d'outils
    if (ImGui::Button(ICON_MDI_SPEAKER_STOP "##stop_sound", ImVec2(-1, 50))) {  // Le bouton prend toute la largeur de la fenêtre et a une hauteur de 50 pixels
        m_player.Stop();
    }
    
    ImGui::GetFont()->Scale = old_size;
    ImGui::PopFont();
    ImGui::PopStyleColor();

    
    // Fermeture de la fenêtre ImGui
    ImGui::End();
}

#include "imgui_internal.h"
void MainWindow::Loop()
{
    // Main loop
    bool done = false;

    while (!done)
    {
        Uint64 frameStart = SDL_GetTicks();
        bool aboutToClose = m_gui.PollEvent();

        m_gui.StartFrame();

        auto vp = ImGui::GetMainViewport();

        auto pos = vp->WorkPos;
        auto size = vp->WorkSize;
        pos.x += 60;
        size.x -= 60;
        vp->WorkPos = pos;
        vp->WorkSize = size;
        ImGui::DockSpaceOverViewport(0, vp);
        float height = DrawMainMenuBar();

        ProcessStory();

        // ------------  Draw all windows
        m_libraryWindow.Draw();

        if (m_libraryManager.IsInitialized())
        {
            m_consoleWindow.Draw();
            m_emulatorDock.Draw();
            m_debuggerWindow.Draw();
            m_resourcesDock.Draw();
            m_nodeEditorWindow.Draw();
            m_moduleEditorWindow.Draw();
            m_variablesWindow.Draw();
            m_cpuWindow.Draw();

            static MemoryEditor mem_edit_1;
            mem_edit_1.DrawWindow("RAM view", m_chip32_ctx.ram.mem, m_chip32_ctx.ram.size);

            m_PropertiesWindow.SetSelectedNode(m_nodeEditorWindow.GetSelectedNode());
            m_PropertiesWindow.Draw();


            // static ImGuiAxis toolbar2_axis = ImGuiAxis_Y;
            // DockingToolbar("Toolbar2", &toolbar2_axis);

            DrawToolBar(height);
        }

        m_aboutDialog.Draw();
        m_projectPropertiesDialog.Draw();

        if (aboutToClose)
        {
            ImGui::OpenPopup("QuitConfirm");
        }
        if (ShowQuitConfirm())
        {
            done = true;
        }

        m_gui.EndFrame();

        
        // Rendering and event handling
        Uint64 frameTime = SDL_GetTicks() - frameStart; // Temps écoulé pour la frame

        if (frameTime < 32) { // 16 ms =  60 FPS
            SDL_Delay(32 - frameTime); // Attendez pour compléter la frame
        }


    }

    m_gui.Destroy();
}


void MainWindow::LogEvent(const std::string &txt, bool critical)
{
    m_consoleWindow.AddLog(txt, critical ? 1 : 0);
}


void MainWindow::OpenFunction(const std::string &uuid, const std::string &name)
{
    m_nodeEditorWindow.OpenFunction(uuid, name);
}


void MainWindow::Build(bool compileonly)
{
    m_dbg.run_result = VM_FINISHED;
    m_dbg.free_run = false;

    if (!compileonly)
    {
        // 3. Convert all media to desired type format
        auto options = m_story->GetOptions();
        m_resources.ConvertResources(m_story->AssetsPath(), "", options.image_format, options.sound_format); // pas de répertoire de destination
    }

    Chip32::Assembler::Error err;
    m_debuggerWindow.ClearErrors();
    if (m_story->GenerateBinary(m_currentCode, err))
    {
        m_result.Print();

        if (m_story->CopyProgramTo(m_rom_data, sizeof (m_rom_data)))
        {
            //            m_ramView->SetMemory(m_ram_data, sizeof(m_ram_data));
//            m_romView->SetMemory(m_rom_data, m_program.size());
            m_story->SaveBinary();
            chip32_initialize(&m_chip32_ctx);
            m_dbg.run_result = VM_READY;
            UpdateVmView();
        }
        else
        {
            m_logger.Log("Program too big. Expand ROM memory.");
        }
    }
    else
    {
        m_logger.Log(err.ToString(), true);
        m_debuggerWindow.AddError(err.line, err.message); // show also the error in the code editor
    }
}


void MainWindow::SetExternalSourceFile(const std::string &filename)
{
    m_externalSourceFileName = filename;
}


void MainWindow::UpdateVmView()
{
    // FIXME
//    m_vmDock->updateRegistersView(m_chip32_ctx);

    // Highlight next line in the test editor
    uint32_t pcVal = m_chip32_ctx.registers[PC];

    if (m_story->GetAssemblyLine(pcVal, m_dbg.line))
    {
        m_debuggerWindow.HighlightLine(m_dbg.line);
        std::cout << "Executing line: " << m_dbg.line << std::endl;
    }
    else
    {
        // Not found
        m_logger.Log("Reached end or instruction not found line: " + std::to_string(m_dbg.line));
    }
    // Refresh RAM content
//    m_ramView->SetMemory(m_ram_data, m_chip32_ctx.ram.size);
}


void MainWindow::LoadParams()
{
    try {

        // Modules directory
        std::filesystem::path dlDir = std::filesystem::path(pf::getConfigHome()) / "ost_modules";
        std::filesystem::create_directories(dlDir);
        m_nodesFactory.SetModulesRootDirectory(dlDir.string());

        std::string loc = pf::getConfigHome() + "/ost_settings.json";
        // read a JSON file
        std::ifstream i(loc);
        nlohmann::json j;
        i >> j;

        nlohmann::json recents = j["recents"];
        for (auto& element : recents) {

            if (std::filesystem::exists(element))
            {
                m_recentProjects.push_back(element);
            }
        }

        nlohmann::json library_path = j["library_path"];

        if (std::filesystem::exists(library_path))
        {
            m_libraryManager.Initialize(library_path);
        }

        nlohmann::json store_url = j.value("store_url", "https://gist.githubusercontent.com/DantSu/3aea4c1fe15070bcf394a40b89aec33e/raw/stories.json");
        m_libraryManager.SetStoreUrl(store_url);

    }
    catch(std::exception &e)
    {
        m_logger.Log(e.what(), true);
    }
}
