#ifndef TUNINGCONTROLLER_H
#define TUNINGCONTROLLER_H

#include <QObject>
#include "../lib/Tuning/ITuningSignalManager.h"
#include "../lib/Tuning/ITuningTcpClient.h"

class AppSignalParam;
class TuningSignalState;

class TuningController : public QObject
{
	Q_OBJECT

public:
	TuningController() = delete;
	TuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient);

	AppSignalParam signalParam(const QString& appSignalId, bool* ok);
	TuningSignalState signalState(const QString& appSignalId, bool* ok);

	Q_INVOKABLE QVariant signalParam(const QString& appSignalId);	// If no signal with specified appSignalID found, QVariant is undefined
	Q_INVOKABLE QVariant signalState(const QString& appSignalId);	// If no signal with specified appSignalID found, QVariant is undefined

	Q_INVOKABLE bool writeValue(QString appSignalId, double value);

private:
	TuningSignalManager* m_signalManager = nullptr;
	TuningTcpClient* m_tcpClient = nullptr;
};

#endif



//AppSignalParam signalParam(const QString& appSignalID, bool* ok);
//TuningSignalState signalState(const QString& appSignalID, bool* ok);

//Q_INVOKABLE QVariant signalParam(const QString& appSignalID);	// If no signal with specified appSignalID found, QVariant is undefined
//Q_INVOKABLE QVariant signalState(const QString& appSignalID);	// If no signal with specified appSignalID found, QVariant is undefined

//Q_INVOKABLE bool writeValue(QString appSignalID, float value);

//signals:
//void signal_writeValue(QString appSignalID, float value, bool* ok);
//void signal_getParam(QString appSignalID, AppSignalParam* result, bool* ok);
//void signal_getState(QString appSignalID, TuningSignalState* result, bool* ok);
