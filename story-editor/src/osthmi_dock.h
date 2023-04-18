#ifndef OSTHMI_DOCK_H
#define OSTHMI_DOCK_H

#include <QDockWidget>
#include "ui_ost-hmi.h"

class OstHmiDock : public QDockWidget
{
    Q_OBJECT
public:
    OstHmiDock();

    void SetImage(const QString &fileName);

signals:
    void sigOkButton();

private:
    Ui::ostDisplay m_uiOstDisplay;
};

#endif // OSTHMI_DOCK_H
