#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <functional>
#include <memory>
#include <string>

#include "gui.h"
#include "console_window.h"
#include "debugger_window.h"
#include "emulator_dock.h"
#include "resources_dock.h"
#include "node_editor_window.h"
#include "properties_window.h"
#include "variables_window.h"
#include "library_window.h"
#include "cpu_window.h"
// Dialogs
#include "about_dialog.h"
#include "project_properties_dialog.h"

#include "event_bus.h"
#include "app_controller.h"
#include "i_logger.h"
#include "imgui_toast_notifier.h"

class MainWindow : public std::enable_shared_from_this<MainWindow>, public ILogSubject
{
public:
    MainWindow(ILogger& logger, EventBus& eventBus, AppController& appController);
    ~MainWindow();

    bool Initialize();
    bool Loop();

private:
    enum VmEventType { EvNoEvent, EvStep, EvRun, EvOkButton, EvPreviousButton, EvNextButton, EvAudioFinished, EvStop, EvHomeButton};

    ILogger& m_logger;
    EventBus& m_eventBus;
    AppController &m_appController; // Controller for application logic

    std::shared_ptr<StoryProject> m_story; // Current story
    std::shared_ptr<StoryProject> m_module; // Current module

    std::vector<std::string> m_recentProjects;

    Gui m_gui;
    EmulatorDock m_emulatorDock;
    ConsoleWindow m_consoleWindow;
    DebuggerWindow m_debuggerWindow;
    CpuWindow m_cpuWindow;
    ResourcesDock m_resourcesDock;
    NodeEditorWindow m_nodeEditorWindow;
    NodeEditorWindow m_moduleEditorWindow;
    PropertiesWindow m_PropertiesWindow;
    LibraryWindow m_libraryWindow;
    VariablesWindow m_variablesWindow;

    // Dialogs
    AboutDialog m_aboutDialog;
    ProjectPropertiesDialog m_projectPropertiesDialog;

    ImGuiToastNotifier m_toastNotifier;

    // From IStoryManager (proxy to StoryProject class)
    void OpenProject(const std::string &uuid);
    void CloseProject();
    void OpenFunction(const std::string &uuid, const std::string &name);
    void OpenModule(const std::string &uuid);  
    void NewModule();
    void SaveModule();
    void CloseModule();
    
    // From ILogSubject
    virtual void LogEvent(const std::string &txt, bool critical) override;

    void RefreshProjectInformation();
    float DrawMainMenuBar();
    bool ShowQuitConfirm();
    void DrawToolBar(float topPadding);
    void UpdateVmView();
};

#endif // MAINWINDOW_H
