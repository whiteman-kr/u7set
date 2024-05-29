#include <VFrame30/TuningController.h>
#include "../AppSignalLib/ITuningSignalManager.h"

namespace VFrame30
{
	TuningController::TuningController(ITuningSignalManager& signalManager, ITuningConnection& tuningConnection, ITuningAuthorization& tuningAuthorization, QWidget* parent) :
		QObject(parent),
		m_parent(parent),
		m_signalManager(signalManager),
		m_tuningConnection(tuningConnection),
		m_tuningAuthorization(tuningAuthorization)
	{
		return;
	}

	AppSignalParam TuningController::signalParam(const QString& appSignalId, bool* ok) const
	{
		return m_signalManager.signalParam(appSignalId, ok);
	}

	TuningSignalState TuningController::signalState(const QString& appSignalId, bool* ok) const
	{
		return m_signalManager.state(appSignalId, ok);
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

		// --
		//
		std::vector<QString> signalIds{appSignalIds.begin(), appSignalIds.end()};
		std::vector<TuningSignalState> states;

		m_signalManager.state(signalIds, &states, nullptr);

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
		return m_signalManager.signalExists(::calcHash(signalId));
	}

	bool TuningController::signalsExist(QStringList signalIds)
	{
		return m_signalManager.signalsExist(signalIds);
	}

	bool TuningController::isDiscrete(QString signalId) const
	{
		bool ok = false;

		AppSignalParam asp = m_signalManager.signalParam(::calcHash(signalId), &ok);

		return ok ?
					(asp.type() == E::SignalType::Discrete) :
					false;
	}

	bool TuningController::isAnalog(QString signalId) const
	{
		bool ok = false;

		AppSignalParam asp = m_signalManager.signalParam(::calcHash(signalId), &ok);

		return ok ?
					(asp.type() == E::SignalType::Analog) :
					false;
	}

	int TuningController::precision(QString signalId) const
	{
		bool ok = false;

		AppSignalParam asp = m_signalManager.signalParam(::calcHash(signalId), &ok);

		return (ok == true && asp.isAnalog() == true) ? asp.precision() : 0;
	}

	QStringList TuningController::signalIdsByTag(QString tag) const
	{
		return m_signalManager.signalIdsByTag(tag);
	}

	bool TuningController::writeValue(QString appSignalId, QVariant value)
	{
		if (m_tuningAuthorization.login(m_parent) == false)
		{
			return false;
		}

		return m_tuningConnection.writeTuningSignal(appSignalId, value);
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

	bool TuningController::apply()
	{
		if (m_tuningAuthorization.login(m_parent) == false)
		{
			return false;
		}

		m_tuningConnection.applyTuningSignals();

		return true;
	}

	bool TuningController::tuningLogin() const
	{
		return m_tuningAuthorization.enabled();
	}

	bool TuningController::isLoggedIn() const
	{
		if (m_tuningAuthorization.enabled() == true)
		{
			return m_tuningAuthorization.isLoggedIn();
		}

		return true;
	}

	QString TuningController::userName() const
	{
		return m_tuningAuthorization.userName();
	}

	QStringList TuningController::userTags() const
	{
		return m_tuningAuthorization.userTags();
	}
}
