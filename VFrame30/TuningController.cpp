#include "TuningController.h"
#include "../AppSignalLib/AppSignalParam.h"
#include "../AppSignalLib/ITuningSignalManager.h"

namespace VFrame30
{
	TuningController::TuningController(ITuningSignalManager* signalManager, ITuningConnection* tuningConnection, QWidget* parent) :
		QObject(parent),
		m_signalManager(signalManager),
		m_tuningConnection(tuningConnection)
	{
		assert(m_signalManager);
		return;
	}

	AppSignalParam TuningController::signalParam(const QString& appSignalId, bool* ok) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalParam(appSignalId, ok);
	}

	TuningSignalState TuningController::signalState(const QString& appSignalId, bool* ok) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->state(appSignalId, ok);
	}

	QVariant TuningController::signalParam(const QString& appSignalId) const
	{
		bool ok = true;
		QVariant result = QVariant::fromValue(signalParam(appSignalId, &ok));

		if (ok == false)
		{
			return QVariant();
		}

		return result;
	}

	QVariant TuningController::signalState(const QString& appSignalId) const
	{
		bool ok = true;
		QVariant result = QVariant::fromValue(signalState(appSignalId, &ok));

		if (ok == false)
		{
			return QVariant();
		}

		return result;
	}


	QJSValueList TuningController::signalStates(QStringList appSignalIds) const
	{
		QJSValueList result;

		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return result;
		}

		// --
		//
		std::vector<QString> signalIds{appSignalIds.begin(), appSignalIds.end()};
		std::vector<TuningSignalState> states;

		m_signalManager->state(signalIds, &states, nullptr);

		// --
		//
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return result;
		}

		result.reserve(signalIds.size());
		for (const auto& state : states)
		{
			result.push_back(engine->toScriptValue(state));
		}

		return result;
	}

	bool TuningController::signalExists(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalExists(::calcHash(signalId));
	}

	bool TuningController::signalsExist(QStringList signalIds)
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalsExist(signalIds);
	}

	bool TuningController::isDiscrete(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_signalManager->signalParam(::calcHash(signalId), &ok);

		return ok ?
					(asp.type() == E::SignalType::Discrete) :
					false;
	}

	bool TuningController::isAnalog(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_signalManager->signalParam(::calcHash(signalId), &ok);

		return ok ?
					(asp.type() == E::SignalType::Analog) :
					false;
	}

	int TuningController::precision(QString signalId) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_signalManager->signalParam(::calcHash(signalId), &ok);

		return (ok == true && asp.isAnalog() == true) ? asp.precision() : 0;
	}

	QStringList TuningController::signalIdsByTag(QString tag) const
	{
		if (m_signalManager == nullptr)
		{
			assert(m_signalManager);
			return {};
		}

		return m_signalManager->signalIdsByTag(tag);
	}

	bool TuningController::writeValue(QString appSignalId, QVariant value)
	{
		if (checkTuningAccess() == false)
		{
			return true;	// Access is denied, this is not an error
		}

		return m_tuningConnection->writeTuningSignal(appSignalId, value);
	}

	bool TuningController::writeValueBool(QString appSignalId, bool value)
	{
		return writeValue(appSignalId, value);
	}

	bool TuningController::writeValueInt(QString appSignalId, int value)
	{
		return writeValue(appSignalId, value);
	}

	bool TuningController::writeValueDouble(QString appSignalId, double value)
	{
		return writeValue(appSignalId, value);
	}

	void TuningController::apply()
	{
		if (checkTuningAccess() == false)
		{
			return;	// Access is denied, this is not an error
		}

		m_tuningConnection->applyTuningSignals();

		return;
	}

	bool TuningController::checkTuningAccess() const
	{
		return true;
	}

}
