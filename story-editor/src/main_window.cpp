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

#include "nodes_factory.h"
#include "media_node_widget.h"
#include "call_function_node_widget.h"
#include "module_node_widget.h"
#include "variable_node_widget.h"
#include "operator_node_widget.h"
#include "print_node_widget.h"
#include "syscall_node_widget.h"
#include "function_entry_widget.h"


MainWindow::MainWindow(ILogger& logger, EventBus& eventBus, AppController& appController)
    : m_logger(logger)
    , m_eventBus(eventBus)
    , m_appController(appController)
     , m_emulatorDock(appController)
    , m_debuggerWindow(appController)
    , m_cpuWindow(appController)
    , m_resourcesDock(appController.GetResourceManager(), appController)
    , m_nodeEditorWindow(appController, appController.GetNodesFactory(), m_widgetFactory, IStoryProject::PROJECT_TYPE_STORY)
    , m_moduleEditorWindow(appController, appController.GetNodesFactory(), m_widgetFactory, IStoryProject::PROJECT_TYPE_MODULE)
    , m_libraryWindow(appController, appController.GetLibraryManager(), appController.GetNodesFactory(), eventBus)
    , m_projectPropertiesDialog(appController, appController.GetResourceManager())
{
    CloseProject();
    CloseModule();

    // define style for all directories
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_MDI_FOLDER);
    // define style for all files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ICON_MDI_FILE);


    // g OperatorNodeUuid = "0226fdac-8f7a-47d7-8584-b23aceb712ec";
// static const std::string CallFunctionNodeUuid = "02745f38-9b11-49fe-94b1-b2a6b78249fb";
// static const std::string VariableNodeUuid = "020cca4e-9cdc-47e7-a6a5-53e4c9152ed0";
// static const std::string PrintNodeUuid = "02ee27bc-ff1d-4f94-b700-eab55052ad1c";
// static const std::string SyscallNodeUuid = "02225cff-4975-400e-8130-41524d8af773";
// static const std::string ModuleNodeUuid = "02e4c728-ef72-4003-b7c8-2bee8834a47e";


    // registerNode<MediaNodeWidget>("media-node");
    m_widgetFactory.registerNode<OperatorNodeWidget>(OperatorNodeUuid);
    m_widgetFactory.registerNode<CallFunctionNodeWidget>(CallFunctionNodeUuid);
    // m_widgetFactory.registerNode<ModuleNodeWidget>("module-node");
    m_widgetFactory.registerNode<VariableNodeWidget>(VariableNodeUuid);
    m_widgetFactory.registerNode<PrintNodeWidget>(PrintNodeUuid);
    m_widgetFactory.registerNode<SyscallNodeWidget>(SyscallNodeUuid);
    m_widgetFactory.registerNode<FunctionEntryWidget>(FunctionEntryNodeUuid);

    m_eventBus.Subscribe<OpenProjectEvent>([this](const OpenProjectEvent &event) {
        OpenProject(event.GetUuid());
    });

    m_eventBus.Subscribe<OpenFunctionEvent>([this](const OpenFunctionEvent &event) {
        OpenFunction(event.GetUuid(), event.GetName());
    });

    m_eventBus.Subscribe<GenericResultEvent>([this](const GenericResultEvent &event) {

        if (event.IsSuccess()) {
            m_logger.Log("Operation successful: " + event.GetMessage());
            m_toastNotifier.addToast("Success", event.GetMessage(), ToastType::Success);
        } else {
            m_logger.Log("Operation failed: " + event.GetMessage(), true);
            m_toastNotifier.addToast("Error", event.GetMessage(), ToastType::Error);
        }
    });

    m_eventBus.Subscribe<ModuleEvent>([this](const ModuleEvent &event) {
        if (event.GetType() == ModuleEvent::Type::Open) {
            OpenModule(event.GetUuid());
        } else if (event.GetType() == ModuleEvent::Type::Closed) {
            CloseModule();
        } else if (event.GetType() == ModuleEvent::Type::BuildSuccess) {
            m_toastNotifier.addToast("Module", "Module built successfully", ToastType::Success);
            m_debuggerWindow.SetScript(m_appController.GetModuleAssembly());
        } else if (event.GetType() == ModuleEvent::Type::BuildFailure) {
            m_toastNotifier.addToast("Module", "Module build failed", ToastType::Error);
        }
    });
}

MainWindow::~MainWindow()
{
    m_gui.Destroy();
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


            if (!init)
                ImGui::EndDisabled();

            ImGui::Separator();

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

            if (ImGui::MenuItem("Close module"))
            {
                CloseModule();
            }


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

    m_logger.RegisterSubject(shared_from_this());

    // GUI Init
    if (m_gui.Initialize())
    {
    //  gui.ApplyTheme();
        m_debuggerWindow.Initialize();
        m_emulatorDock.Initialize();
        m_nodeEditorWindow.Initialize();
        m_moduleEditorWindow.Initialize();
        m_propertiesWindow.Initialize();
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
    m_propertiesWindow.Enable();
    m_variablesWindow.Enable();
    m_cpuWindow.Enable();

    RefreshProjectInformation();
}

void MainWindow::NewModule()
{
    auto module = m_appController.NewModule();

    if (module)
    {
        m_moduleEditorWindow.Load(module);
        m_moduleEditorWindow.Enable();
        m_logger.Log("New module created.");
    }
    else
    {
        m_logger.Log("Failed to create new module.");
    }
}

void MainWindow::SaveModule()
{
    m_moduleEditorWindow.SaveNodesToProject();
    m_appController.SaveModule();
    m_logger.Log("Modules saved");
    m_toastNotifier.addToast("Module", "Module saved", ToastType::Success);
}

void MainWindow::OpenModule(const std::string &uuid)
{
    auto module = m_appController.OpenModule(uuid);
    if (module)
    {
        m_moduleEditorWindow.Load(module);
        m_moduleEditorWindow.Enable();

        m_emulatorDock.Enable();
        m_consoleWindow.Enable();
        m_debuggerWindow.Enable();
        m_resourcesDock.Enable();
        m_propertiesWindow.Enable();
        m_variablesWindow.Enable();
        m_cpuWindow.Enable();
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
    m_propertiesWindow.Disable();
    m_variablesWindow.Disable();
    m_cpuWindow.Disable();

    RefreshProjectInformation();
}


void MainWindow::CloseModule()
{
    m_moduleEditorWindow.Clear();
    m_moduleEditorWindow.Disable();

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
    ImVec2 size = ImVec2(60, ImGui::GetIO().DisplaySize.y - topPadding);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(ImVec2(0, topPadding));

    // Style pour les coins arrondis des boutons
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    
    // Création de la fenêtre pour la barre d'outils
    if (ImGui::Begin("ToolBar", nullptr, window_flags))
    {
        // Taille réduite des boutons
        float buttonSize = 36.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float offsetX = (windowWidth - buttonSize) * 0.5f;
        
        // Style pour les couleurs des boutons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        // Taille de fonte réduite pour les icônes
        float old_size = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale *= 1.2f;
        ImGui::PushFont(ImGui::GetFont());

        // Bouton Stop Sound
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button(ICON_MDI_SPEAKER_STOP "##stop_sound", ImVec2(buttonSize, buttonSize))) {
            m_appController.StopAudio();
        }
        ImGui::PopStyleColor();
        
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Stop Sound");
        }

        // Espacement vertical
        ImGui::Dummy(ImVec2(0, 8));

        // Bouton Build
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.9f, 1.0f));
        if (ImGui::Button(ICON_MDI_HAMMER "##build_project", ImVec2(buttonSize, buttonSize))) {
            // Compile story if window focused, otherwise module
            if (m_nodeEditorWindow.IsFocused())
            {
                m_appController.CompileNodes(IStoryProject::PROJECT_TYPE_STORY);
            }
            else
            {
                m_appController.CompileNodes(IStoryProject::PROJECT_TYPE_MODULE);
            }
        }
        ImGui::PopStyleColor();
        
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Build Project");
        }

        // Restaurer le scale de la fonte
        ImGui::GetFont()->Scale = old_size;
        ImGui::PopFont();

        // Pop les couleurs des boutons
        ImGui::PopStyleColor(3); // Button, ButtonHovered, ButtonActive
        
        // Fermeture de la fenêtre ImGui
        ImGui::End();
    }
    
    // Pop les styles de frame
    ImGui::PopStyleVar(2); // FrameRounding, FramePadding
}

#include "imgui_internal.h"
bool MainWindow::Loop()
{
    // Main loop
    static bool done = false;


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

    // ------------  Draw all windows
    m_libraryWindow.Draw();
    m_errorListDock.Draw();

    if (m_appController.IsLibraryManagerInitialized())
    {
        bool nodeEditorFocused = m_nodeEditorWindow.IsFocused();
        m_consoleWindow.Draw();
        m_emulatorDock.Draw();
        m_debuggerWindow.Draw();
        m_resourcesDock.Draw();
        m_nodeEditorWindow.Draw();
        m_moduleEditorWindow.Draw();


        auto currentStory = nodeEditorFocused ? m_nodeEditorWindow.GetCurrentStory() : m_moduleEditorWindow.GetCurrentStory();
        m_variablesWindow.Draw(currentStory);
        m_cpuWindow.Draw();

        static MemoryEditor mem_edit_1;
        mem_edit_1.DrawWindow("RAM view", m_appController.GetChip32Context()->ram.mem, m_appController.GetChip32Context()->ram.size);

        auto selectedNode = nodeEditorFocused ? m_nodeEditorWindow.GetSelectedNode() : m_moduleEditorWindow.GetSelectedNode();
        m_propertiesWindow.SetSelectedNode(selectedNode);

        m_propertiesWindow.Draw(currentStory);


        // static ImGuiAxis toolbar2_axis = ImGuiAxis_Y;
        // DockingToolbar("Toolbar2", &toolbar2_axis);

        DrawToolBar(height);
    }

    m_aboutDialog.Draw();
    m_projectPropertiesDialog.Draw();

    m_toastNotifier.render();

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

    return done;
}


void MainWindow::LogEvent(const std::string &txt, bool critical)
{
    m_consoleWindow.AddLog(txt, critical ? 1 : 0);
}


void MainWindow::OpenFunction(const std::string &uuid, const std::string &name)
{
    m_nodeEditorWindow.OpenFunction(uuid, name);
}
