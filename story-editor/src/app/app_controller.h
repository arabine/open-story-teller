// app_controller.h
#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <string>
#include <vector>
#include <memory> // Pour std::shared_ptr
#include <functional> // Pour std::function
#include <set>      // Pour std::set

// Inclure les entêtes des dépendances de AppController
#include "story_project.h"      // Pour StoryProject
#include "chip32_assembler.h"   // Pour Chip32::Assembler
#include "chip32_vm.h"          // Pour chip32_ctx_t, Chip32::Result, chip32_result_t
#include "i_story_manager.h"    // Interface implémentée par AppController
#include "i_audio_event.h"      // Interface implémentée par AppController
#include "i_logger.h"           // Interface implémentée par AppController (pour les logs métier)
#include "thread_safe_queue.h"  // Pour ThreadSafeQueue
#include "audio_player.h"       // Pour AudioPlayer
#include "library_manager.h"    // Pour LibraryManager
#include "nodes_factory.h"      // Pour NodesFactory
#include "resource_manager.h"  // Pour ResourceManager (si elle est utilisée directement par AppController)
#include "web_server.h"         // Pour WebServer
#include "story_machine.h"      // Peut-être renommé en VmEvent ou Processus de VM
#include "variable.h"           // Pour Variable (si géré par AppController)
#include "debug_context.h"     // Pour DebugContext
#include "event_bus.h"

// Forward declaration pour éviter les dépendances circulaires si le logger est une GUI
class ILogger; // Peut être implémenté par la console_window par exemple


class AppController : public IStoryManager, public IAudioEvent
{
public:
    enum VmEventType { EvNoEvent, EvStep, EvRun, EvOkButton, EvPreviousButton, EvNextButton, EvAudioFinished, EvStop, EvHomeButton};

    struct VmEvent
    {
        VmEventType type;
    };

    // Le constructeur prend une référence vers une implémentation de ILogger
    // pour que la logique métier puisse logger sans dépendre de la GUI.
    AppController(ILogger& logger, EventBus& eventBus);
    ~AppController();

    // Initialisation de l'AppController
    bool Initialize();

    std::shared_ptr<StoryProject> NewProject() { return m_libraryManager.NewProject();  }
    void OpenProject(const std::string &uuid);
    void ImportProject(const std::string &fileName, int format);
    void Log(const std::string &txt, bool critical = false) override;
    void PlaySoundFile(const std::string &fileName);
    std::string BuildFullAssetsPath(const std::string_view fileName) const;
    void OpenFunction(const std::string &uuid, const std::string &name);
    void NewStory();
    void CloseProject();
    void SaveProject();
    std::shared_ptr<StoryProject> NewModule();
    void SaveModule();
    void CloseModule();
    std::shared_ptr<IStoryProject> OpenModule(const std::string &uuid);
    void OpenStory(const std::string &path = "");
    void SaveStory(const std::string &path = "");
    void ExportStory(const std::string &filename);
    std::shared_ptr<StoryProject> GetCurrentStory() const { return m_story; }
    std::shared_ptr<StoryProject> GetModuleStory() const { return m_module; }
    void BuildNodes(IStoryProject::Type type);
    void Build(bool compileonly);
    void BuildCode(std::shared_ptr<StoryProject> story, bool compileonly, bool force = false);

    // --- Fonctions de IStoryManager ---
    virtual void SetExternalSourceFile(const std::string &filename) override;
    virtual void LoadBinaryStory(const std::string &filename) override;
    virtual void ToggleBreakpoint(int line) override;
    virtual uint32_t GetRegister(int reg) override;
    virtual void ScanVariable(const std::function<void(std::shared_ptr<Variable> element)>& operation) override;
    virtual void AddVariable() override;
    virtual void DeleteVariable(int i) override;
    virtual void Play() override;
    virtual void Step() override;
    virtual void Run() override;
    virtual void Ok() override;
    virtual void Stop() override;
    virtual void Pause() override;
    virtual void Home() override;
    virtual void Next() override;
    virtual void Previous() override;
    virtual std::string VmState() const override;
    virtual void BuildNodes(bool compileonly);
    virtual void BuildCode(bool compileonly);
    virtual std::shared_ptr<IStoryProject> GetCurrentProject() override;

    // --- Fonctions de IAudioEvent ---
    virtual void EndOfAudio() override;

    // --- Fonctions internes de AppController (logique métier) ---
    void SaveParams();
    void LoadParams();

    // Méthodes pour interagir avec la VM et le débogueur
    chip32_ctx_t* GetChip32Context() { return &m_chip32_ctx; }
    DebugContext* GetDebugContext() { return &m_dbg; }
    std::string GetStringFromMemory(uint32_t addr);
    void ProcessStory();
    void StepInstruction();
    void StopAudio() { m_player.Stop(); }

    bool IsLibraryManagerInitialized() const { return m_libraryManager.IsInitialized(); }

    // Getters pour les managers gérés par AppController
    ResourceManager& GetResourceManager() { return m_resources; }
    LibraryManager& GetLibraryManager() { return m_libraryManager; }
    NodesFactory& GetNodesFactory() { return m_nodesFactory; }
    AudioPlayer& GetAudioPlayer() { return m_player; }
    WebServer& GetWebServer() { return m_webServer; }
    const std::vector<std::string>& GetRecentProjects() const { return m_recentProjects; }
    void AddRecentProject(const std::string& projectPath);

    // Fonction pour les syscalls (appel système) de la VM
    uint8_t Syscall(chip32_ctx_t *ctx, uint8_t code);

    // Méthode pour mettre à jour l'état de la vue VM (peut être un événement notifié à la GUI)
    void UpdateVmView();

private:
    ILogger& m_logger; // Référence au logger pour les messages métier
    EventBus& m_eventBus; // Bus d'événements pour la communication entre composants

    std::shared_ptr<StoryProject> m_story;
    std::shared_ptr<StoryProject> m_module;
    uint8_t m_rom_data[16*1024];
    uint8_t m_ram_data[16*1024];
    chip32_ctx_t m_chip32_ctx;
    Chip32::Result m_result;
    DebugContext m_dbg; // Contexte de débogage
    std::string m_currentCode;
    std::string m_externalSourceFileName;
    std::vector<std::string> m_recentProjects;

    NodesFactory m_nodesFactory;
    ResourceManager m_resources; // Gère les ressources (images, sons)
    LibraryManager m_libraryManager; // Gère les bibliothèques
    AudioPlayer m_player; // Gère la lecture audio
    ThreadSafeQueue<VmEvent> m_eventQueue; // File d'événements de la VM
    WebServer m_webServer; // Serveur web intégré

    // Fonctions privées utilitaires pour la logique métier
    void SetupVM(int start_line, uint32_t entry_point);
};

#endif // APP_CONTROLLER_H
