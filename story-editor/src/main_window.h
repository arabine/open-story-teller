#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QAction>
#include <QScreen>
#include <QtWidgets/QApplication>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QDockWidget>


#include <QtNodes/ConnectionStyle>

#include <QtNodes/StyleCollection>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>


using QtNodes::DataFlowGraphicsScene;
using QtNodes::ConnectionStyle;


using QtNodes::NodeRole;
using QtNodes::StyleCollection;
using QtNodes::DataFlowGraphModel;
using QtNodes::NodeDelegateModelRegistry;

#include "ui_choose-file.h"
#include "story_project.h"
#include "story_graph_model.h"
#include "story_graph_scene.h"
#include "chip32_assembler.h"
#include "chip32_vm.h"
#include "vm_dock.h"
#include "resources_dock.h"
#include "script_editor_dock.h"
#include "memory_view_dock.h"
#include "osthmi_dock.h"
#include "nodeeditor_dock.h"
#include "log_dock.h"
#include "toolbar.h"

struct DebugContext
{
    uint32_t event_mask{0};
    bool wait_event{0};
    bool running{false};
    int line{-1};
    chip32_result_t run_result{VM_FINISHED};

    static void DumpCodeAssembler(Chip32::Assembler & assembler) {

        for (std::vector<Chip32::Instr>::const_iterator iter = assembler.Begin();
             iter != assembler.End(); ++iter)
        {
            if (iter->isRomCode() || iter->isRomData)
            {
                qDebug() << "-------------------";
                qDebug() << "Instr: " << iter->mnemonic.c_str();
                qDebug() << "Addr: " <<  Qt::hex << iter->addr;
                qDebug() << "Line: " << iter->line;
                qDebug() << "\t- Opcode: "  << Qt::hex <<  iter->code.opcode
                                            << ", opcode args: " << iter->code.bytes;

                int i = 1;
                for (auto arg : iter->compiledArgs)
                {
                    qDebug() << "\t- Arg " << i << " : " << Qt::hex << arg;
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


class VmEvent : public QEvent {
public:
    VmEvent(QEvent::Type type)
        : QEvent(type)
    {}
    static const QEvent::Type evStep = static_cast<QEvent::Type>(QEvent::User + 1);
    static const QEvent::Type evOkButton = static_cast<QEvent::Type>(QEvent::User + 2);

    uint8_t ev{0};
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void stepInstruction();
    void closeEvent(QCloseEvent *event);

private:
    StoryProject m_project;
    StoryGraphModel m_model;
    StoryGraphScene m_scene;

    // Qt stuff
    ToolBar *m_toolbar{nullptr};
    NodeEditorDock *m_nodeEditorDock{nullptr};
    OstHmiDock *m_ostHmiDock{nullptr};
    ResourcesDock *m_resourcesDock{nullptr};
    ScriptEditorDock *m_scriptEditorDock{nullptr};
    VmDock *m_vmDock{nullptr};
    MemoryViewDock *m_ramView{nullptr};
    MemoryViewDock *m_romView{nullptr};
    LogDock *m_logDock{nullptr};

    QDialog *m_chooseFileDialog;
    Ui::chooseFileDIalog m_chooseFileUi;

    // VM
    uint8_t m_rom_data[16*1024];
    uint8_t m_ram_data[16*1024];
    chip32_ctx_t m_chip32_ctx;

    // Assembleur & Debugger
    std::vector<uint8_t> m_program;
    Chip32::Assembler m_assembler;
    Chip32::Result m_result;
    DebugContext m_dbg;

    // Private functions
    void createActions();
    void createStatusBar();
    void save();
    void DisplayNode(StoryNode *m_tree, QtNodes::NodeId parentId);
    void about();
    void open();
    void buildScript();
    void highlightNextLine();
    void readSettings();
    void updateAll();
    uint8_t Syscall(uint8_t code);
    QString GetFileName(uint32_t addr);

    bool event(QEvent *event);
    void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // MAIN_WINDOW_H
