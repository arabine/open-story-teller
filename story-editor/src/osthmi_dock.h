#ifndef OSTHMI_DOCK_H
#define OSTHMI_DOCK_H

#include "dock_widget_base.h"
#include "ui_ost-hmi.h"

class OstHmiDock : public DockWidgetBase
{
    Q_OBJECT
public:
    OstHmiDock();

    void SetImage(const QString &fileName);
    void ClearImage();

signals:
    void sigOkButton();
    void sigLeftButton();
    void sigRightButton();

private:
    Ui::ostDisplay m_uiOstDisplay;
};

#endif // OSTHMI_DOCK_H
