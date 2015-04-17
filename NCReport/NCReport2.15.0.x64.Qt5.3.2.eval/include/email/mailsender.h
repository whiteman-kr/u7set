#ifndef LMAILSENDER_H
#define LMAILSENDER_H

#include <QString>
#include <QStringList>
#include <QThread>
#include <QPointer>

#include <QTcpSocket>
#include <QAuthenticator>

#include "../ncreport_global.h"

class LMailSenderPrivate;

class NCREPORTSHARED_EXPORT LMailSender : public QThread
{
    Q_OBJECT

public:
    enum Priority {
        High,
        Normal,
        Low
    };

    enum ContentType {
        Text,
        Html,
        MultiPartMixed
    };

    enum Encoding {
        _7bit,
        _8bit,
        base64
    };

    enum ISO {
        Utf8,
        ISO88591
    };

    enum SendMode {
        SerialMode,
        ThreadMode
    };

    LMailSender(bool ssl = false, QObject* parent = 0);
    LMailSender(bool ssl, SendMode mode, QObject* parent = 0);
    virtual ~LMailSender();

private:
    Q_DECLARE_PRIVATE(LMailSender);
    QScopedPointer<LMailSenderPrivate> d_ptr;

private:
    void run();
    void start() { QThread::start(); }

private slots:
    void errorReceived(QAbstractSocket::SocketError socketError);
    void proxyAuthentication(const QNetworkProxy& proxy, QAuthenticator* authenticator);

public:
    bool send();
    QString lastError();
    QString lastCmd();
    QString lastResponse();
    QString lastMailData();

    void setSmtpServer(const QString& smtpServer);
    void setPort(int port);
    void setTimeout(int timeout);
    void setLogin(const QString& login, const QString& passwd);
    void setSsl(bool ssl);
    void setCc(const QStringList& cc);
    void setBcc(const QStringList& bcc);
    void setAttachment(const QString& filename, const QByteArray& data);
    void setReplyTo(const QString& replyTo);
    void setPriority(Priority priority);
    void setFrom(const QString& from);
    void setTo(const QStringList& to);
    void setSubject(const QString& subject);
    void setBody(const QString& body);
    void setFromName(const QString& fromName);
    void setContentType(ContentType contentType);
    void setISO(ISO iso);
    void setEncoding(Encoding encoding);
    void setProxyAuthenticator(const QAuthenticator& authenticator);

    QStringList to() const;
    QString from() const;
};

#endif
