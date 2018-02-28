#pragma once

#include <QObject>
#include "../lib/Tuning/ITuningSignalManager.h"
#include "../lib/Tuning/ITuningTcpClient.h"
#include "VFrame30Lib_global.h"

class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT TuningController : public QObject
	{
		Q_OBJECT

	public:
		TuningController() = delete;
		TuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QObject* parent = nullptr);

	public:
		AppSignalParam signalParam(const QString& appSignalId, bool* ok);
		TuningSignalState signalState(const QString& appSignalId, bool* ok);

		Q_INVOKABLE QVariant signalParam(const QString& appSignalId);	// If no signal with specified appSignalID found, QVariant is undefined
		Q_INVOKABLE QVariant signalState(const QString& appSignalId);	// If no signal with specified appSignalID found, QVariant is undefined

		Q_INVOKABLE bool writeValue(QString appSignalId, double value);

	private:
		ITuningSignalManager* m_signalManager = nullptr;
		ITuningTcpClient* m_tcpClient = nullptr;
	};

}
