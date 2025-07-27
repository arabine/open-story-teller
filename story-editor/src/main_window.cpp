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

MainWindow::MainWindow()
    : m_resources(*this)
    , m_nodesFactory(*this)
    , m_libraryManager(*this, m_nodesFactory)
    , m_emulatorWindow(*this)
    , m_debuggerWindow(*this)
    , m_cpuWindow(*this)
    , m_resourcesWindow(*this)
    , m_nodeEditorWindow(*this, m_nodesFactory)
    , m_moduleEditorWindow(*this, m_nodesFactory, IStoryProject::Type::PROJECT_TYPE_MODULE)
    , m_libraryWindow(*this, m_libraryManager, m_nodesFactory)
    , m_variablesWindow(*this)
    , m_player(*this)
    , m_webServer(m_libraryManager)
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
    CloseModule();

    // define style for all directories
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_MDI_FOLDER);
    // define style for all files
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ICON_MDI_FILE);

}

MainWindow::~MainWindow()
{
    SaveParams();
}


std::string MainWindow::GetStringFromMemory(uint32_t addr)
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

void MainWindow::Step()
{
    m_eventQueue.push({VmEventType::EvStep});
}

void MainWindow::Run()
{
    m_eventQueue.push({VmEventType::EvRun});
}

void MainWindow::Ok()
{
    m_eventQueue.push({VmEventType::EvOkButton});
}


void MainWindow::Stop()
{
    m_dbg.run_result = VM_FINISHED; // better than sending an event: avoid infinite loops in assembly
}

void MainWindow::Pause()
{

}

void MainWindow::Home()
{
    m_eventQueue.push({VmEventType::EvHomeButton});
}

void MainWindow::Next()
{
    Log("Next button");
    m_eventQueue.push({VmEventType::EvNextButton});
}

void MainWindow::Previous()
{
    Log("Previous button");
    m_eventQueue.push({VmEventType::EvPreviousButton});
}

std::string MainWindow::VmState() const
{
    std::string state = "Unknown";

    switch (m_dbg.run_result)
    {
        case VM_READY:
            state = "VM Ready";
            break;
        case VM_FINISHED:
            state = "VM Finished";
            break;
        case VM_SKIPED:
            state = "VM Skiped";
            break;
        case VM_WAIT_EVENT:
            state = "VM Wait Event";
            break;
        case VM_OK:
            state = "VM Ok";
            break;
        default:
            state = "VM Error";
            break;
    }
    return state;
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

    // Error states
    if (m_dbg.run_result > VM_OK)
        return;

    // Check events

    VmEvent event;
    if (m_eventQueue.try_pop(event))
    {
        if (event.type == VmEventType::EvStep)
        {
            StepInstruction();
            m_dbg.run_result = VM_OK; // FIXME: bizarre d'écraser le code de retour...
        }
        else if (event.type == VmEventType::EvRun)
        {
            m_dbg.free_run = true;
            m_dbg.run_result = VM_OK;
        }
        else if (event.type == VmEventType::EvStop)
        {
            m_dbg.run_result = VM_FINISHED;
        }

        // Events managed only if the code is in wait event state
        if (m_dbg.run_result == VM_WAIT_EVENT)
        {

            if (event.type == VmEventType::EvOkButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_OK_BUTTON))
                {
                    m_chip32_ctx.registers[R0] = EV_MASK_OK_BUTTON;
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvPreviousButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_PREVIOUS_BUTTON))
                {
                    m_chip32_ctx.registers[R0] = EV_MASK_PREVIOUS_BUTTON;
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvNextButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_NEXT_BUTTON))
                {
                    m_chip32_ctx.registers[R0] = EV_MASK_NEXT_BUTTON;
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvAudioFinished)
            {
                if (m_dbg.IsValidEvent(EV_MASK_END_OF_AUDIO))
                {
                    m_chip32_ctx.registers[R0] = EV_MASK_END_OF_AUDIO;
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvHomeButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_HOME_BUTTON))
                {
                    m_chip32_ctx.registers[R0] = EV_MASK_HOME_BUTTON;
                    m_dbg.run_result = VM_OK;
                }
            }
        }
    }

    if (m_dbg.run_result == VM_OK)
    {
        if (m_dbg.m_breakpoints.contains(m_dbg.line))
        {
          //  Log("Breakpoint on line: " + std::to_string(m_dbg.line + 1));
            m_dbg.run_result = VM_WAIT_EVENT;
            m_dbg.free_run = false;
        }

        if (m_dbg.free_run)
        {
            StepInstruction();
        }
    }

    if (m_dbg.run_result == VM_FINISHED)
    {
        m_dbg.free_run = false;
    }
    else if (m_dbg.run_result > VM_OK)
    {
        std::string error = "VM Error: ";
        switch (m_dbg.run_result)
        {
            case VM_ERR_STACK_OVERFLOW:
                error += "Stack overflow";
                break;
            case VM_ERR_STACK_UNDERFLOW:
                error += "Stack underflow";
                break;
            case VM_ERR_INVALID_ADDRESS:
                error += "Invalid address";
                break;
            case VM_ERR_UNSUPPORTED_OPCODE:
                error += "Invalid address";
                break;
            case VM_ERR_UNKNOWN_OPCODE:
                error += "Unknown opcode";
                break;
            case VM_ERR_UNHANDLED_INTERRUPT:
                error += "Unhandled interrupt";
                break;
            case VM_ERR_INVALID_REGISTER:
                error += "Invalid register";
                break;
            default:
                error += "Unknown error";
                break;
        }
        error += " (line: " + std::to_string(m_dbg.line) + ")";
        Log(error, true);
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
            std::string imageFile = m_story->BuildFullAssetsPath(GetStringFromMemory(m_chip32_ctx.registers[R0]));
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
            std::string soundFile = m_story->BuildFullAssetsPath(GetStringFromMemory(m_chip32_ctx.registers[R1]));
            Log("Sound: " + soundFile);
            m_player.Play(soundFile);
        }
        retCode = SYSCALL_RET_OK; // We continue execution, script must wait for event if necessary (end of audio)
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
        // Empty event queue
        m_eventQueue.clear();

        // Event mask is located in R0
        m_dbg.event_mask = m_chip32_ctx.registers[R0];
        // optional timeout is located in R1
        // if timeout is set to zero, wait for infinite and beyond
        retCode = SYSCALL_RET_WAIT_EV; // set the VM in pause
    }
    else if (code == 3)
    {
        // FIXME
    }
    else         // Printf
    if (code == 4)
    {
        // In R0: string with escaped characters
        // R1: Number of arguments
        // R2, R3 ... arguments

        // Integers: stored in registers by values
        // Strings: first character address in register

        std::string text = GetStringFromMemory(ctx->registers[R0]);

        int arg_count = ctx->registers[R1];
        char working_buf[200] = {0};

        switch(arg_count){
            case 0: 
                Log(text);
                break;
            case 1: 
                snprintf(working_buf, sizeof(working_buf), text.c_str(), ctx->registers[R2]);
                Log(working_buf);
                break;
            case 2: 
                snprintf(working_buf, sizeof(working_buf), text.c_str(), ctx->registers[R2], ctx->registers[R3]);
                Log(working_buf);
                break;
            case 3: 
                snprintf(working_buf, sizeof(working_buf), text.c_str(), ctx->registers[R2], ctx->registers[R3], ctx->registers[R4]);
                Log(working_buf);
                break;
            default:
                break;
        }
    
    }
    // WAIT (sleep)
    else if (code == 5)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ctx->registers[R0]));
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

float MainWindow::DrawMainMenuBar()
{
    bool showParameters = false;
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

                m_story = m_libraryManager.NewProject();

                if (m_story)
                {
                    SaveProject();
                    m_libraryManager.Scan(); // Add new project to library
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

    return height;
}

bool MainWindow::Initialize()
{
    bool success = false;
    LoadParams();

    m_nodesFactory.ScanModules();

    // GUI Init
    if (m_gui.Initialize())
    {
        m_player.Initialize(); // Initialize audio after GUI (uses SDL)
    //  gui.ApplyTheme();

        m_debuggerWindow.Initialize();
        m_emulatorWindow.Initialize();
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
            const char* image_items[] = { "Native (no conversion)", "QOIF (Quite Ok Image Format" };
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
            const char* sound_items[] = { "Native (no conversion)", "WAV (16-bit stereo)", "QOAF (Quite Ok Audio Format" };
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


        ImGui::AlignTextToFramePadding();
        ImGui::Text("Image");
        ImGui::SameLine();

        ImGui::Text("%s", m_story->GetTitleImage().c_str());

        ImGui::SameLine();

        static bool isImage = true;
        if (ImGui::Button("Select...##image")) {
            ImGui::OpenPopup("popup_button");
            isImage = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CLOSE_BOX_OUTLINE "##delimage")) {
            m_story->SetTitleImage("");
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Sound");
        ImGui::SameLine();

        ImGui::Text("%s", m_story->GetTitleSound().c_str());

        ImGui::SameLine();

        if (ImGui::Button("Play " ICON_MDI_PLAY))
        {
            PlaySoundFile(BuildFullAssetsPath(m_story->GetTitleSound()));
        }

        ImGui::SameLine();

        if (ImGui::Button("Select...##sound")) {
            ImGui::OpenPopup("popup_button");
            isImage = false;
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CLOSE_BOX_OUTLINE "##delsound")) {
            m_story->SetTitleSound("");
        }


        if (ImGui::BeginPopup("popup_button")) {
            ImGui::SeparatorText(isImage ? "Images" : "Sounds");

            auto [filtreDebut, filtreFin] = isImage ? Images() : Sounds();
            int n = 0;
            for (auto it = filtreDebut; it != filtreFin; ++it, n++)
            {
                if (ImGui::Selectable((*it)->file.c_str()))
                {
                    if (isImage)
                    {
                        m_story->SetTitleImage((*it)->file);
                    }
                    else
                    {
                        m_story->SetTitleSound((*it)->file);
                    }
                }
            }

            ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
        }


        auto GetImageFormat = [](int idx) -> Resource::ImageFormat
        {
            Resource::ImageFormat img{Resource::IMG_SAME_FORMAT};
            if (idx < Resource::IMG_FORMAT_COUNT) {
                img = static_cast<Resource::ImageFormat>(idx);
            }
            return img;
        };

        auto GetSoundFormat = [](int idx) -> Resource::SoundFormat {

            Resource::SoundFormat img{Resource::SND_FORMAT_WAV};
            if (idx < Resource::IMG_FORMAT_COUNT) {
                img = static_cast<Resource::SoundFormat>(idx);
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
    Log("Project saved");
}

void MainWindow::OpenProject(const std::string &uuid)
{
    CloseProject();

    m_story = m_libraryManager.GetStory(uuid);

    // DEBUG CODE !!!!!!!!!!!!!  Permet si décommenter de forcer l'import, permet de tester plus facilement l'algo en ouvrant le projet
    // PackArchive arch(*this);
    // std::string basePath = m_libraryManager.LibraryPath() + "/" + uuid;
    // arch.ConvertJsonStudioToOst(basePath, uuid, m_libraryManager.LibraryPath());

    if (!m_story)
    {
        Log("Cannot find story: " + uuid);
    }
    else if (m_story->Load(m_resources, m_nodesFactory))
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
        m_debuggerWindow.Enable();
        m_resourcesWindow.Enable();
        m_PropertiesWindow.Enable();
        m_variablesWindow.Enable();
        m_cpuWindow.Enable();
    }
    else
    {
        Log("Open project error");
    }

    RefreshProjectInformation();
}

void MainWindow::NewModule()
{
    auto module = m_nodesFactory.NewModule();
    m_moduleEditorWindow.Load(module);

    m_moduleEditorWindow.Enable();
}


void MainWindow::SaveModule()
{
    m_nodesFactory.SaveAllModules(m_resources);;
    Log("Modules saved");
}

void MainWindow::OpenModule(const std::string &uuid)
{
    m_module = m_nodesFactory.GetModule(uuid);
    if (!m_module)
    {
        Log("Cannot find module: " + uuid);
    }
    else if (m_module->Load(m_resources, m_nodesFactory))
    {
        Log("Open module success");
        m_moduleEditorWindow.Load(m_module);
    }
    else
    {
        Log("Open module error");
    }


    m_moduleEditorWindow.Enable();
}



void MainWindow::CloseProject()
{
    // FIXME: not sure but if present, we lost some information in the library manager

    // if (m_story)
    // {
    //     m_story->Clear();
    //     m_story.reset();
    // }

    m_resources.Clear();
    m_nodeEditorWindow.Clear();
    m_nodeEditorWindow.Disable();

    m_emulatorWindow.ClearImage();
    m_consoleWindow.ClearLog();
    m_debuggerWindow.ClearErrors();
    m_debuggerWindow.SetScript("");   
    
    m_emulatorWindow.Disable();
    m_debuggerWindow.Disable();
    m_resourcesWindow.Disable();
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
        Log("Unknown file format: " + filePathName);
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

       // DrawStatusBar();

        ProcessStory();

        // ------------  Draw all windows
        m_libraryWindow.Draw();

        if (m_libraryManager.IsInitialized())
        {
            m_consoleWindow.Draw();
            m_emulatorWindow.Draw();
            m_debuggerWindow.Draw();
            m_resourcesWindow.Draw();
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

        if (frameTime < 32) { // 16 ms =  60 FPS
            SDL_Delay(32 - frameTime); // Attendez pour compléter la frame
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

std::string MainWindow::BuildFullAssetsPath(const std::string_view fileName) const
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

void MainWindow::OpenFunction(const std::string &uuid, const std::string &name)
{
    m_nodeEditorWindow.OpenFunction(uuid, name);
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

void MainWindow::LoadBinaryStory(const std::string &filename)
{
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp != NULL)
    {
        fseek(fp, 0L, SEEK_END);
        long int sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (sz <= m_chip32_ctx.rom.size)
        {
            size_t sizeRead = fread(m_chip32_ctx.rom.mem, sz, 1, fp);
            if (sizeRead == sz)
            {
                m_dbg.run_result = VM_READY;
                chip32_initialize(&m_chip32_ctx);
                Log("Loaded binary file: " + filename);
            }
            else
            {
                Log("Failed to load binary file", true);
            }
            
        }
        fclose(fp);
    }
}

void MainWindow::ToggleBreakpoint(int line)
{
    if (m_dbg.m_breakpoints.contains(line))
    {
        m_dbg.m_breakpoints.erase(line);
    }
    else
    {
        m_dbg.m_breakpoints.insert(line);
    }
}

uint32_t MainWindow::GetRegister(int reg)
{
    uint32_t regVal = 0;

    if (reg >= 0 && reg < REGISTER_COUNT)
    {
        regVal = m_chip32_ctx.registers[reg];
    }

    return regVal;
}

void MainWindow::ScanVariable(const std::function<void(std::shared_ptr<Variable> element)>& operation)
{
    if (m_story)
    {
        m_story->ScanVariable(operation);
    }
}

void MainWindow::AddVariable() 
{
    m_story->AddVariable();
}

void MainWindow::DeleteVariable(int i)
{
    m_story->DeleteVariable(i);
}

void MainWindow::BuildNodes(bool compileonly)
{
    if (m_story->GenerateScript(m_currentCode))
    {
        m_debuggerWindow.SetScript(m_currentCode);
        Build(compileonly);
    }
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
            Log("Program too big. Expand ROM memory.");
        }
    }
    else
    {
        Log(err.ToString(), true);
        m_debuggerWindow.AddError(err.line, err.message); // show also the error in the code editor
    }
}


void MainWindow::BuildCode(bool compileonly)
{
    m_currentCode = SysLib::ReadFile(m_externalSourceFileName);
    m_debuggerWindow.SetScript(m_currentCode);
    Build(compileonly);
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
        Log(e.what(), true);
    }
}
