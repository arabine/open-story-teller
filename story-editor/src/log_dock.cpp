#include "log_dock.h"

LogDock::LogDock()
    : QDockWidget(tr("Logs"))
{
    setObjectName("OstHmiDock"); // used to save the state
    m_logUi.setupUi(this);
}

void LogDock::Append(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
//    const char *file = context.file ? context.file : "";
//    const char *function = context.function ? context.function : "";
    switch (type) {
//    case QtDebugMsg:
//        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
//        break;
//    case QtInfoMsg:
//        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
//        break;
//    case QtWarningMsg:
//        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
//        break;
    case QtCriticalMsg:
        m_logUi.logText->appendHtml("<font color=\"Red\">" + msg + "</font>");
        break;
//    case QtFatalMsg:
//        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);

    default:
        m_logUi.logText->appendHtml("<font color=\"Aqua\">" + msg + "</font>");
        break;
    }
}
