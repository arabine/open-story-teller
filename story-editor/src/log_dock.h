#ifndef LOGDOCK_H
#define LOGDOCK_H

#include <QDockWidget>
#include "ui_ost-log.h"

class LogDock : public QDockWidget
{
public:
    LogDock();

    void Append(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Ui::ostLog m_logUi;
};

#endif // LOGDOCK_H
