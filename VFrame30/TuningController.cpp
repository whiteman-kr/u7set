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
		assert(m_tcpClient);

		return;
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

	bool TuningController::writeValue(QString appSignalId, double value)
	{
		if (m_tcpClient == nullptr)
		{
			assert(m_tcpClient);
			return false;
		}

		appSignalId = appSignalId.trimmed();

		bool ok = false;
		AppSignalParam appSignal = signalParam(appSignalId, &ok);

		if (ok == false)
		{
			return false;
		}

		TuningValue tuningValue;

		tuningValue.setType(appSignal.toTuningType());

		switch (tuningValue.type())
		{
		case TuningValueType::Discrete:
			tuningValue.setInt32Value(static_cast<qint32>(value));
			break;

		case TuningValueType::SignedInt32:
			tuningValue.setInt32Value(static_cast<qint32>(value));
			break;

		case TuningValueType::Float:
			tuningValue.setFloatValue(static_cast<float>(value));
			break;

		default:
			assert(false);
			return false;
		}

		ok = m_tcpClient->writeTuningSignal(appSignalId, tuningValue);

		return ok;
	}

}
