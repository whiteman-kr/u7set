/***************************************************************************
 *   Copyright (C) 2010 by NociSoft Software Solutions   
 *   support@nocisoft.com   
 ***************************************************************************/

#ifndef NCREPORTHTTPCLIENT_H
#define NCREPORTHTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QSslError;
class QAuthenticator;
class QNetworkReply;
QT_END_NAMESPACE

class NCReportHttpClient : public QObject
{
Q_OBJECT
public:
    explicit NCReportHttpClient(QObject *parent = 0);
	//NCReportHttpClient(QObject *parent, const QObject* sender, const char* signal );

	void downloadContent( const QString& urlText );
	void waitForDownloadContent( const QString& urlText );
	QString errorMessage() const;
	int error() const;
	const QByteArray& content() const;
	void clear();

private slots:
	void cancelDownload();
	void httpFinished();
	void httpReadyRead();
	void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
	void slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);
#ifndef QT_NO_OPENSSL
	void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif

signals:
	void readBytes(qint64 bytesRead, qint64 totalBytes);
	void finished();

private:
	QUrl url;
	QNetworkAccessManager qnam;
	QNetworkReply *reply;
	QByteArray data;
	int httpGetId;
	bool httpRequestAborted;
	bool ready;
	QString errorMsg;
	int errorCode;

	void startRequest(QUrl url);
	void setError( const QString& msg, int code );
};

#endif // NCREPORTHTTPCLIENT_H
