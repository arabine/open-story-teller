#include "vm_dock.h"

VmDock::VmDock(Chip32::Assembler &assembler)
    : QDockWidget(tr("Virtual Machine"))
{
    setObjectName("VirtualMachineDock");  // used to save the state

    m_uiVM.setupUi(this);

    // Add registers
    m_uiVM.regsTable->setRowCount(REGISTER_COUNT);
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        std::string regName;
        assembler.GetRegisterName(i, regName);
        QTableWidgetItem *regNameItem = new QTableWidgetItem(regName.c_str());
        QTableWidgetItem *regValueItem = new QTableWidgetItem(QString::number(0));

        m_uiVM.regsTable->setItem(i, 0, regNameItem);
        m_uiVM.regsTable->setItem(i, 1, regValueItem);
    }

    connect(m_uiVM.generateButton, &QPushButton::clicked, [=](bool checked) {
        emit sigCompile();
    });

    connect(m_uiVM.playButton, &QPushButton::clicked, [=](bool checked) {
        emit sigStepInstruction();
    });

    connect(m_uiVM.buildButton, &QPushButton::clicked, [=](bool checked) {
        emit sigBuild();
    });
}

void VmDock::updateRegistersView(const chip32_ctx_t &ctx)
{
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        m_uiVM.regsTable->item(i, 1)->setText(QString::number(ctx.registers[i]));
    }
}

