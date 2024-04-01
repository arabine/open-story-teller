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

struct DebugContext
{
    uint32_t event_mask{0};
    bool wait_event{0};
    bool free_run{false};
    int line{-1};
    chip32_result_t run_result{VM_FINISHED};

    std::set<int> m_breakpoints;

    void Stop() {
        run_result = VM_FINISHED;
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


class MainWindow : public IStoryManager, public IAudioEvent
{
public:
    MainWindow();
    ~MainWindow();

    void Initialize();
    void Loop();

private:
    enum VmEventType { EvNoEvent, EvStep, EvOkButton, EvPreviousButton, EvNextButton, EvAudioFinished};

    std::shared_ptr<StoryProject> m_story;

    // VM
    uint8_t m_rom_data[16*1024];
    uint8_t m_ram_data[16*1024];
    chip32_ctx_t m_chip32_ctx;

    // Assembleur & Debugger
    std::vector<uint8_t> m_program;
    Chip32::Assembler m_assembler;
    Chip32::Result m_result;
    DebugContext m_dbg;
    std::string m_currentCode;


    std::vector<std::string> m_recentProjects;

    ResourceManager m_resources;

    LibraryManager m_libraryManager;

    Gui m_gui;
    EmulatorWindow m_emulatorWindow;
    ConsoleWindow m_consoleWindow;
    CodeEditor m_editorWindow;

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


    // From IStoryManager (proxy to StoryProject class)
    virtual void OpenProject(const std::string &uuid) override;
    virtual void Log(const std::string &txt, bool critical = false) override;
    virtual void PlaySoundFile(const std::string &fileName) override;;
    virtual std::string BuildFullAssetsPath(const std::string &fileName) const override;
    virtual std::pair<FilterIterator, FilterIterator> Images() override;
    virtual std::pair<FilterIterator, FilterIterator> Sounds() override;

    virtual void AddResource(std::shared_ptr<Resource> res) override;
    virtual void ClearResources() override;
    virtual std::pair<FilterIterator, FilterIterator> Resources() override;
    virtual void DeleteResource(FilterIterator &it) override;
    virtual void Build() override;
    virtual std::list<std::shared_ptr<Connection>> GetNodeConnections(unsigned long nodeId) override;
    virtual std::string GetNodeEntryLabel(unsigned long nodeId) override;
    virtual void Play() override;
    virtual void Ok() override;
    virtual void Pause() override;
    virtual void Next() override;
    virtual void Previous() override;

    // From IAudioEvent
    virtual void EndOfAudio() override;

    void SaveParams();
    void LoadParams();

    void DrawMainMenuBar();
    bool ShowQuitConfirm();

    void NewProjectPopup();
    void SaveProject();
    void CloseProject();
    void DrawStatusBar();

    bool CompileToAssembler();
    void ConvertResources();
    void GenerateBinary();
    void UpdateVmView();
    uint8_t Syscall(chip32_ctx_t *ctx, uint8_t code);
    std::string GetFileNameFromMemory(uint32_t addr);
    void ProcessStory();
    void StepInstruction();
    void RefreshProjectInformation();
    void ProjectPropertiesPopup();
};

#endif // MAINWINDOW_H
