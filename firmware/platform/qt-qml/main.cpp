#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine> // pour addImportPath()
#include <QString>
#include <QtGlobal> // qInstallMessageHandler()
#include <QtQml> // pour qmlRegisterType()
#include <QIcon>

#include "packarchive.h"
#include "StoryTellerModel.h" // LCD adapter from C to QML

#ifdef USE_WINDOWS_OS
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    //set names
    QCoreApplication::setOrganizationName("D8S");
    QCoreApplication::setApplicationName("Open Story Teller");

    QGuiApplication app(argc, argv);

    StoryTellerModel storyTeller;

    QQmlApplicationEngine engine;

    QQmlContext * ctxt = engine.rootContext();
    ctxt->setContextProperty("storyTeller", &storyTeller); // on rend cet objet accessible en QML

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
