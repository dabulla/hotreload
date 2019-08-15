#ifndef LOCALAPPLICATIONCACHE_H
#define LOCALAPPLICATIONCACHE_H

#include <QNetworkAccessManager>

class LocalApplicationCache : public QNetworkAccessManager
{
    Q_OBJECT
public:
    LocalApplicationCache(QObject *parent = nullptr);

    // QNetworkAccessManager interface
protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);
protected slots:
    void cacheReply(QNetworkReply *reply);
};

#endif // LOCALAPPLICATIONCACHE_H
