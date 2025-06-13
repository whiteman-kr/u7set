#pragma once

#include "AppSettings.h"
#include "SimModelPackets.h"

#include <QHostAddress>
#include <QJsonObject>
#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QBasicTimer>



class SimTestUDPWorker : public QObject
{
	Q_OBJECT
public:
	explicit SimTestUDPWorker(QObject* parent = nullptr);

public slots:
	void getStat();
	void onSimControlMode(const QString& mode);
	void read(const QString& signalID);
	void write(const QString& signalID, const QString& value);
	void createSocket();
	void createTimer();

	void setShowServerState(bool enable);
	void setValueType(SignalType type);


private slots:
	void onReadyRead();

private:
	QByteArray createRequestState(int packetType);
	QByteArray createRequestRead(const QString& signalID);
	QByteArray createRequestWrite(const QString& signalID, const QString& value);
	void timerEvent(QTimerEvent* event) override;
	SignalType m_valueType = SignalType::AnalogFloat;

signals:
	void resultReady(const QString& message);
	void simStateReady(int errorCode, int stateCode);


private:
	AppSettings m_settings;
	QUdpSocket* m_socket = nullptr;
	QBasicTimer m_timer;

	bool m_showServerState = true;
	QString m_pendingSignalID;
};

class SimTestUDPController : public QObject
{
	Q_OBJECT
	QThread workerThread;

public:
	SimTestUDPController();
	~SimTestUDPController();

public slots:
	void setShowServerState(bool enable);
	

signals:
	void resultReady(const QString& action);
	void operateGetStat();
	void operateRead(const QString& signalID);
	void operateWrite(const QString& signalID, const QString& value);
	void simControlMode(const QString& mode);
	void reloadSettings();
	void showServerStateChanged(bool enable);

	void simStateReady(int errorCode, int stateCode);
	void setValueType(SignalType type);
};