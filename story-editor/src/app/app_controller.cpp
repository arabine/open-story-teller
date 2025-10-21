// app_controller.cpp
#include "app_controller.h"

// Inclure les entêtes des dépendances de AppController nécessaires pour l'implémentation
#include <filesystem>
#include <fstream>
#include <chrono>   // Pour std::this_thread::sleep_for
#include <thread>   // Pour std::this_thread
#include <algorithm> // Pour std::find

#include "platform_folders.h" // Pour pf::getConfigHome()
#include "uuid.h"           // Pour la génération d'UUID
#include "sys_lib.h"        // Pour SysLib::ReadFile, GetFileExtension, GetFileName
#include "pack_archive.h"   // Pour PackArchive
#include "json.hpp"
#include "variable.h"       // Pour Variable
#include "all_events.h"
#include "Localization.h"
#include "chip32_machine.h"

AppController::AppController(ILogger& logger, EventBus& eventBus)
    : m_logger(logger)
    , m_eventBus(eventBus)
    , m_resources(logger)
    , m_nodesFactory(logger)
    , m_libraryManager(logger, m_nodesFactory)
    , m_player(*this)
    , m_webServer(m_libraryManager)
{

    Callback<uint8_t(chip32_ctx_t *, uint8_t)>::func = std::bind(&AppController::Syscall, this, std::placeholders::_1, std::placeholders::_2);
    m_machine.ctx.syscall = static_cast<syscall_t>(Callback<uint8_t(chip32_ctx_t *, uint8_t)>::callback);

    // Assurez-vous que ces fonctions existent ou sont implémentées ailleurs
    // CloseProject() et CloseModule() étaient dans MainWindow
    // Si elles gèrent des états VM/projet, elles doivent être ici.
    CloseProject();
    CloseModule();
}

AppController::~AppController()
{
    SaveParams(); // Sauvegarde les paramètres à la destruction
}

bool AppController::Initialize()
{
    LoadParams();
    m_nodesFactory.ScanModules();
    m_player.Initialize(); // Initialise l'audio
    return true; // Supposons que l'initialisation de AppController est toujours un succès simple
}

// --- Implémentations de IStoryManager ---

void AppController::OpenProject(const std::string &uuid)
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
        m_eventBus.Emit(std::make_shared<OpenProjectEvent>(uuid));
    }
}


void AppController::Log(const std::string &txt, bool critical)
{
    m_logger.Log(txt, critical);
}

void AppController::PlaySoundFile(const std::string &fileName)
{
    m_player.Play(fileName);
}

std::string AppController::BuildFullAssetsPath(const std::string_view fileName) const
{
    if (m_story)
    {
        return m_story->BuildFullAssetsPath(fileName);
    }
    return std::string();
}

void AppController::OpenFunction(const std::string &uuid, const std::string &name)
{
    m_eventBus.Emit(std::make_shared<OpenFunctionEvent>(uuid, name));
}

void AppController::NewStory()
{
    CloseProject();
    m_story = m_libraryManager.NewProject();
    if (m_story)
    {
        SaveStory(); // Sauvegarde le nouveau projet
        m_libraryManager.Scan(); // Ajoute le nouveau projet à la bibliothèque
        OpenStory(m_story->GetUuid()); // Ouvre le nouveau projet
    }
}

void AppController::OpenStory(const std::string &uuid_or_path)
{
    CloseProject();

    // Tente d'abord d'ouvrir par UUID via LibraryManager
    m_story = m_libraryManager.GetStory(uuid_or_path);

    // Si non trouvé par UUID, essaie d'ouvrir par chemin direct (si uuid_or_path est un chemin)
    if (!m_story && std::filesystem::exists(uuid_or_path)) {
        // Crée un StoryProject à partir d'un chemin existant si nécessaire
        // Cette logique est plus complexe et dépend de comment votre StoryProject est construit
        // For now, assume GetStory can handle paths if it's not a UUID
        // Or you might need a dedicated method like m_libraryManager.GetStoryByPath()
        // For simplicity, if it's a path, try to find its UUID or create temp StoryProject
        // This part needs careful design based on your LibraryManager/StoryProject logic.
        // For this example, we assume GetStory() attempts to resolve paths if it's not a UUID.
        m_logger.Log("Attempting to open story by path (not UUID): " + uuid_or_path);
        // A more robust solution would involve LibraryManager::GetStory(path) or similar.
        // For now, if GetStory(uuid) failed and it's a path, we might assume it's a direct project path.
        // This is a simplification and might need adjustment based on your actual library logic.
        // If your StoryProject can be loaded directly from a path:
        // m_story = std::make_shared<StoryProject>(uuid_or_path); // Example, constructor might need more args
        // m_story->Load(m_resources, m_nodesFactory);
        // Or, more likely, LibraryManager needs to handle this.
    }


    if (!m_story)
    {
        m_logger.Log("Cannot find story: " + uuid_or_path, true);
    }
    else if (m_story->Load(m_resources, m_nodesFactory))
    {
        m_logger.Log("Open project success: " + m_story->GetProjectFilePath());

        // Add to recent if not exists
        const auto& proj_path = m_story->GetProjectFilePath();
        AddRecentProject(proj_path);

        // Notify GUI (or specific windows) to enable/load project data
        // These calls would be replaced by events or direct calls to GUI objects in MainWindow
        // For now, we'll keep the conceptual calls that AppController would trigger:
        // m_nodeEditorWindow.Load(m_story); // This logic belongs to GUI, AppController notifies
        // m_nodeEditorWindow.Enable();     // This logic belongs to GUI
        // etc.
        UpdateVmView(); // To refresh debugger/CPU views
    }
    else
    {
        m_logger.Log("Open project error for: " + uuid_or_path, true);
    }
}

void AppController::SaveStory(const std::string &path)
{
    if (m_story)
    {
        m_story->Save(m_resources);
        m_logger.Log("Project saved: " + m_story->GetProjectFilePath());
    } else {
        m_logger.Log("No project open to save.", true);
    }
}

void AppController::ExportStory(const std::string &filename)
{
    // Logique d'exportation de l'histoire
    // Cette partie est manquante dans le snippet original, mais devrait être ici.
    m_logger.Log("Export story to: " + filename);
}

void AppController::CompileNodes(IStoryProject::Type type)
{
    if (type == IStoryProject::Type::PROJECT_TYPE_STORY && m_story)
    {
        if (m_story->GenerateCompleteProgram(m_storyAssembly))
        {
            m_logger.Log("Nodes script generated for story.");
            Build(true); // Compile seulement par défaut
        }
        else
        {
            m_logger.Log("Failed to generate script for story.", true);
        }
    } 
    else if (type == IStoryProject::Type::PROJECT_TYPE_MODULE && m_module)
    {
        if (m_module->GenerateCompleteProgram(m_moduleAssembly))
        {
            m_logger.Log("Nodes script generated for module.");
            BuildModule(true);
        }
        else
        {
            m_logger.Log("Failed to generate script for module.", true);
            // if (m_errorListDock) {
            //     m_errorListDock->AddError("Failed to generate assembly code from nodes");
            // }
            m_eventBus.Emit(std::make_shared<ModuleEvent>(ModuleEvent::Type::BuildFailure, m_module->GetUuid()));
        }
    }
}

void AppController::Build(bool compileonly)
{
    m_dbg.run_result = VM_FINISHED;
    m_dbg.free_run = false;
    m_dbg.m_breakpoints.clear(); // Clear breakpoints on new build

    if (!m_story) {
        m_logger.Log("No story loaded to build.", true);
        return;
    }

    if (!compileonly)
    {
        // Convert all media to desired type format
        auto options = m_story->GetOptions();
        m_resources.ConvertResources(m_story->AssetsPath(), "", options.image_format, options.sound_format);
    }


    if (m_machine.Build(m_storyAssembly))
    {
        m_machine.SaveBinary(m_story->BinaryFileName());
        m_dbg.run_result = VM_READY;
        UpdateVmView(); // Notifie la GUI de mettre à jour la vue VM
        m_logger.Log("Build successful. VM ready.");

         m_eventBus.Emit(std::make_shared<GenericResultEvent>(false, "Build success"));
    }
    else
    {
        auto err = m_machine.assembler.GetLastError(); // FIXME: l'erreur ne vient pas uniquement de l'assembleur, enfouir cela et remonter d'autres erreurs liées à la machine 
        m_eventBus.Emit(std::make_shared<GenericResultEvent>(false, "Build error: " + err.ToString()));
    }
}


void AppController::BuildModule(bool compileonly)
{
    if (!m_module) {
        m_logger.Log("No module loaded to build.", true);
        return;
    }

    // Le code du module est déjà complet avec .main: et halt
    // Pas besoin de wrapper !
    
    m_logger.Log("=== Module Assembly Code ===");
    m_logger.Log(m_moduleAssembly);
    m_logger.Log("============================");

    if (m_machine.Build(m_moduleAssembly))
    {
        m_logger.Log("Module compiled successfully!");
        
        // Save the binary to disk
        m_machine.SaveBinary(m_module->BinaryFileName());
        m_dbg.run_result = VM_READY;
        UpdateVmView(); // Notifie la GUI de mettre à jour la vue VM
        m_eventBus.Emit(std::make_shared<ModuleEvent>(ModuleEvent::Type::BuildSuccess, m_module->GetUuid()));
    }
    else
    {
        // Compilation failed - show error
        auto err = m_machine.assembler.GetLastError(); // FIXME: l'erreur ne vient pas uniquement de l'assembleur, enfouir cela et remonter d'autres erreurs liées à la machine 
        
        std::string errorMsg = err.ToString();
        auto errObj = std::make_shared<ModuleEvent>(ModuleEvent::Type::BuildFailure, m_module->GetUuid());
        errObj->SetFailure(errorMsg, err.line);
        m_eventBus.Emit(errObj);
    }
}


void AppController::BuildCode(std::shared_ptr<StoryProject> story, bool compileonly, bool force)
{
    // Note: Dans le code original, BuildCode lisait m_externalSourceFileName.
    // Il faut s'assurer que m_storyAssembly est bien défini avant d'appeler Build.
    if (story) {
        m_storyAssembly = SysLib::ReadFile(story->GetProjectFilePath()); // Simplifié pour l'exemple
        // La GUI (DebuggerWindow) doit être notifiée pour SetScript.
        // m_debuggerWindow.SetScript(m_storyAssembly);
        Build(compileonly);
    } else {
        m_logger.Log("No story provided for BuildCode.", true);
    }
}


void AppController::BuildCode(bool compileonly)
{
    m_storyAssembly = SysLib::ReadFile(m_externalSourceFileName);
    // m_debuggerWindow.SetScript(m_storyAssembly); // FIXME: GUI event
    Build(compileonly);
}

void AppController::SetExternalSourceFile(const std::string &filename)
{
    m_externalSourceFileName = filename;
    m_logger.Log("External source file set to: " + filename);
}

void AppController::LoadBinaryStory(const std::string &filename)
{

    if (m_machine.LoadBinary(filename))
    {
        m_dbg.run_result = VM_READY;
        m_logger.Log("Loaded binary file: " + filename);
        UpdateVmView();
    }
    else
    {
        m_logger.Log("Failed to open binary file: " + filename, true);
    }
}

void AppController::ToggleBreakpoint(int line)
{
    if (m_dbg.m_breakpoints.contains(line))
    {
        m_dbg.m_breakpoints.erase(line);
        m_logger.Log("Removed breakpoint at line: " + std::to_string(line + 1));
    }
    else
    {
        m_dbg.m_breakpoints.insert(line);
        m_logger.Log("Set breakpoint at line: " + std::to_string(line + 1));
    }
    // Notify debugger window to update breakpoint display
}

uint32_t AppController::GetRegister(int reg)
{
    uint32_t regVal = 0;
    if (reg >= 0 && reg < REGISTER_COUNT) // Assurez-vous que REGISTER_COUNT est défini
    {
        regVal = m_machine.ctx.registers[reg];
    }
    return regVal;
}


void AppController::Play()
{
    if ((m_dbg.run_result == VM_READY) || (m_dbg.run_result == VM_FINISHED))
    {
        m_dbg.free_run = true;
        m_dbg.run_result = VM_OK; // actually starts the execution
        m_logger.Log("VM: Play initiated.");
    }
}

void AppController::Step()
{
    m_eventQueue.push({VmEventType::EvStep});
    m_logger.Log("VM: Step event queued.");
}

void AppController::Run()
{
    m_eventQueue.push({VmEventType::EvRun});
    m_logger.Log("VM: Run event queued.");
}

void AppController::Ok()
{
    m_eventQueue.push({VmEventType::EvOkButton});
    m_logger.Log("VM: OK button event queued.");
}

void AppController::Stop()
{
    m_dbg.run_result = VM_FINISHED;
    m_dbg.free_run = false;
    m_logger.Log("VM: Stopped.");
}

void AppController::Pause()
{
    // Logic for pausing VM (if VM supports pausing execution mid-instruction)
    // For now, if free_run is false and VM_OK, it effectively waits for event
    m_dbg.free_run = false;
    m_logger.Log("VM: Pause requested.");
}

void AppController::Home()
{
    m_eventQueue.push({VmEventType::EvHomeButton});
    m_logger.Log("VM: Home button event queued.");
}

void AppController::Next()
{
    m_logger.Log("VM: Next button event queued.");
    m_eventQueue.push({VmEventType::EvNextButton});
}

void AppController::Previous()
{
    m_logger.Log("VM: Previous button event queued.");
    m_eventQueue.push({VmEventType::EvPreviousButton});
}

std::string AppController::VmState() const
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
            state = "VM Skipped"; // Typo in original, corrected to Skipped
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

// --- Implémentations de IAudioEvent ---

void AppController::EndOfAudio()
{
    m_logger.Log("End of audio track. Queuing event for VM.");
    m_eventQueue.push({VmEventType::EvAudioFinished});
}

// --- Fonctions internes de AppController (logique métier) ---

void AppController::SaveParams()
{
    nlohmann::json j;
    j["recents"] = m_recentProjects;
    j["library_path"] = m_libraryManager.LibraryPath();
    j["store_url"] = m_libraryManager.GetStoreUrl();
    j["language"] = Localization::Instance().GetCurrentLang();

    std::string loc = pf::getConfigHome() + "/ost_settings.json";
    std::ofstream o(loc);
    o << std::setw(4) << j << std::endl;

    m_logger.Log("Saved settings to: " + loc);
}

void AppController::LoadParams()
{
    try {
        std::filesystem::path dlDir = std::filesystem::path(pf::getConfigHome()) / "ost_modules";
        std::filesystem::create_directories(dlDir);
        m_nodesFactory.SetModulesRootDirectory(dlDir.string());

        std::string loc = pf::getConfigHome() + "/ost_settings.json";
        std::ifstream i(loc);
        nlohmann::json j;
        i >> j;

        if (j.contains("recents")) {
            for (auto& element : j["recents"]) {
                std::string path_str = element.get<std::string>();
                if (std::filesystem::exists(path_str))
                {
                    m_recentProjects.push_back(path_str);
                }
            }
        }

        if (j.contains("library_path")) {
            std::string lib_path = j["library_path"].get<std::string>();
            if (std::filesystem::exists(lib_path))
            {
                m_libraryManager.Initialize(lib_path);
            }
        }

        if (j.contains("store_url")) {
            m_libraryManager.SetStoreUrl(j.value("store_url", "https://gist.githubusercontent.com/DantSu/3aea4c1fe15070bcf394a40b89aec33e/raw/stories.json"));
        } else {
            m_logger.Log("No 'store_url' found in settings, using default.", false);
        }

        if (j.contains("language")) {
            std::string lang = j["language"].get<std::string>();
            Localization::Instance().LoadLanguage(lang);
        }

    }
    catch(const std::exception &e)
    {
        m_logger.Log("Error loading settings: " + std::string(e.what()), true);
    }
}


void AppController::ProcessStory()
{
    if (m_dbg.run_result == VM_FINISHED || m_dbg.run_result > VM_OK)
        return;

    VmEvent event;
    if (m_eventQueue.try_pop(event))
    {
        if (event.type == VmEventType::EvStep)
        {
            StepInstruction();
            m_dbg.run_result = VM_OK; // Correction : ceci écrase le code de retour, potentiellement indésirable
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
                    m_machine.SetEvent(EV_MASK_OK_BUTTON);
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvPreviousButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_PREVIOUS_BUTTON))
                {
                    m_machine.SetEvent(EV_MASK_PREVIOUS_BUTTON);
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvNextButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_NEXT_BUTTON))
                {
                    m_machine.SetEvent(EV_MASK_NEXT_BUTTON);
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvAudioFinished)
            {
                if (m_dbg.IsValidEvent(EV_MASK_END_OF_AUDIO))
                {
                    m_machine.SetEvent(EV_MASK_END_OF_AUDIO);
                    m_dbg.run_result = VM_OK;
                }
            }
            else if (event.type == VmEventType::EvHomeButton)
            {
                if (m_dbg.IsValidEvent(EV_MASK_HOME_BUTTON))
                {
                    m_machine.SetEvent(EV_MASK_HOME_BUTTON);
                    m_dbg.run_result = VM_OK;
                }
            }
        }
    }

    if (m_dbg.run_result == VM_OK)
    {
        if (m_dbg.m_breakpoints.contains(m_dbg.line))
        {
            m_logger.Log("Breakpoint hit on line: " + std::to_string(m_dbg.line + 1));
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
            case VM_ERR_STACK_OVERFLOW: error += "Stack overflow"; break;
            case VM_ERR_STACK_UNDERFLOW: error += "Stack underflow"; break;
            case VM_ERR_INVALID_ADDRESS: error += "Invalid address"; break;
            case VM_ERR_UNSUPPORTED_OPCODE: error += "Unsupported opcode"; break;
            case VM_ERR_UNKNOWN_OPCODE: error += "Unknown opcode"; break;
            case VM_ERR_UNHANDLED_INTERRUPT: error += "Unhandled interrupt"; break;
            case VM_ERR_INVALID_REGISTER: error += "Invalid register"; break;
            default: error += "Unknown error"; break;
        }
        error += " (line: " + std::to_string(m_dbg.line) + ")";
        m_logger.Log(error, true);
    }

    // In this case, we wait for single step debugger
    if ((m_dbg.run_result == VM_OK) && !m_dbg.free_run)
    {
        m_dbg.run_result = VM_WAIT_EVENT;
    }
}

void AppController::StepInstruction()
{
    m_dbg.run_result = chip32_step(&m_machine.ctx);
    UpdateVmView();
}


uint8_t AppController::Syscall(chip32_ctx_t *ctx, uint8_t code)
{
    uint8_t retCode = SYSCALL_RET_OK;
    m_logger.Log("SYSCALL: " + std::to_string(code));

    // Media
    if (code == 1) // Execute media
    {
        // R0: image file name address, R1: sound file name address
        if (ctx->registers[R0] != 0)
        {
            std::string imageFile = m_story->BuildFullAssetsPath(Chip32::Machine::GetStringFromMemory(ctx, ctx->registers[R0]));
            m_logger.Log("Image: " + imageFile);
            // Ici, vous notifieriez la fenêtre de l'émulateur
            // m_emulatorDock.SetImage(imageFile); // Ceci est une dépendance GUI
        }
        else
        {
            // m_emulatorDock.ClearImage(); // Dépendance GUI
        }

        if (ctx->registers[R1] != 0)
        {
            std::string soundFile = m_story->BuildFullAssetsPath(Chip32::Machine::GetStringFromMemory(ctx, ctx->registers[R1]));
            m_logger.Log("Sound: " + soundFile);
            m_player.Play(soundFile);
        }
        retCode = SYSCALL_RET_OK;
    }
    else if (code == 2) // Wait for event
    {
        m_eventQueue.clear();
        m_dbg.event_mask = ctx->registers[R0];
        // optional timeout is located in R1
        retCode = SYSCALL_RET_WAIT_EV;
    }
    else if (code == 3)
    {
        // FIXME: Unknown syscall 3, log it for debugging
        m_logger.Log("Syscall 3 called (FIXME)", false);
    }
    else if (code == 4) // Printf (printf-like behavior)
    {
        std::string format = Chip32::Machine::GetStringFromMemory(ctx, ctx->registers[R0]);
        int arg_count = ctx->registers[R1];
        
        std::vector<uint32_t> args;
        for (int i = 0; i < arg_count && i < 4; ++i) {
            args.push_back(ctx->registers[R2 + i]);
        }

        // Send event to UI
        auto evObj = std::make_shared<VmStateEvent>();
        evObj->type = VmStateEvent::Type::PrintEvent;
        evObj->printOutput = Chip32::Machine::FormatStringWithPlaceholders(ctx, format, args);
        m_eventBus.Emit(evObj);
    }
    else if (code == 5) // WAIT (sleep)
    {
        // Sleep is in milliseconds, R0 contains the duration
        std::this_thread::sleep_for(std::chrono::milliseconds(ctx->registers[R0]));
        m_logger.Log("VM slept for " + std::to_string(ctx->registers[R0]) + " ms.");
    }
    else
    {
        m_logger.Log("Unknown syscall code: " + std::to_string(code), true);
    }

    return retCode;
}

void AppController::UpdateVmView()
{
    // FIXME !!

    // C'est une fonction de notification pour la GUI.
    // AppController ne devrait pas directement manipuler les vues GUI.
    // Au lieu de cela, il émettrait un signal ou appellerait un observer.
    uint32_t pcVal = m_machine.ctx.registers[PC];

    if (m_story)
    {
        if (m_machine.GetAssemblyLine(pcVal, m_dbg.line))
        {
            m_logger.Log("Executing line: " + std::to_string(m_dbg.line + 1));
        // m_debuggerWindow.HighlightLine(m_dbg.line); // Dépendance GUI
        }        
    }
    // m_cpuWindow.updateRegistersView(m_chip32_ctx); // Dépendance GUI
    // m_memoryEditor.DrawWindow("RAM view", m_chip32_ctx.ram.mem, m_chip32_ctx.ram.size); // Dépendance GUI
}

void AppController::AddRecentProject(const std::string& projectPath)
{
    // Remove if already exists to move to front
    m_recentProjects.erase(
        std::remove(m_recentProjects.begin(), m_recentProjects.end(), projectPath),
        m_recentProjects.end()
    );

    // Add to front
    m_recentProjects.insert(m_recentProjects.begin(), projectPath);

    // Limit to 10 recent projects
    if (m_recentProjects.size() > 10) {
        m_recentProjects.resize(10);
    }

    // Save recent projects on disk
    SaveParams();
}

void AppController::CloseProject()
{
    if (m_story)
    {
        m_story->Clear(); // Clear story resources/data
        m_story.reset(); // Release shared_ptr
    }
    m_resources.Clear();

    // Clear VM state
    m_dbg.run_result = VM_FINISHED;
    m_dbg.free_run = false;
    m_dbg.m_breakpoints.clear();
    
    m_resources.Clear(); // Clear loaded resources
    m_eventQueue.clear(); // Clear any pending VM events

    m_logger.Log("Project closed.");
}

void AppController::SaveProject()
{
    if (m_story)
    {
        m_story->Save(m_resources);
        m_libraryManager.Scan(); // Add new project to library
        m_logger.Log("Project saved: " + m_story->GetProjectFilePath());
    } else {
        m_logger.Log("No project open to save.", true);
    }
}

std::shared_ptr<StoryProject> AppController::NewModule()
{
    m_module = m_nodesFactory.NewModule();
    return m_module;
}

void AppController::SaveModule()
{
    m_nodesFactory.SaveAllModules(m_resources);
    m_logger.Log("Modules saved.");
}

std::shared_ptr<StoryProject> AppController::OpenModule(const std::string &uuid)
{
    m_module = m_nodesFactory.GetModule(uuid);
    if (!m_module)
    {
        m_eventBus.Emit(std::make_shared<GenericResultEvent>(false, "Cannot find module: " + uuid));
        
    }
    else if (m_module->Load(m_resources, m_nodesFactory))
    {
        m_eventBus.Emit(std::make_shared<GenericResultEvent>(true, "Open module success: " + uuid));
    }
    else
    {
        m_eventBus.Emit(std::make_shared<GenericResultEvent>(false, "Failed to open module: " + uuid));
    }
    return m_module;
}

void AppController::CloseModule()
{
    if (m_module) {
        m_module->Clear();
        m_module.reset();
    }
    // Notify GUI (e.g., m_moduleEditorWindow.Clear(); m_moduleEditorWindow.Disable();)
    m_logger.Log("Module closed.");
}

void AppController::ImportProject(const std::string &filePathName, int format)
{
    (void) format; // Not used in the original snippet but kept for signature.
    PackArchive archive(m_logger, m_nodesFactory); // PackArchive constructor might need an ILogger, adjust if needed

    auto ext = SysLib::GetFileExtension(filePathName);
    auto filename = SysLib::GetFileName(filePathName);

    if ((ext == "pk") || (filename == "ni"))
    {
        archive.ImportCommercialFormat(filePathName, m_libraryManager.LibraryPath(), m_libraryManager.CommercialDbView());
        m_logger.Log("Imported commercial format: " + filePathName);
    }
    else if ((ext == "json") || (ext == "zip"))
    {
        archive.ImportStudioFormat(filePathName, m_libraryManager.LibraryPath());
        m_logger.Log("Imported studio format: " + filePathName);
    }
    else
    {
        m_logger.Log("Unknown file format for import: " + filePathName, true);
    }

    // Send event success
    m_eventBus.Emit(std::make_shared<GenericResultEvent>(true, "Import successful"));

}

std::shared_ptr<IStoryProject> AppController::GetCurrentProject()
{
    return m_story;
}

std::shared_ptr<IStoryProject> AppController::GetCurrentModule()
{
    return m_module;
}
