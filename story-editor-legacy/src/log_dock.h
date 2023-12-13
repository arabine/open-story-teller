#ifndef LOGDOCK_H
#define LOGDOCK_H

#include "dock_widget_base.h"
#include "ui_ost-log.h"

class LogDock : public DockWidgetBase
{
public:
    LogDock();

    void Append(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Ui::ostLog m_logUi;
};

#endif // LOGDOCK_H
