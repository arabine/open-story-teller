#include "main_window.h"
#include <filesystem>
#include <SDL3/SDL.h>
#include "platform_folders.h"

#include "media_converter.h"

#include "pack_archive.h"
#include "uuid.h"

#ifdef USE_WINDOWS_OS
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#include "ImGuiFileDialog.h"

MainWindow::MainWindow()
    : m_libraryManager(*this)
    , m_emulatorWindow(*this)
    , m_resourcesWindow(*this)
    , m_nodeEditorWindow(*this)
    , m_libraryWindow(*this, m_libraryManager)
    , m_player(*this)

{
    // VM Initialize
    m_chip32_ctx.stack_size = 512;

    m_chip32_ctx.rom.mem = m_rom_data;
    m_chip32_ctx.rom.addr = 0;
    m_chip32_ctx.rom.size = sizeof(m_rom_data);

    m_chip32_ctx.ram.mem = m_ram_data;
    m_chip32_ctx.ram.addr = sizeof(m_rom_data);
    m_chip32_ctx.ram.size = sizeof(m_ram_data);

    Callback<uint8_t(chip32_ctx_t *, uint8_t)>::func = std::bind(&MainWindow::Syscall, this, std::placeholders::_1, std::placeholders::_2);
    m_chip32_ctx.syscall = static_cast<syscall_t>(Callback<uint8_t(chip32_ctx_t *, uint8_t)>::callback);

    CloseProject();
}

MainWindow::~MainWindow()
{
    SaveParams();
}


std::string MainWindow::GetFileNameFromMemory(uint32_t addr)
{
    char strBuf[100];
    bool isRam = addr & 0x80000000;
    addr &= 0xFFFF; // mask the RAM/ROM bit, ensure 16-bit addressing
    if (isRam) {
        strcpy(&strBuf[0], (const char *)&m_chip32_ctx.ram.mem[addr]);
    } else {
        strcpy(&strBuf[0], (const char *)&m_chip32_ctx.rom.mem[addr]);
    }
    return strBuf;
}

void MainWindow::Play()
{

    if ((m_dbg.run_result == VM_READY) || (m_dbg.run_result == VM_FINISHED))
    {
        m_dbg.free_run = true;
        m_dbg.run_result = VM_OK; // actually starts the execution
    }

}

void MainWindow::Ok()
{
    m_eventQueue.push({VmEventType::EvOkButton});
}

void MainWindow::Stop()
{
    m_eventQueue.push({VmEventType::EvOkButton});
}

void MainWindow::Pause()
{

}

void MainWindow::Next()
{
    Log("End of audio track");
    m_eventQueue.push({VmEventType::EvNextButton});
}

void MainWindow::Previous()
{
    Log("End of audio track");
    m_eventQueue.push({VmEventType::EvPreviousButton});
}

void MainWindow::EndOfAudio()
{
    Log("End of audio track");
    m_eventQueue.push({VmEventType::EvAudioFinished});
}

void MainWindow::StepInstruction()
{
    m_dbg.run_result = chip32_step(&m_chip32_ctx);
    UpdateVmView();
}

void MainWindow::ProcessStory()
{
    if (m_dbg.run_result == VM_FINISHED)
        return;

    if (m_dbg.run_result == VM_READY)
        return;

    // 1. First, check events
    if (m_dbg.run_result == VM_WAIT_EVENT)
    {
        VmEvent event;
        if (m_eventQueue.try_pop(event))
        {
            if (event.type == VmEventType::EvStep)
            {
                m_dbg.run_result = VM_OK;
            }
            else if (event.type == VmEventType::EvOkButton)
            {
                m_chip32_ctx.registers[R0] = 0x01;
                m_dbg.run_result = VM_OK;
            }
            else if (event.type == VmEventType::EvPreviousButton)
            {
                m_chip32_ctx.registers[R0] = 0x02;
                m_dbg.run_result = VM_OK;
            }
            else if (event.type == VmEventType::EvNextButton)
            {
                m_chip32_ctx.registers[R0] = 0x04;
                m_dbg.run_result = VM_OK;
            }
            else if (event.type == VmEventType::EvAudioFinished)
            {
                m_chip32_ctx.registers[R0] = 0x08;
                m_dbg.run_result = VM_OK;
            }
            else if (event.type == VmEventType::EvStop)
            {
                m_dbg.run_result = VM_FINISHED;
            }
        }
    }

    if (m_dbg.run_result == VM_OK)
    {
        if (m_dbg.m_breakpoints.contains(m_dbg.line + 1))
        {
            Log("Breakpoint on line: " + std::to_string(m_dbg.line + 1));
            m_dbg.free_run = false;
            return;
        }
        StepInstruction();
    }

    if (m_dbg.run_result == VM_FINISHED)
    {
        m_dbg.free_run = false;
    }

    // In this case, we wait for single step debugger
    if ((m_dbg.run_result == VM_OK) && !m_dbg.free_run)
    {
        m_dbg.run_result = VM_WAIT_EVENT;
    }
}

uint8_t MainWindow::Syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;
    Log("SYSCALL: " + std::to_string(code));

    // Media
    if (code == 1) // Execute media
    {
        if (m_chip32_ctx.registers[R0] != 0)
        {
            // image file name address is in R0
            std::string imageFile = m_story->BuildFullAssetsPath(GetFileNameFromMemory(m_chip32_ctx.registers[R0]));
            Log("Image: " + imageFile);
            m_emulatorWindow.SetImage(imageFile);
        }
        else
        {
            m_emulatorWindow.ClearImage();
        }

        if (m_chip32_ctx.registers[R1] != 0)
        {
            // sound file name address is in R1
            std::string soundFile = m_story->BuildFullAssetsPath(GetFileNameFromMemory(m_chip32_ctx.registers[R1]));
            Log(", Sound: " + soundFile);
            m_player.Play(soundFile);
        }
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    // WAIT EVENT bits:
    // 0: block
    // 1: OK button
    // 2: home button
    // 3: pause button
    // 4: rotary left
    // 5: rotary right
    else if (code == 2) // Wait for event
    {
        // Event mask is located in R0
        // optional timeout is located in R1
        // if timeout is set to zero, wait for infinite and beyond
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }


    return retCode;
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
                SaveProject();
            }

            if (ImGui::MenuItem("Close project"))
            {
                CloseProject();
            }

            if (ImGui::MenuItem("Project settings"))
            {
                showParameters = true;
            }

            if (!init)
                ImGui::EndDisabled();

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
        if (m_story)
        {
            ImGui::OpenPopup("ProjectPropertiesPopup");

            // Init some variables
            std::size_t length = m_story->GetName().copy(m_project_name, sizeof(m_project_name));
            m_project_name[length] = '\0';
        }
    }

    if (showNewProject)
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDialog", "Choose a parent directory for your project", nullptr, config);
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImVec2 parent_pos = ImGui::GetWindowPos();
    //ImVec2 parent_size = ImGui::GetWindowSize();
    //ImVec2 center(parent_pos.x + parent_size.x * 0.5f, parent_pos.y + parent_size.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Story Editor - v%s", LibraryManager::GetVersion().c_str());
        ImGui::Text("http://www.openstoryteller.org");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");

        ImGui::Text("%s", SDL_GetPlatform());
        ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
        ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::SameLine(300);
        if (ImGui::Button("Close", ImVec2(100, 35)))
        {
           ImGui::CloseCurrentPopup();
        }
       ImGui::EndPopup();
    }
}

void MainWindow::Initialize()
{
    LoadParams();

    // GUI Init
    m_gui.Initialize();
  //  gui.ApplyTheme();

    m_editorWindow.Initialize();
    m_emulatorWindow.Initialize();
    m_nodeEditorWindow.Initialize();
    m_PropertiesWindow.Initialize();
    m_libraryWindow.Initialize();


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


void MainWindow::NewProjectPopup()
{
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGuiFileDialog::Instance()->Display("ChooseDirDialog"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string projdir = ImGuiFileDialog::Instance()->GetCurrentPath();

            if (!std::filesystem::is_directory(projdir))
            {
                CloseProject();

                m_story = m_libraryManager.NewProject();

                if (m_story)
                {
                    SaveProject();
                    OpenProject(m_story->GetUuid());
                }
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }
}

void MainWindow::ProjectPropertiesPopup()
{
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("ProjectPropertiesPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {

        ImGui::Text("Project name: "); ImGui::SameLine();
        ImGui::InputTextWithHint("##project_name", "Project name", m_project_name, IM_ARRAYSIZE(m_project_name));

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
            if (display_item_current_idx == 0)
            {
                m_story->SetDisplayFormat(320, 240);
            }
            else
            {
                m_story->SetDisplayFormat(640, 480);
            }

            m_story->SetImageFormat(GetImageFormat(image_item_current_idx));
            m_story->SetSoundFormat(GetSoundFormat(sound_item_current_idx));
            m_story->SetName(m_project_name);

            SaveProject();

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
}

void MainWindow::SaveProject()
{
    nlohmann::json model;
    m_story->Save(m_resources);
}

void MainWindow::OpenProject(const std::string &uuid)
{
    CloseProject();
    m_story = m_libraryManager.GetStory(uuid);

    if (!m_story)
    {
        Log("Cannot find story: " + uuid);
    }
    else if (m_story->Load(m_resources))
    {
        Log("Open project success");
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
            SaveParams();
        }

        m_nodeEditorWindow.Enable();
        m_emulatorWindow.Enable();
        m_consoleWindow.Enable();
        m_editorWindow.Enable();
        m_resourcesWindow.Enable();
        m_PropertiesWindow.Enable();
    }
    else
    {
        Log("Open project error");
    }

    RefreshProjectInformation();
}


void MainWindow::ImportProject(const std::string &fileName, int format)
{
    PackArchive archive(*this);

    archive.ImportStudioFormat(fileName, m_libraryManager.LibraryPath());

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


void MainWindow::CloseProject()
{
    if (m_story)
    {
        m_story->Clear();
        m_story.reset();
    }

    m_resources.Clear();

    m_nodeEditorWindow.Clear();
    m_emulatorWindow.ClearImage();
    m_consoleWindow.ClearLog();
    m_editorWindow.ClearErrors();
    m_editorWindow.SetScript("");

    m_nodeEditorWindow.Disable();
    m_emulatorWindow.Disable();
    m_editorWindow.Disable();
    m_resourcesWindow.Disable();
    m_PropertiesWindow.Disable();

    RefreshProjectInformation();
}



void MainWindow::Loop()
{
    // Main loop
    bool done = false;

    while (!done)
    {
        Uint64 frameStart = SDL_GetTicks();
        bool aboutToClose = m_gui.PollEvent();

        m_gui.StartFrame();

       ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        DrawMainMenuBar();
       // DrawStatusBar();


        ProcessStory();


        // ------------  Draw all windows
        m_libraryWindow.Draw();

        if (m_libraryManager.IsInitialized())
        {
            m_consoleWindow.Draw();
            m_emulatorWindow.Draw();
            m_editorWindow.Draw();
            m_resourcesWindow.Draw();
            m_nodeEditorWindow.Draw();


            m_PropertiesWindow.SetSelectedNode(m_nodeEditorWindow.GetSelectedNode());
            m_PropertiesWindow.Draw();
        }

        NewProjectPopup();
        ProjectPropertiesPopup();

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

        if (frameTime < 16) { // Limite de 60 FPS
            SDL_Delay(16 - frameTime); // Attendez pour compléter la frame
        }


    }

    m_gui.Destroy();
}


void MainWindow::Log(const std::string &txt, bool critical)
{
    m_consoleWindow.AddLog(txt, critical ? 1 : 0);
}

void MainWindow::PlaySoundFile(const std::string &fileName)
{
    Log("Play sound file: " + fileName);
    m_player.Play(fileName);
}

std::string MainWindow::BuildFullAssetsPath(const std::string &fileName) const
{
    return m_story->BuildFullAssetsPath(fileName);
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

std::shared_ptr<BaseNode> MainWindow::CreateNode(const std::string &type)
{
    return m_story->CreateNode(type);
}

void MainWindow::Build(bool compileonly)
{
    if (m_story->GenerateScript(m_currentCode))
    {
        m_editorWindow.SetScript(m_currentCode);
        m_dbg.run_result = VM_FINISHED;
        m_dbg.free_run = false;

        if (!compileonly)
        {
            // 3. Convert all media to desired type format
            m_resources.ConvertResources(m_story->AssetsPath(), ""); // pas de répertoire de destination
        }

        Chip32::Assembler::Error err;
        if (m_story->GenerateBinary(m_currentCode, err))
        {
            m_result.Print();

            Log("Binary successfully generated.");

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
                Log("Program too big. Expand ROM memory.");
            }
        }
        else
        {
            Log(err.ToString(), true);
            m_editorWindow.AddError(err.line, err.message); // show also the error in the code editor
        }
    }

}

void MainWindow::DeleteNode(const std::string &id)
{
    m_story->DeleteNode(id);
}

void MainWindow::DeleteLink(std::shared_ptr<Connection> c)
{
    m_story->DeleteLink(c);
}

std::list<std::shared_ptr<Connection>> MainWindow::GetNodeConnections(const std::string &nodeId)
{
    return m_story->GetNodeConnections(nodeId);
}

void MainWindow::UpdateVmView()
{
    // FIXME
//    m_vmDock->updateRegistersView(m_chip32_ctx);

    // Highlight next line in the test editor
    uint32_t pcVal = m_chip32_ctx.registers[PC];

    uint32_t line = 1;
    if (m_story->GetAssemblyLine(pcVal, line))
    {
        m_dbg.line = (line - 1);
        m_editorWindow.HighlightLine(m_dbg.line);
    }
    else
    {
        // Not found
        Log("Reached end or instruction not found line: " + std::to_string(m_dbg.line));
    }
    // Refresh RAM content
//    m_ramView->SetMemory(m_ram_data, m_chip32_ctx.ram.size);
}


void MainWindow::SaveParams()
{
    nlohmann::json j;
    nlohmann::json recents(m_recentProjects);

    j["recents"] = recents;
    j["library_path"] = m_libraryManager.LibraryPath();
    j["store_url"] = m_libraryManager.GetStoreUrl();

    std::string loc = pf::getConfigHome() + "/ost_settings.json";
    std::ofstream o(loc);
    o << std::setw(4) << j << std::endl;

    Log("Saved settings to: " + loc);
}

void MainWindow::LoadParams()
{
    try {
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

        nlohmann::json store_url = j.value("store_url", "https://gist.githubusercontent.com/DantSu/8920929530b58f2acbfcf1ed0e7746f9/raw/stories-contrib.json");
        m_libraryManager.SetStoreUrl(store_url);

    }
    catch(std::exception &e)
    {

    }
}
