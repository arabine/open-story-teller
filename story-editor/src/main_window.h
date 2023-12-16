#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gui.h"
#include "console_window.h"
#include "code_editor.h"

#include "emulator_window.h"
#include "resources_window.h"
#include "node_editor_window.h"
#include "node_properties_window.h"

#include "chip32_assembler.h"
#include "chip32_vm.h"
#include "story_project.h"
#include "i_story_project.h"

struct DebugContext
{
    uint32_t event_mask{0};
    bool wait_event{0};
    bool free_run{false};
    int line{-1};
    chip32_result_t run_result{VM_FINISHED};

    std::set<int> m_breakpoints;

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

#include <functional>

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) {
        return func(args...);
    }
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;



class MainWindow : public IStoryProject
{
public:
    MainWindow();
    ~MainWindow();

    void Initialize();
    void Loop();

private:

    StoryProject m_project;

    // VM
    uint8_t m_rom_data[16*1024];
    uint8_t m_ram_data[16*1024];
    chip32_ctx_t m_chip32_ctx;

    // Assembleur & Debugger
    std::vector<uint8_t> m_program;
    Chip32::Assembler m_assembler;
    Chip32::Result m_result;
   // DebugContext m_dbg;

    std::vector<std::string> m_recentProjects;

    ResourceManager m_resources;

    Gui gui;
    EmulatorWindow m_emulatorWindow;
    ConsoleWindow m_consoleWindow;
    CodeEditor editor;

    ResourcesWindow m_resourcesWindow;

    NodeEditorWindow m_nodeEditorWindow;

    NodePropertiesWindow m_nodePropertiesWindow;


    char mBufAddress[200];
    char mBufReceivePath[200];
    char mBufSendPath[200];
    char mBufPort[10];

    int octets[4];

    std::string mServerAddr;
    std::string mServerRecUrl;
    std::string mServerSndUrl;
    int mServerPort;


    // From IStoryProject (proxy to StoryProject class)
    virtual void PlaySoundFile(const std::string &fileName) override;;
    virtual std::string BuildFullAssetsPath(const std::string &fileName) const override;
    virtual std::pair<FilterIterator, FilterIterator> Images() override;
    virtual std::pair<FilterIterator, FilterIterator> Sounds() override;

    virtual void AddResource(std::shared_ptr<Resource> res) override;
    virtual void ClearResources() override;
    virtual std::pair<FilterIterator, FilterIterator> Resources() override;


    void SaveParams();
    void LoadParams();

    void DrawMainMenuBar();
    void ShowOptionsWindow();
    bool ShowQuitConfirm();

    void NewProjectPopup();
    void SaveProject();
    void EnableProject();
    void CloseProject();
    void OpenProjectDialog();
    void DrawStatusBar();
};

#endif // MAINWINDOW_H
