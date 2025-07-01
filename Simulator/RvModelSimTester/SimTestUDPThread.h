#pragma once

#include "../RvModelSimShared/SimModelPackets.h"
#include "AppSettings.h"

#include <QBasicTimer>
#include <QHostAddress>
#include <QObject>
#include <QThread>
#include <QUdpSocket>

class SimTestUDPWorker : public QObject
{
	Q_OBJECT
public:
	explicit SimTestUDPWorker(QObject* parent = nullptr);

public slots:
	void getStat();                                            // Request server statistics
	void onSimControlMode(const QString& mode);                // Set simulation control mode

	void read(const QString& signalID);                        // Read value of a signal by ID
	void write(const QString& signalID, const QString& value); // Write value to a signal by ID

	void createSocket();                                       // Create and initialize UDP socket
	void createTimer();                                        // Create and start timer for periodic tasks

	void setShowServerState(bool enable);                      // Enable or disable showing server state
	void setValueType(SignalType type);                        // Set the type of value to use for signals


private slots:
	void onReadyRead();

private:
	QByteArray createRequestState(int packetType);                                // Create a request packet for state

	QByteArray createRequestRead(const QString& signalID);                        // Create a request packet to read a signal
	QByteArray createRequestWrite(const QString& signalID, const QString& value); // Create a request packet to write a signal

	void timerEvent(QTimerEvent* event) override;                                 // Handle timer events


signals:
	void resultReady(const QString& message);                                     // Send result to Log model
	void simStateReady(int errorCode, int stateCode);                             // Simulation state code


private:
	AppSettings m_settings;
	QBasicTimer m_timer;

	QUdpSocket* m_socket = nullptr;
	QString m_pendingSignalID = nullptr;
	SignalType m_valueType = SignalType::AnalogFloat;

	bool m_showServerState = true;
};

class SimTestUDPController : public QObject
{
	Q_OBJECT

public:
	SimTestUDPController();
	~SimTestUDPController();

public slots:
	void setShowServerState(bool enable);                             // Enable or disable showing server state


signals:
	void resultReady(const QString& action);
	void operateGetStat();                                            // Request to get server statistics
	void operateRead(const QString& signalID);                        // Request to read a signal
	void operateWrite(const QString& signalID, const QString& value); // Request to write a signal
	void simControlMode(const QString& mode);                         // Set simulation control mode
	void reloadSettings();                                            // Reload settings
	void showServerStateChanged(bool enable);                         // Show server state changes

	void simStateReady(int errorCode, int stateCode);                 // Simulation state code
	void setValueType(SignalType type);                               // Set the type of value to use for signals

private:
	QThread m_workerThread;
};
