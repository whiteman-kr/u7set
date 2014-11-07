#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "../qtservice/src/qtservice.h"
#include "../include/UdpSocket.h"
#include "../include/CircularLogger.h"






class BaseServiceController;

class ReceivedFile
{
private:
	static quint32 m_serialID;

	quint32 m_ID = 0;
	QString m_fileName;
	quint32 m_fileSize = 0;
	quint32 m_CRC32 = CRC32_INITIAL_VALUE;
	quint32 m_dataSize = 0;
	char* m_data = nullptr;

public:
	ReceivedFile(const QString& fileName, quint32 fileSize);
	~ReceivedFile();

	quint32 ID() { return m_ID; }

	bool appendData(const char *ptr, quint32 len);

	QString fileName() const { return m_fileName; }

	quint32 CRC32();
};


// BaseServiceWorker class
//

class BaseServiceWorker : public QObject
{
    Q_OBJECT

private:
	int m_serviceType = STP_BASE;

	BaseServiceController* m_baseServiceController;

	CircularLogger& m_log;

	UdpSocketThread* m_serverSocketThread = nullptr;

	// members needed for file sending
	//
	UdpSocketThread* m_sendFileClientSocketThread = nullptr;

	QFile m_fileToSend;
	QFileInfo m_fileToSendInfo;

	SendFileStart m_sendFileStart;
	SendFileNext m_sendFileNext;

	bool m_sendFileReadNextPartOK = false;
	bool m_sendFileFirstRead = true;

	bool sendFileReadNextPart();
	void stopSendFile(quint32 errorCode);

	// members needed for file receiving
	//
	QHash<quint32, ReceivedFile*> m_receivedFile;

	void onSendFileStartRequest(const UdpRequest &request, UdpRequest &ack);
	void onSendFileNextRequest(const UdpRequest &request, UdpRequest &ack);

public:
    BaseServiceWorker(BaseServiceController* baseServiceController, int serviceType);
    virtual ~BaseServiceWorker();

	virtual void threadStarted() {}
	virtual void threadFinished() {}

signals:
    void ackBaseRequest(UdpRequest request);

	void startMainFunction();
	void stopMainFunction();
	void restartMainFunction();

	void endSendFile(bool result, QString fileName);
	void fileReceived(ReceivedFile* receivedFile);

	void sendFileRequest(UdpRequest request);

private slots:
	void onSendFileAckReceived(UdpRequest udpRequest);
	void onSendFileAckTimeout();

public slots:
	void onThreadStarted();
	void onThreadFinished();

    void onBaseRequest(UdpRequest request);

	void onSendFile(QHostAddress address, quint16 port, QString fileName);
	void onFreeReceivedFile(quint32 fileID);
};


// MainFunctionWorker class
//

class MainFunctionWorker : public QObject
{
	Q_OBJECT

private:
	BaseServiceController* m_baseServiceController = nullptr;

public:

	MainFunctionWorker(BaseServiceController* baseServiceController);
	virtual ~MainFunctionWorker();

	virtual void threadStarted() { QThread::sleep(2); qDebug() << "Called MainFunctionWorker::threadStarted"; }
	virtual void threadFinished() { QThread::sleep(2); qDebug() << "Called MainFunctionWorker::threadFinished"; }

signals:
	void mainFunctionWork();
	void mainFunctionStopped();

public slots:
	void onThreadStartedSlot();
	void onThreadFinishedSlot();
};


// BaseServiceController class
//


class BaseServiceController : public QObject
{
    Q_OBJECT

public:
    enum MainFunctionState
    {
		stopped = SS_MF_STOPPED,
		starts = SS_MF_STARTS,
		work = SS_MF_WORK,
		stops = SS_MF_STOPS
    };

private:
    QMutex m_mutex;

    QThread m_baseWorkerThread;
    QThread m_mainFunctionThread;

	bool m_mainFunctionNeedRestart;

	bool m_mainFunctionStopped;

	int m_serviceType;

    qint64 m_serviceStartTime;
    qint64 m_mainFunctionStartTime;

    MainFunctionState m_mainFunctionState;

    quint32 m_majorVersion;
    quint32 m_minorVersion;
    quint32 m_buildNo;
	quint32 m_crc;

	QTimer m_timer500ms;

	void checkMainFunctionState();

signals:
	void sendFile(QHostAddress address, quint16 port, QString fileName);

	void freeReceivedFile(quint32 fileID);

public slots:
	void stopMainFunction();
	void startMainFunction();
	void restartMainFunction();

	virtual void onEndSendFile(bool result, QString fileName);
	virtual void onFileReceived(ReceivedFile* receivedFile);

private slots:
	void onTimer500ms();

	void onMainFunctionWork();
	void onMainFunctionStopped();

public:
	CircularLogger log;

    BaseServiceController(int serviceType);
    virtual ~BaseServiceController();

	void getServiceInfo(ServiceInformation& serviceInfo);

	virtual void initLog();
};


