#ifndef STATEFULHOTRELOADSERVER_H
#define STATEFULHOTRELOADSERVER_H

//#include <QtNetwork/QLocalSocket>

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
//// HotReloadServer is the Cpp part for Hot Code Reloading.
/// Copy paste it to your application or put it
/// as a seperate class. This way you can use your custom Cpp-Application
/// with the benefits of hot code reloading.
/// It will not be part of your production
/// release, so you can put an #ifdef aroudnd it
/// feel free to add caching and client update features
/// for procution.
///
/// HotReloadServer consists of a minimal Http-Server and a WebSocket Server
/// Your app is loaded over http. HotReloadServer watches for file changes
/// on the specified folder. When a change occurrs it notifies your app
/// about it via websocket and tells a new URL to relaod the App.
/// The App does not reload completely, it has the chance to carry over
/// the application-state. Qml Propertybindings do the rest of the magic.
/// In the Testapp you find a template for ApplicationState, that also serializes
/// and reloads when the application closes.

class StatefulHotReloadServer {
public:
    // This is what the client needs to know to reload
    // contains everything except for ip, which is already known
    QString versionPath() {
        qDebug() << "Server URL" << m_updateNotifyServer->serverUrl().toString();
        QString versionDir(":");
        versionDir.append(QString::number(m_directoryFileServer->serverPort()));
        versionDir.append("/v");
        versionDir.append(QString::number(m_currentVersion));
        versionDir.append("/");
        return versionDir;
    }
    bool addPath(QString path) {
        bool watching = m_watcher->addPath(path);
        if(watching) {
            qDebug() << "Watchdog watching:" << path;
            return true;
        } else {
            qDebug() << "Error: Cannot watch " << path;
            return false;
        }
    }
    bool addPathRecursively(QString path) {
        QFileInfo fi(path);
        if(m_skipDirs.contains(fi.canonicalFilePath()))
            return false;
        bool wasAdded = false;
        if(fi.isFile()) {
            if(!m_watcher->files().contains(path)) {
                wasAdded = addPath(path);
            }
        } else if(fi.isDir()) {
            if(!m_watcher->directories().contains(path)) {
                wasAdded = addPath(path);
                QDir dir(path);
                QStringList subdirsAndFiles = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
                for(auto iter = subdirsAndFiles.cbegin(); iter != subdirsAndFiles.cend() ; ++iter) {
                    addPathRecursively(dir.filePath(*iter));
                }
            }
        }
        return wasAdded;
    }
    // reloadDelay: If initializing qml takes long, make sure it is only reloaded once. Watchdog may fire multiple times
    // which results in multiple reloads if reloadDelay is too short. 500ms seems okay for QtCreator. 50ms if reloading qml multiple times is okay!
    StatefulHotReloadServer(QString dir, QStringList skipDirs, quint16 watchdogPort = 8080, quint16 updateNotifierPort = 8081, int reloadDelay = 50)
        :m_currentVersion(0)
        ,m_currentDirectoryInfo(dir)
        ,m_skipDirs(skipDirs)
        ,m_timer(new QTimer())
        ,m_watcher(new QFileSystemWatcher())
        ,m_updateNotifyServer(new QWebSocketServer("Watchdog update notifier", QWebSocketServer::NonSecureMode))
        ,m_directoryFileServer(new QTcpServer())
    {
        bool supportSsl = QSslSocket::supportsSsl();
        if(!supportSsl) {
            qDebug() << "No Ssl support!";
            QString sslLib = QSslSocket::sslLibraryBuildVersionString();
            qDebug() << "Please download and install OpenSSL (version info:" << sslLib << ")";
        }
        for(auto iter = skipDirs.cbegin() ; iter != skipDirs.cend() ; ++iter) {
            m_skipDirs.append(QFileInfo(*iter).canonicalFilePath());
        }
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
        QObject::connect(m_watcher, &QFileSystemWatcher::fileChanged, [this](const QString &path) {
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
                    char webBrowserRXData[3000];
                    QRegExp regexp("GET (/?v(\\d+)/)?(.*) HTTP/.*");
                    while(0 < socket->readLine(webBrowserRXData, 3000)) {
                        int pos = regexp.indexIn(webBrowserRXData);
                        //qDebug() << ":" << webBrowerRXData;
                        if (pos > -1) {
                            QFileInfo fileInfo(m_currentDirectoryInfo.filePath() + "/" + regexp.cap(3));
                            qDebug() << "requested version:" << regexp.cap(2) << "path:" << regexp.cap(3) << "(" << fileInfo.filePath() << ")";
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
                            QFileInfo fileInfo(m_currentDirectoryInfo.filePath() + "/" + requestedPath);
                            if(!fileInfo.exists()) {
                                if(fileInfo.fileName() != "qmldir") {
                                    qDebug() << "File does not exist (" + fileInfo.filePath() + ")";
                                } else {
                                    // emulate qmldir file from directory. Only if no qmldir-file exists!
                                    QDir dir(fileInfo.dir());
                                    QStringList filters;
                                    filters << "*.qml";
                                    dir.setNameFilters(filters);
                                    QStringList files(dir.entryList(QDir::Files));
                                    QString qmlDirFileString;
                                    for(int i=0 ; i < files.length() ; ++i) {
                                        qmlDirFileString.append(QFileInfo(files[i]).baseName() + " 1.0 " + files[i] + "\n");
                                    }
                                    socket->write(qmlDirFileString.toLatin1(), qmlDirFileString.length());
                                    qDebug() << "qmldir generated";
                                }
                            } else {
                                //qDebug() << "absolute file path:" << fileInfo.canonicalFilePath();
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
                    } else {
                        qDebug() << "Http header could not be parsed, no GET?";
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
    QStringList         m_skipDirs;
    QSet<QString>       m_changes;
    QTimer             *m_timer;
    QFileSystemWatcher *m_watcher;
    QWebSocketServer   *m_updateNotifyServer;
    QTcpServer         *m_directoryFileServer;
};

#endif // STATEFULHOTRELOADSERVER_H
