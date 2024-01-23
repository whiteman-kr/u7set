#include "AppSignalController.h"
#include "../AppSignalLib/ComparatorSet.h"

namespace VFrame30
{

	//
	//	AppSignalController
	//
	AppSignalController::AppSignalController(IAppSignalManager* appSignalManager, QObject* parent /*= nullptr*/) :
		QObject(parent),
		m_appSignalManager(appSignalManager)
	{
		assert(m_appSignalManager);
	}

	int AppSignalController::signalsCount() const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return false;
		}

		return m_appSignalManager->signalsCount();
	}

	bool AppSignalController::signalExists(Hash hash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return false;
		}

		return m_appSignalManager->signalExists(hash);
	}

	bool AppSignalController::signalExists(const QString& appSignalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return false;
		}

		return m_appSignalManager->signalExists(appSignalId);
	}

	AppSignalParam AppSignalController::signalParam(Hash signalHash, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return {};
		}

		return m_appSignalManager->signalParam(signalHash, found);
	}

	AppSignalParam AppSignalController::signalParam(const QString& appSignalId, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return {};
		}

		return m_appSignalManager->signalParam(appSignalId, found);
	}

	AppSignalState AppSignalController::signalState(Hash signalHash, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return {};
		}

		return m_appSignalManager->signalState(signalHash, found);
	}

	AppSignalState AppSignalController::signalState(const QString& appSignalId, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return {};
		}

		return m_appSignalManager->signalState(appSignalId, found);
	}

	void AppSignalController::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return;
		}

		return m_appSignalManager->signalState(appSignalHashes, result, found);
	}

	void AppSignalController::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return;
		}

		return m_appSignalManager->signalState(appSignalIds, result, found);
	}

	QStringList AppSignalController::signalTags(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return {};
		}

		return m_appSignalManager->signalTags(signalHash);
	}

	QStringList AppSignalController::signalTags(const QString& appSignalId) const
	{
		return signalTags(::calcHash(appSignalId));
	}

	bool AppSignalController::signalHasTag(Hash signalHash, const QString& tag) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return false;
		}

		return m_appSignalManager->signalHasTag(signalHash, tag);
	}

	bool AppSignalController::signalHasTag(const QString& appSignalId, const QString& tag) const
	{
		return signalHasTag(::calcHash(appSignalId), tag);
	}

	std::vector<std::shared_ptr<Comparator>> AppSignalController::setpointsByInputSignalId(const QString& appSignalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			Q_ASSERT(m_appSignalManager);
			return {};
		}

		return m_appSignalManager->setpointsByInputSignalId(appSignalId);
	}

	IAppSignalManager* AppSignalController::appSignalManager()
	{
		return m_appSignalManager;
	}

	const IAppSignalManager* AppSignalController::appSignalManager() const
	{
		return m_appSignalManager;
	}

	//
	//	ScriptAppSignalController
	//
	ScriptAppSignalController::ScriptAppSignalController(const IAppSignalManager* appSignalManager, QObject* parent) :
		QObject(parent),
		m_appSignalManager(appSignalManager)
	{
		assert(m_appSignalManager);
		qDebug() << "ScriptAppSignalController::ScriptAppSignalController";
	}

	ScriptAppSignalController::~ScriptAppSignalController()
	{
		qDebug() << "ScriptAppSignalController::~ScriptAppSignalController()";
	}

	int ScriptAppSignalController::signalsCount() const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return 0;
		}

		return m_appSignalManager->signalsCount();
	}

	QJSValue ScriptAppSignalController::signalParam(QString signalId) const
	{
		return signalParam(::calcHash(signalId));
	}

	QJSValue ScriptAppSignalController::signalParam(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;
		AppSignalParam s = m_appSignalManager->signalParam(signalHash, &ok);

		if (ok == false)
		{
			return {};
		}

		QJSEngine* engine = qjsEngine(this);

		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		return engine->toScriptValue(s);
	}

	QString ScriptAppSignalController::equipmentToAppSignalId(QString equipmentId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		return m_appSignalManager->equipmentToAppSignalId(equipmentId);
	}

	QString ScriptAppSignalController::equipmentToAppSiganlId(QString equipmentId) const
	{
		return equipmentToAppSignalId(equipmentId);
	}

	QJSValue ScriptAppSignalController::signalState(QString signalId) const
	{
		return signalState(::calcHash(signalId));
	}

	QJSValue ScriptAppSignalController::signalState(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;
		AppSignalState s = m_appSignalManager->signalState(signalHash, &ok);

		if (ok == false)
		{
			return {};
		}

		QJSEngine* engine = qjsEngine(this);

		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		return engine->toScriptValue(s);
	}

	QJSValueList ScriptAppSignalController::signalStates(QStringList signalIds) const
	{
		QJSValueList result;

		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return result;
		}

		// --
		//
		std::vector<QString> appSignalIds{signalIds.begin(), signalIds.end()};
		std::vector<AppSignalState> states;

		m_appSignalManager->signalState(appSignalIds, &states, nullptr);

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

	bool ScriptAppSignalController::signalExists(QString signalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		return m_appSignalManager->signalExists(::calcHash(signalId));
	}

	bool ScriptAppSignalController::signalsExist(QStringList signalIds)
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		return m_appSignalManager->signalsExist(signalIds);
	}

	bool ScriptAppSignalController::isDiscrete(QString signalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;
		E::SignalType type = m_appSignalManager->signalType(::calcHash(signalId), &ok);

		return ok ?
					(type == E::SignalType::Discrete) :
					false;
	}

	bool ScriptAppSignalController::isAnalog(QString signalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;
		E::SignalType type = m_appSignalManager->signalType(::calcHash(signalId), &ok);

		return ok ?
					(type == E::SignalType::Analog) :
					false;
	}

	int ScriptAppSignalController::precision(QString signalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;

		AppSignalParam asp = m_appSignalManager->signalParam(::calcHash(signalId), &ok);

		return (ok == true && asp.isAnalog() == true) ? asp.precision() : 0;
	}

	QStringList ScriptAppSignalController::signalIdsByTag(QString tag) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		return m_appSignalManager->signalIdsByTag(tag);
	}

}
