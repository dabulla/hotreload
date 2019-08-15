#include "localapplicationcachefactory.h"
#include "localapplicationcache.h"

QNetworkAccessManager *LocalApplicationCacheFactory::create(QObject *parent)
{
    return new LocalApplicationCache(parent);
}
