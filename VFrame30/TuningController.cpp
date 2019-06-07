#include "TuningController.h"
#include "../lib/AppSignal.h"
#include "../lib/Tuning/ITuningSignalManager.h"
#include "../lib/Tuning/ITuningTcpClient.h"

namespace VFrame30
{

	TuningController::TuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QObject* parent) :
		QObject(parent),
		m_signalManager(signalManager),
		m_tcpClient(tcpClient)
	{
		assert(m_signalManager);
		return;
	}

	void TuningController::setTcpClient(ITuningTcpClient* tcpClient)
	{
		assert(tcpClient);
		m_tcpClient = tcpClient;

		return;
	}

	void TuningController::resetTcpClient()
	{
		m_tcpClient = nullptr;
	}

	AppSignalParam TuningController::signalParam(const QString& appSignalId, bool* ok)
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return AppSignalParam();
		}

		return m_signalManager->signalParam(appSignalId, ok);
	}

	TuningSignalState TuningController::signalState(const QString& appSignalId, bool* ok)
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return TuningSignalState();
		}

		return m_signalManager->state(appSignalId, ok);
	}

	QVariant TuningController::signalParam(const QString& appSignalId)
	{
		bool ok = true;
		QVariant result = QVariant::fromValue(signalParam(appSignalId, &ok));

		if (ok == false)
		{
			return QVariant();
		}

		return result;
	}

	QVariant TuningController::signalState(const QString& appSignalId)
	{
		bool ok = true;
		QVariant result = QVariant::fromValue(signalState(appSignalId, &ok));

		if (ok == false)
		{
			return QVariant();
		}

		return result;
	}

	bool TuningController::writeValue(QString appSignalId, QVariant value)
	{
		if (m_tcpClient == nullptr)
		{
			qDebug() << "Warning: Attempt to write tuning value while m_tcpClient is not set.";
			return false;
		}

		if (writingEnabled() == false)
		{
			return true;	// Access is denied, this is not an error
		}

		appSignalId = appSignalId.trimmed();

		bool ok = false;
		AppSignalParam appSignal = signalParam(appSignalId, &ok);

		if (ok == false)
		{
			return false;
		}

		switch (value.type())
		{
		case QVariant::Bool:
			if (appSignal.toTuningType() != TuningValueType::Discrete)
			{
				assert(false);	// Bool is allowed only for discrete signals
				return false;
			}
			break;

		case QVariant::Int:
			if (appSignal.toTuningType() == TuningValueType::Discrete)
			{
				value = value.toInt() == 0 ? false : true;	// Discrete signals can be set by 0 or 1
			}
			break;

		case QVariant::LongLong:
			assert(false);	// Must not arrive from script
			return false;

		case QMetaType::Float:
			assert(false);	// Must not arrive from script
			return false;

		case QVariant::Double:
			if (appSignal.toTuningType() == TuningValueType::Discrete)
			{
				value = value.toBool();	// Discrete signals can be set by 0.0 or 1.0
			}

			if (appSignal.toTuningType() == TuningValueType::SignedInt32)
			{
				value = value.toInt();
			}

			if (appSignal.toTuningType() == TuningValueType::Float)
			{
				value = value.toFloat();
			}
			break;

		default:
			assert(false);	// Some unknown type arrived from script
			return false;
		}

		TuningValue tuningValue(value);

		ok = m_tcpClient->writeTuningSignal(appSignalId, tuningValue);

		return ok;
	}

	bool TuningController::writingEnabled() const
	{
		return true;
	}

}
