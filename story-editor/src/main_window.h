// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023-2099 Anthony Rabine <anthony@rabine.fr>

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
#include <QtNodes/GraphicsView>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/StyleCollection>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>


using QtNodes::DataFlowGraphicsScene;
using QtNodes::ConnectionStyle;
using QtNodes::BasicGraphicsScene;
using QtNodes::GraphicsView;
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
#include "log_dock.h"
#include "toolbar.h"
#include "new_project_dialog.h"
#include "resource_model.h"

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

protected:
    void BuildAndRun();

private slots:
    void stepInstruction();
    void closeEvent(QCloseEvent *event);
    void slotDefaultDocksPosition();
    void slotWelcome();

private:
    StoryProject m_project;
    ResourceModel m_resourceModel;
    StoryGraphModel m_model;
    StoryGraphScene m_scene;
    GraphicsView *m_view{nullptr};

    // Qt stuff
    ToolBar *m_toolbar{nullptr};
    OstHmiDock *m_ostHmiDock{nullptr};
    ResourcesDock *m_resourcesDock{nullptr};
    ScriptEditorDock *m_scriptEditorDock{nullptr};
    VmDock *m_vmDock{nullptr};
    MemoryViewDock *m_ramView{nullptr};
    MemoryViewDock *m_romView{nullptr};
    LogDock *m_logDock{nullptr};
    QSettings m_settings;
    QDialog *m_chooseFileDialog;
    Ui::chooseFileDIalog m_chooseFileUi;
    NewProjectDialog *m_newProjectDialog{nullptr};
    QStringList m_recentProjects;

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
    void SaveProject();
    void DisplayNode(StoryNode *m_tree, QtNodes::NodeId parentId);
    void buildScript();
    void highlightNextLine();
    void readSettings();
    void updateAll();
    uint8_t Syscall(uint8_t code);
    QString GetFileName(uint32_t addr);

    bool event(QEvent *event);
    void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void NewProject();

    void RefreshProjectInformation();
    void CloseProject();
    void OpenProjectDialog();
    void ExitProgram();
    void EnableProject();
    void OpenProject(const QString &filePath);
    QString ReadResourceFile(const QString &fileName);
};

#endif // MAIN_WINDOW_H
