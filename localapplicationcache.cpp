#include "localapplicationcache.h"

#include <QNetworkReply>
#include <QStandardPaths>
#include <QDir>

#include "cachereply.h"

QString tryGetCacheFile(const QUrl &url) {
    QString foundFile;
    QString pathLatest = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    pathLatest.append("/hotreload/latest/");
    QDir dirLatest(pathLatest);
    if(!dirLatest.mkpath(".")) {
        qDebug() << "Could not create cache dir (" << dirLatest << ")";
        return "";
    }
    // remove version part in Urls. E.g. the /v234/ in 'myhost.com/v234/thefile.qml'
    QRegExp regexp("(/?v(\\d+)/)?(.*)");
    QString pathRequest = url.path();
    int pos = regexp.indexIn(pathRequest);
    if(pos > -1) {
        foundFile = dirLatest.canonicalPath() + "/" + regexp.cap(3);
    } else {
        qDebug() << "unknown request url format for Stateful Hot Reload";
        return "";
    }
    return foundFile;
}

LocalApplicationCache::LocalApplicationCache(QObject *parent)
    :QNetworkAccessManager(parent)
{
    //connect(this, &QNetworkAccessManager::finished, this, &LocalApplicationCache::cacheReply);
}


QNetworkReply *LocalApplicationCache::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QUrl url(request.url());
    QString scheme( url.scheme() );
    if( scheme.startsWith("cached")) {
        QNetworkRequest newRequest(request);
        QUrl urlHttp(request.url());
        urlHttp.setScheme(scheme.remove(0, 6));
        newRequest.setUrl(urlHttp);
        QString fileInCache = tryGetCacheFile(url);

        QNetworkReply *httpReply = QNetworkAccessManager::createRequest(op, newRequest, outgoingData);
        CacheReply *cacheReply = new CacheReply(this);
        cacheReply->setCacheFile(fileInCache);
        cacheReply->setNetworkReply(httpReply);
        return cacheReply;
    } else {
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }
}

void LocalApplicationCache::cacheReply(QNetworkReply *reply)
{
    qDebug() << "REPLY: " << reply->url();
}
