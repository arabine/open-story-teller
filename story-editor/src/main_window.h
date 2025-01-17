#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <functional>

#include "gui.h"
#include "console_window.h"
#include "code_editor.h"

#include "emulator_window.h"
#include "resources_window.h"
#include "node_editor_window.h"
#include "properties_window.h"

#include "chip32_assembler.h"
#include "chip32_vm.h"
#include "story_project.h"
#include "i_story_manager.h"
#include "thread_safe_queue.h"
#include "audio_player.h"
#include "library_manager.h"
#include "library_window.h"
#include "cpu_window.h"
#include "story_machine.h"
#include "web_server.h"

struct DebugContext
{
    uint32_t event_mask{0};
    bool wait_event{0};
    bool free_run{false};
    uint32_t line{0};
    chip32_result_t run_result{VM_FINISHED};

    std::set<int> m_breakpoints;

    void Stop() {
        run_result = VM_FINISHED;
    }

    bool IsValidEvent(uint32_t event) {
        return (event_mask & event) != 0;
    }

    static void DumpCodeAssembler(Chip32::Assembler & assembler) {

        for (std::vector<Chip32::Instr>::const_iterator iter = assembler.Begin();
             iter != assembler.End(); ++iter)
        {
            if (iter->isRomCode() || iter->isRomData)
            {
                std::cout << "-------------------" << std::endl;
                std::cout << "Instr: " << iter->mnemonic.c_str() << std::endl;
                std::cout << "Addr: " <<  std::hex << iter->addr << std::endl;
                std::cout << "Line: " << iter->line << std::endl;
                std::cout << "\t- Opcode: "  << std::hex <<  iter->code.opcode
                         << ", opcode args: " << iter->code.bytes << std::endl;

                int i = 1;
                for (auto arg : iter->compiledArgs)
                {
                    std::cout  << "\t- Arg " << i << " : " << std::hex << arg << std::endl;
                    i++;
                }
            }
        }

    }
};


class MainWindow : public IStoryManager, public IAudioEvent, public ILogger
{
public:
    MainWindow();
    ~MainWindow();

    bool Initialize();
    void Loop();

private:
    enum VmEventType { EvNoEvent, EvStep, EvRun, EvOkButton, EvPreviousButton, EvNextButton, EvAudioFinished, EvStop, EvHomeButton};

    std::shared_ptr<StoryProject> m_story;

    // VM
    uint8_t m_rom_data[16*1024];
    uint8_t m_ram_data[16*1024];
    chip32_ctx_t m_chip32_ctx;
    
    Chip32::Result m_result;
    DebugContext m_dbg;
    std::string m_currentCode;

    std::vector<std::string> m_recentProjects;

    ResourceManager m_resources;

    LibraryManager m_libraryManager;

    Gui m_gui;
    EmulatorWindow m_emulatorWindow;
    ConsoleWindow m_consoleWindow;
    CodeEditor m_codeEditorWindow;
    CpuWindow m_cpuWindow;

    char m_project_name[256] = "";

    ResourcesWindow m_resourcesWindow;

    NodeEditorWindow m_nodeEditorWindow;

    PropertiesWindow m_PropertiesWindow;

    LibraryWindow m_libraryWindow;

    AudioPlayer m_player;

    struct VmEvent
    {
        VmEventType type;
    };

    ThreadSafeQueue<VmEvent> m_eventQueue;

    WebServer m_webServer;

    // From IStoryManager (proxy to StoryProject class)
    virtual void OpenProject(const std::string &uuid) override;
    virtual void ImportProject(const std::string &filePathName, int format);
    virtual void PlaySoundFile(const std::string &fileName) override;;
    virtual std::string BuildFullAssetsPath(const std::string_view fileName) const override;
    virtual std::pair<FilterIterator, FilterIterator> Images() override;
    virtual std::pair<FilterIterator, FilterIterator> Sounds() override;
    virtual void OpenFunction(const std::string &uuid, const std::string &name) override;

    virtual void AddResource(std::shared_ptr<Resource> res) override;
    virtual void ClearResources() override;
    virtual std::pair<FilterIterator, FilterIterator> Resources() override;
    virtual void DeleteResource(FilterIterator &it) override;
    

    virtual void BuildNodes(bool compileonly) override;
    virtual void BuildCode(bool compileonly) override;
    virtual void LoadBinaryStory(const std::string &filename) override;
    virtual void ToggleBreakpoint(int line) override;
    virtual uint32_t GetRegister(int reg) override;

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


    // From ILogger
    virtual void Log(const std::string &txt, bool critical = false) override;

    // From IAudioEvent
    virtual void EndOfAudio() override;

    void SaveParams();
    void LoadParams();

    float DrawMainMenuBar();
    bool ShowQuitConfirm();
    void DrawToolBar(float topPadding);

    void SaveProject();
    void CloseProject();
    void DrawStatusBar();

    void UpdateVmView();
    uint8_t Syscall(chip32_ctx_t *ctx, uint8_t code);
    std::string GetFileNameFromMemory(uint32_t addr);
    void ProcessStory();
    void StepInstruction();
    void RefreshProjectInformation();
    void ProjectPropertiesPopup();
    void Build(bool compileonly);
};

#endif // MAINWINDOW_H
