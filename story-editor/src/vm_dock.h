#ifndef VMDOCK_H
#define VMDOCK_H

#include "dock_widget_base.h"
#include "ui_ost-vm.h"

#include "chip32_assembler.h"

class VmDock : public DockWidgetBase
{
    Q_OBJECT

public:
    VmDock(Chip32::Assembler &assembler);

    void updateRegistersView(const chip32_ctx_t &ctx);

signals:
    void sigStepInstruction();
    void sigBuild();
    void sigCompile();

private:
    Ui::ostVM m_uiVM;
};

#endif // VMDOCK_H
