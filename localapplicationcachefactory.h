#ifndef LOCALAPPLICATIONCACHEFACTORY_H
#define LOCALAPPLICATIONCACHEFACTORY_H

#include <QQmlNetworkAccessManagerFactory>

class LocalApplicationCacheFactory : public QQmlNetworkAccessManagerFactory
{
public:
    virtual QNetworkAccessManager *create(QObject *parent = nullptr);
};

#endif // LOCALAPPLICATIONCACHEFACTORY_H
