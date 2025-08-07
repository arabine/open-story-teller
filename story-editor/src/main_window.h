#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <functional>
#include <memory>
#include <string>

#include "gui.h"
#include "console_window.h"
#include "debugger_window.h"
#include "emulator_dock.h"
#include "resources_window.h"
#include "node_editor_window.h"
#include "properties_window.h"
#include "variables_window.h"
#include "library_window.h"
#include "cpu_window.h"
// Dialogs
#include "about_dialog.h"

#include "event_bus.h"
#include "app_controller.h"
#include "i_logger.h"


class MainWindow : public std::enable_shared_from_this<MainWindow>, public ILogSubject
{
public:
    MainWindow(ILogger& logger, EventBus& eventBus, AppController& appController);
    ~MainWindow();

    bool Initialize();
    void Loop();

private:
    enum VmEventType { EvNoEvent, EvStep, EvRun, EvOkButton, EvPreviousButton, EvNextButton, EvAudioFinished, EvStop, EvHomeButton};

    ILogger& m_logger;
    EventBus& m_eventBus;
    AppController &m_appController; // Controller for application logic

    std::shared_ptr<StoryProject> m_story; // Current story
    std::shared_ptr<StoryProject> m_module; // Current module

    std::vector<std::string> m_recentProjects;

    Gui m_gui;
    EmulatorDock m_emulatorWindow;
    ConsoleWindow m_consoleWindow;
    DebuggerWindow m_debuggerWindow;
    CpuWindow m_cpuWindow;

    char m_project_name[256] = "";

    ResourcesWindow m_resourcesWindow;
    NodeEditorWindow m_nodeEditorWindow;
    NodeEditorWindow m_moduleEditorWindow;
    PropertiesWindow m_PropertiesWindow;
    LibraryWindow m_libraryWindow;
    VariablesWindow m_variablesWindow;

    // Dialogs
    AboutDialog m_aboutDialog;

    // From IStoryManager (proxy to StoryProject class)
    void OpenProject(const std::string &uuid);
    void SaveProject();
    void CloseProject();

    void OpenModule(const std::string &uuid);  
    void NewModule();
    void SaveModule();
    void CloseModule();
    
    // From ILogSubject
    virtual void LogEvent(const std::string &txt, bool critical) override;

    void LoadParams();

    float DrawMainMenuBar();
    bool ShowQuitConfirm();
    void DrawToolBar(float topPadding);

    void UpdateVmView();
    uint8_t Syscall(chip32_ctx_t *ctx, uint8_t code);
    std::string GetStringFromMemory(uint32_t addr);
    void ProcessStory();
    void StepInstruction();
    void RefreshProjectInformation();
    void ProjectPropertiesPopup();
    void Build(bool compileonly);
};

#endif // MAINWINDOW_H
