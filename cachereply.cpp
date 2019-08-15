#include "cachereply.h"
#include <QNetworkAccessManager>
#include <QBuffer>
#include <QTimer>
#include <QFile>
#include <QDir>

CacheReply::CacheReply( QObject *parent )
    : QNetworkReply(parent)
    , m_r(0)
    , m_w(0)
{
}

void CacheReply::setCacheFile(QString path)
{
    m_cacheFilePath = path;
}

void CacheReply::setNetworkReply(QNetworkReply *reply)
{
    m_reply = reply;
    QObject::connect(reply, &QNetworkReply::finished, this, &CacheReply::cacheAndFinish);
}

void CacheReply::abort()
{
    // NOOP
}

void CacheReply::cacheAndFinish()
{
    if(m_reply->error() == QNetworkReply::NoError) {
        // Cache what was returned from server
        if(m_cacheFilePath.isEmpty()) {
            qDebug() << "Could not cache reply";
        }
        QDir dir(m_cacheFilePath);
        dir.cdUp();
        if(!dir.mkpath(".")) {
            qDebug() << "Could not create parent directory for cache.";
        }
        QFile file(m_cacheFilePath);
        file.open(QIODevice::WriteOnly);
        if(!file.isWritable()) {
            qDebug() << "Could not write cache file.";
        }
        QByteArray memory = m_reply->readAll();
        file.write(memory);
        this->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        this->write(memory);
        //this->close();
    } else {
        if(!m_cacheFilePath.isEmpty()) {
            QFile requestedFileInCache(m_cacheFilePath);
            if(!requestedFileInCache.exists()) {
                qDebug() << "Cannot reach network and no fallback version exists";
            } else if(!requestedFileInCache.open(QIODevice::ReadOnly)) {
                qDebug() << "Cannot reach network and fallback cannot be read";
            }
            this->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
            this->write(requestedFileInCache.readAll());
        }
    }
    QTimer::singleShot( 0, this, &QNetworkReply::readyRead );
    QTimer::singleShot( 0, this, &QNetworkReply::finished );
}


bool CacheReply::isSequential() const
{
    return true;
}

bool CacheReply::open(OpenMode mode)
{
    return QNetworkReply::open(mode);
}

void CacheReply::close()
{
    return QNetworkReply::close();
}

qint64 CacheReply::pos() const
{
    return m_r;
}

qint64 CacheReply::size() const
{
    return m_content.size();
}

bool CacheReply::seek(qint64 pos)
{
    m_r = pos;
    m_w = pos;
    return true;
}

bool CacheReply::atEnd() const
{
    return m_r == m_content.size();
}

bool CacheReply::reset()
{
    m_r = 0;
    m_w = 0;
    //m_content.clear();
    QNetworkReply::reset();
    return m_buffer.reset();
}

qint64 CacheReply::bytesAvailable() const
{
    return m_content.size()-m_w;
}

qint64 CacheReply::bytesToWrite() const
{
    return 0;
}

bool CacheReply::canReadLine() const
{
    return m_r<m_content.size();
}

bool CacheReply::waitForReadyRead(int msecs)
{
    (void)msecs;
    return true;
}

bool CacheReply::waitForBytesWritten(int msecs)
{
    (void)msecs;
    return true;
}

qint64 CacheReply::readData(char *data, qint64 maxlen)
{
    qint64 number = qMin(maxlen, static_cast<qint64>(m_content.size())-m_r);
    memcpy(data, m_content.constData()+m_r, static_cast<size_t>(number));
    m_r += number;
    return number;
}

qint64 CacheReply::writeData(const char *data, qint64 len)
{
    m_content.resize(static_cast<int>(m_w+len));
    memcpy(m_content.data()+m_w, data, static_cast<size_t>(len));
    m_w += len;
    return len;
}
