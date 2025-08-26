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

// Définitions des registres et événements CHIP-32 si non déjà dans chip32_vm.h
// Assurez-vous que ces définitions sont accessibles.
#ifndef R0
#define R0 0
#endif
#ifndef R1
#define R1 1
#endif
#ifndef R2
#define R2 2
#endif
#ifndef R3
#define R3 3
#endif
#ifndef R4
#define R4 4
#endif
#ifndef PC
#define PC 7 // Exemple de registre Program Counter
#endif

#ifndef EV_MASK_OK_BUTTON
#define EV_MASK_OK_BUTTON 1
#endif
#ifndef EV_MASK_HOME_BUTTON
#define EV_MASK_HOME_BUTTON 2
#endif
#ifndef EV_MASK_PREVIOUS_BUTTON
#define EV_MASK_PREVIOUS_BUTTON 4
#endif
#ifndef EV_MASK_NEXT_BUTTON
#define EV_MASK_NEXT_BUTTON 8
#endif
#ifndef EV_MASK_END_OF_AUDIO
#define EV_MASK_END_OF_AUDIO 16
#endif

// Définitions pour les codes de retour de Syscall
#ifndef SYSCALL_RET_OK
#define SYSCALL_RET_OK 0
#endif
#ifndef SYSCALL_RET_WAIT_EV
#define SYSCALL_RET_WAIT_EV 1 // Exemple: VM doit attendre un événement
#endif


AppController::AppController(ILogger& logger, EventBus& eventBus)
    : m_logger(logger)
    , m_eventBus(eventBus) // m_eventBus pour les événements
    , m_resources(logger) // m_resources a besoin d'un IStoryManager/IResourceSource
    , m_nodesFactory(logger) // m_nodesFactory a besoin d'un IStoryManager/IResourceSource
    , m_libraryManager(logger, m_nodesFactory) // m_libraryManager a besoin d'un IStoryManager/INodeFactory
    , m_player(*this) // m_player a besoin d'un IAudioEvent
    , m_webServer(m_libraryManager)
{
    // VM Initialize - Déplacé du constructeur de MainWindow
    m_chip32_ctx.stack_size = 512;

    m_chip32_ctx.rom.mem = m_rom_data;
    m_chip32_ctx.rom.addr = 0;
    m_chip32_ctx.rom.size = sizeof(m_rom_data);

    m_chip32_ctx.ram.mem = m_ram_data;
    m_chip32_ctx.ram.addr = sizeof(m_rom_data);
    m_chip32_ctx.ram.size = sizeof(m_ram_data);

    // Initialise le trampoline de syscall avec cette instance
    // SyscallTrampoline::s_instance = this;
    // m_chip32_ctx.syscall = SyscallTrampoline::Callback;

    Callback<uint8_t(chip32_ctx_t *, uint8_t)>::func = std::bind(&AppController::Syscall, this, std::placeholders::_1, std::placeholders::_2);
    m_chip32_ctx.syscall = static_cast<syscall_t>(Callback<uint8_t(chip32_ctx_t *, uint8_t)>::callback);

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

void AppController::BuildNodes(IStoryProject::Type type)
{
    if (type == IStoryProject::Type::PROJECT_TYPE_STORY && m_story) {
        if (m_story->GenerateScript(m_currentCode))
        {
            // La GUI (DebuggerWindow) doit être notifiée de cette mise à jour.
            // Au lieu de appeler m_debuggerWindow.SetScript(m_currentCode); directement,
            // AppController pourrait émettre un événement ou un callback.
            // Pour l'instant, on suppose une notification ou que la GUI tire les données.
            m_logger.Log("Nodes script generated for story.");
            Build(true); // Compile seulement par défaut
        }
    } else if (type == IStoryProject::Type::PROJECT_TYPE_MODULE && m_module) {
        // Logique de génération de script pour le module
        // Similaire à BuildNodes pour le projet principal.
        m_logger.Log("Nodes script generated for module.");
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

    Chip32::Assembler::Error err;
    // La GUI (DebuggerWindow) doit être notifiée pour effacer les erreurs. FIXME
    // m_debuggerWindow.ClearErrors();
    
    if (m_story->GenerateBinary(m_currentCode, err))
    {
        m_result.Print(); // Imprime le résultat de l'assemblage (Debug uniquement)

        if (m_story->CopyProgramTo(m_rom_data, sizeof (m_rom_data)))
        {
            m_story->SaveBinary();
            chip32_initialize(&m_chip32_ctx);
            m_dbg.run_result = VM_READY;
            UpdateVmView(); // Notifie la GUI de mettre à jour la vue VM
            m_logger.Log("Build successful. VM ready.");
        }
        else
        {
            m_logger.Log("Program too big. Expand ROM memory.", true);
        }
    }
    else
    {
        m_logger.Log(err.ToString(), true);
        // La GUI (DebuggerWindow) doit être notifiée pour ajouter l'erreur. FIXME
        // m_debuggerWindow.AddError(err.line, err.message);
    }
}

void AppController::BuildCode(std::shared_ptr<StoryProject> story, bool compileonly, bool force)
{
    // Note: Dans le code original, BuildCode lisait m_externalSourceFileName.
    // Il faut s'assurer que m_currentCode est bien défini avant d'appeler Build.
    if (story) {
        m_currentCode = SysLib::ReadFile(story->GetProjectFilePath()); // Simplifié pour l'exemple
        // La GUI (DebuggerWindow) doit être notifiée pour SetScript.
        // m_debuggerWindow.SetScript(m_currentCode);
        Build(compileonly);
    } else {
        m_logger.Log("No story provided for BuildCode.", true);
    }
}


void AppController::BuildCode(bool compileonly)
{
    m_currentCode = SysLib::ReadFile(m_externalSourceFileName);
    // m_debuggerWindow.SetScript(m_currentCode); // FIXME: GUI event
    Build(compileonly);
}


void AppController::BuildNodes(bool compileonly)
{
    if (m_story->GenerateScript(m_currentCode))
    {
       //  m_debuggerWindow.SetScript(m_currentCode); // FIXME: GUI event
        Build(compileonly);
    }
}



void AppController::SetExternalSourceFile(const std::string &filename)
{
    m_externalSourceFileName = filename;
    m_logger.Log("External source file set to: " + filename);
}

void AppController::LoadBinaryStory(const std::string &filename)
{
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp != NULL)
    {
        fseek(fp, 0L, SEEK_END);
        long int sz = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (sz <= m_chip32_ctx.rom.size)
        {
            size_t sizeRead = fread(m_chip32_ctx.rom.mem, 1, sz, fp); // Corrected fread args
            if (sizeRead == (size_t)sz) // Cast sz to size_t for comparison
            {
                m_dbg.run_result = VM_READY;
                chip32_initialize(&m_chip32_ctx);
                m_logger.Log("Loaded binary file: " + filename);
                UpdateVmView();
            }
            else
            {
                m_logger.Log("Failed to load binary file completely. Read " + std::to_string(sizeRead) + " of " + std::to_string(sz) + " bytes.", true);
            }
        } else {
            m_logger.Log("Binary file is too large for ROM: " + std::to_string(sz) + " bytes, max " + std::to_string(m_chip32_ctx.rom.size) + " bytes.", true);
        }
        fclose(fp);
    } else {
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
        regVal = m_chip32_ctx.registers[reg];
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

    }
    catch(const std::exception &e)
    {
        m_logger.Log("Error loading settings: " + std::string(e.what()), true);
    }
}

std::string AppController::GetStringFromMemory(uint32_t addr)
{
    // Buffer local pour la chaîne
    // Assurez-vous qu'il est assez grand pour gérer les chaînes de votre VM
    // et qu'il est terminé par null
    char strBuf[256]; // Augmenté la taille pour plus de sécurité

    // Le bit le plus significatif indique si c'est de la RAM (0x80000000) ou ROM
    bool isRam = (addr & 0x80000000) != 0;
    addr &= 0xFFFF; // Masque pour obtenir l'adresse 16 bits

    // Vérification de l'adresse pour éviter les dépassements de buffer
    if (isRam) {
        if (addr < m_chip32_ctx.ram.size) {
            strncpy(strBuf, (const char *)&m_chip32_ctx.ram.mem[addr], sizeof(strBuf) - 1);
            strBuf[sizeof(strBuf) - 1] = '\0'; // S'assurer que c'est null-terminated
        } else {
            m_logger.Log("GetStringFromMemory: Invalid RAM address: 0x" + std::to_string(addr), true);
            return "";
        }
    } else {
        if (addr < m_chip32_ctx.rom.size) {
            strncpy(strBuf, (const char *)&m_chip32_ctx.rom.mem[addr], sizeof(strBuf) - 1);
            strBuf[sizeof(strBuf) - 1] = '\0'; // S'assurer que c'est null-terminated
        } else {
            m_logger.Log("GetStringFromMemory: Invalid ROM address: 0x" + std::to_string(addr), true);
            return "";
        }
    }
    return strBuf;
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
    m_dbg.run_result = chip32_step(&m_chip32_ctx);
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
            std::string imageFile = m_story->BuildFullAssetsPath(GetStringFromMemory(ctx->registers[R0]));
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
            std::string soundFile = m_story->BuildFullAssetsPath(GetStringFromMemory(ctx->registers[R1]));
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
        std::string text = GetStringFromMemory(ctx->registers[R0]);
        int arg_count = ctx->registers[R1];
        char working_buf[200] = {0};

        // Simplified printf logic for logging
        switch(arg_count){
            case 0: m_logger.Log(text); break;
            case 1: snprintf(working_buf, sizeof(working_buf), text.c_str(), ctx->registers[R2]); m_logger.Log(working_buf); break;
            case 2: snprintf(working_buf, sizeof(working_buf), text.c_str(), ctx->registers[R2], ctx->registers[R3]); m_logger.Log(working_buf); break;
            case 3: snprintf(working_buf, sizeof(working_buf), text.c_str(), ctx->registers[R2], ctx->registers[R3], ctx->registers[R4]); m_logger.Log(working_buf); break;
            default: m_logger.Log("Printf with unsupported arg_count: " + std::to_string(arg_count) + " text: " + text, true); break;
        }
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
    uint32_t pcVal = m_chip32_ctx.registers[PC];

    if (m_story && m_story->GetAssemblyLine(pcVal, m_dbg.line))
    {
        m_logger.Log("Executing line: " + std::to_string(m_dbg.line + 1));
        // m_debuggerWindow.HighlightLine(m_dbg.line); // Dépendance GUI
    }
    else
    {
        m_logger.Log("Reached end or instruction not found (line: " + std::to_string(m_dbg.line) + ")", false);
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
    chip32_initialize(&m_chip32_ctx); // Reset VM context

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

std::shared_ptr<IStoryProject> AppController::OpenModule(const std::string &uuid)
{
    m_module = m_nodesFactory.GetModule(uuid);
    if (!m_module)
    {
        m_eventBus.Emit(std::make_shared<GenericResultEvent>(false, "Cannot find module: " + uuid));
        m_logger.Log("Cannot find module: " + uuid, true);
    }
    else if (m_module->Load(m_resources, m_nodesFactory))
    {
        m_eventBus.Emit(std::make_shared<ModuleEvent>(ModuleEvent::Type::Opened, uuid));
        m_logger.Log("Open module success: " + uuid);
    }
    else
    {
        m_eventBus.Emit(std::make_shared<GenericResultEvent>(false, "Failed to open module: " + uuid));
        m_logger.Log("Open module error: " + uuid, true);
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
    return m_story; // Retourne le projet actuel
}
