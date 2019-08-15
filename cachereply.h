#ifndef CACHEREPLY_H
#define CACHEREPLY_H

#include <QNetworkReply>
#include <QtCore/QBuffer>

class CacheReply : public QNetworkReply
{
    Q_OBJECT
public:
    CacheReply( QObject *parent = nullptr );

    void setCacheFile(QString path);
    void setNetworkReply(QNetworkReply *reply);
public slots:
    void cacheAndFinish();

protected:
    QString m_cacheFilePath;
    QNetworkReply *m_reply;
    QBuffer m_buffer;
    QByteArray m_content;
    qint64 m_r;
    qint64 m_w;

    // QNetworkReply interface
public slots:
    virtual void abort() override;

    // QIODevice interface
public:
    virtual bool isSequential() const override;
    virtual bool open(OpenMode mode) override;
    virtual void close() override;
    virtual qint64 pos() const override;
    virtual qint64 size() const override;
    virtual bool seek(qint64 pos) override;
    virtual bool atEnd() const override;
    virtual bool reset() override;
    virtual qint64 bytesAvailable() const override;
    virtual qint64 bytesToWrite() const override;
    virtual bool canReadLine() const override;
    virtual bool waitForReadyRead(int msecs) override;
    virtual bool waitForBytesWritten(int msecs) override;

protected:
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char *data, qint64 len) override;
};

#endif // CACHEREPLY_H
