#include <QCoreApplication>
#include <QGuiApplication>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QCommandLineParser>

#include "statefulhotreloadserver.h"

//// Command line tool
int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    QCoreApplication::setApplicationName("hotreloadserver");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Hot Reload Server");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    parser.addPositionalArgument("watchdir", QCoreApplication::translate("main", "Directory to watch for changes. Usualy your qml root dir."));
    parser.addPositionalArgument("startscript", QCoreApplication::translate("main", "Optional root Qml file, relative to <watchdir>. If no script is given only the server is started. Note: Use the Hot Reload Server Qml Template for this file."));
    QCommandLineOption skip({{"skip","s"}, "Directories that should not be watched, seperated by ';'", "dirs"});
    parser.addOption(skip);
    parser.process(a);

    if(parser.isSet(helpOption)) {
        parser.showHelp();
    }
    if(parser.isSet(versionOption)) {
        parser.showHelp();
    }
    QString dir(".");
    QStringList skipDirs;
    if(parser.positionalArguments().length() >= 1) {
        dir = parser.positionalArguments().at(0);
    }
    if(parser.isSet(skip)) {
        skipDirs = parser.value(skip).split(";");
    }
    StatefulHotReloadServer server(dir, skipDirs);
    QQmlApplicationEngine *engine;
    if(parser.positionalArguments().length() >= 2) {
        QString startScript = parser.positionalArguments().at(1);
        QDir path(dir);
        startScript = path.filePath(startScript);
        engine = new QQmlApplicationEngine( &a );

        //engine->addImportPath(QDir::currentPath()+"/hotlivereload");
        engine->load(startScript);
    }

    return a.exec();
}
