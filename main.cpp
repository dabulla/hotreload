#include <QCoreApplication>
#include <QGuiApplication>
#include <QtNetwork/QLocalSocket>

#include <QUrl>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QObject>
#include <QMetaObject>
#include <QFile>
#include <QFileInfo>

#include <QTcpServer>
#include <QTcpSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QSet>

#include <QDebug>
#include <QDir>
#include <QStringList>

#include <QQmlApplicationEngine>
#include <QQmlContext>

class HotReloadServer {
public:
    QString versionPath() {
        qDebug() << "DBG: Server URL" << m_updateNotifyServer->serverUrl().toString();
        QString versionDir(":");
        versionDir.append(QString::number(m_directoryFileServer->serverPort()));
        versionDir.append("/v");
        versionDir.append(QString::number(m_currentVersion));
        versionDir.append("/");
        return versionDir;
    }
    void addPathRecursively(QString path) {

        bool watching = m_watcher->addPath(path);
        if(watching) {
            qDebug() << "Watchdog watching:" << path;
        } else {
            qDebug() << "Error: Cannot watch " << path;
            return;
        }
        QDir dir(path);
        QStringList subdirs = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot);
        for(auto iter = subdirs.cbegin(); iter != subdirs.cend() ; ++iter) {
            addPathRecursively(*iter);
        }
    }
    // reloadDely: If initializing qml takes long, make sure it is only reloaded once. Watchdog my fire multiple times
    // which results in multiple reloads if reloadDelay is too short. 500ms seems okay for QtCreator. 50ms if reloading qml multiple times is okay!
    HotReloadServer(QString dir, quint16 watchdogPort = 8080, quint16 updateNotifierPort = 8081, int reloadDelay = 50)
        :m_currentVersion(0)
        ,m_currentDirectoryInfo(dir)
        ,m_timer(new QTimer())
        ,m_watcher(new QFileSystemWatcher())
        ,m_updateNotifyServer(new QWebSocketServer("Watchdog update notifier", QWebSocketServer::NonSecureMode))
        ,m_directoryFileServer(new QTcpServer())
    {
        ////// start watchdog and a timer collecting changes and fires once
        m_timer->setInterval(reloadDelay);
        m_timer->setTimerType(Qt::VeryCoarseTimer);
        m_timer->setSingleShot(true);
        QObject::connect(m_timer, &QTimer::timeout, [this]() {
            ++m_currentVersion;
            QString versionDir(versionPath());
            //send to clients
            for(auto iter = m_connectedClients.cbegin(); iter != m_connectedClients.cend(); ++iter) {
                (*iter)->sendTextMessage(versionDir);
            }
            m_changes.clear();
        });

        addPathRecursively(m_currentDirectoryInfo.canonicalFilePath());

        QObject::connect(m_watcher, &QFileSystemWatcher::directoryChanged, [this](const QString &path) {
            if (!m_changes.contains(path)) {
                qDebug() << "Changes detected: " << path;
            }
            m_changes.insert(path);
            m_timer->start();
            this->addPathRecursively(path);
        });

        ///// start websocket server that tells all clients that they have to relaod
        QObject::connect(m_updateNotifyServer, &QWebSocketServer::newConnection, [this]() {
            qDebug() << "Client connected to watchdog websocket";
            QWebSocket *socket = m_updateNotifyServer->nextPendingConnection();
            socket->sendTextMessage(versionPath());
            m_connectedClients.insert(socket);
            QObject::connect(socket, &QWebSocket::textMessageReceived, [this, socket](QString message) {
                qDebug() << "Client sent data to watchdog websocket:" << message;
                socket->sendTextMessage(versionPath());
            });
            QObject::connect(socket, &QWebSocket::disconnected, [this, socket]() {
                qDebug() << "Client disconnected";
                m_connectedClients.remove(socket);
            });
        });
        bool websocketListening = m_updateNotifyServer->listen(QHostAddress::Any, updateNotifierPort);
        if(websocketListening) {
            qDebug() << "Websocket listening:" << (m_updateNotifyServer->serverUrl().toString());
        } else {
            qDebug() << "Error: Websocket not listening";
        }

        ///// start tcp Server that serves all files of a directory
        qDebug() << "Hot Live Reload serves directory:" << m_currentDirectoryInfo.canonicalFilePath();
        QObject::connect(m_directoryFileServer, &QTcpServer::newConnection, [this](){
            while (m_directoryFileServer->hasPendingConnections())
            {
                QTcpSocket *socket = m_directoryFileServer->nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
                QObject::connect(socket, &QTcpSocket::readyRead, [socket, this](){
                    QString requestedPath;
                    char webBrowerRXData[3000];
                    QRegExp regexp("GET (/?v(\\d+)/)?(.*) HTTP/.*");
                    while(0 < socket->readLine(webBrowerRXData, 3000)) {
                        int pos = regexp.indexIn(webBrowerRXData);
                        //qDebug() << ":" << webBrowerRXData;
                        if (pos > -1) {
                            qDebug() << "requested version:" << regexp.cap(2) << "path:" << regexp.cap(3);
                            requestedPath = regexp.cap(3);
                        }
                    }

                    socket->write("HTTP/1.1 200 OK\r\n");       // \r needs to be before \n
                    //socket->write("Content-Type: text/qml\r\n");
                    //socket->write("Connection: close\r\n");
                    //socket->write("Refresh: 1\r\n\r\n");
                    socket->write("Cache-Control: no-cache, no-store, must-revalidate\r\n");
                    socket->write("Pragma: no-cache\r\n");
                    socket->write("Expires: 0\r\n");
                    socket->write("\r\n");
                    if(requestedPath != "") {
                        if(requestedPath == "?WatchdogServer") {
                            QString url = m_updateNotifyServer->serverUrl().toString();
                            qDebug() << "Tell client Watchdog Server:" << url;
                            socket->write(url.toLatin1(), url.length());
                        } else {
                            QFileInfo fileInfo("./" + requestedPath);
                            if(!fileInfo.exists()) {
                                qDebug() << "File does not exist";
                            } else {
                                qDebug() << "absolute file path:" << fileInfo.canonicalFilePath();
                                if(!fileInfo.canonicalFilePath().startsWith(m_currentDirectoryInfo.canonicalPath())) {
                                    qDebug() << "File not a subdirectory";
                                } else if(!fileInfo.isFile()) {
                                    qDebug() << "File is not a file";
                                } else if(!fileInfo.isReadable()) {
                                    qDebug() << "File cannot be read";
                                } else {
                                    QFile file(fileInfo.canonicalFilePath());
                                    file.open(QFile::ReadOnly);
                                    socket->write(file.readAll());
                                    file.close();
                                }
                            }
                        }
                    }
                    socket->flush();
                    socket->disconnectFromHost();
                });
                //while(!socket->waitForReadyRead(100));  //waiting for data to be read from web browser
            }
        });
        if(!m_directoryFileServer->listen(QHostAddress::Any, watchdogPort)) {
            qDebug() << "Web server could not start";
        } else {
            qDebug()<<"Watchdog running" << m_directoryFileServer->serverAddress();
        }
    }
private:
    QSet<QWebSocket*>   m_connectedClients;
    // version changes and is path of the url for all ressources.
    // This way Qml Component cache knows about new versions
    int                 m_currentVersion;
    QFileInfo           m_currentDirectoryInfo;
    QSet<QString>       m_changes;
    QTimer             *m_timer;
    QFileSystemWatcher *m_watcher;
    QWebSocketServer   *m_updateNotifyServer;
    QTcpServer         *m_directoryFileServer;
};

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    QGuiApplication a(argc, argv);

    QString dir(".");
    if(argc >= 2) {
        dir = argv[1];
    }
    HotReloadServer server(dir);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("./main.qml")));
    return a.exec();
}
