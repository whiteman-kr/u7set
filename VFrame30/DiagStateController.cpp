#include "DiagStateController.h"


namespace VFrame30
{

	//
	//	DiagStateController
	//
	DiagStateController::DiagStateController(IDiagStateManager& diagStateManager, QObject* parent /*= nullptr*/) :
		QObject(parent),
		m_diagStateManager(diagStateManager)
	{
	}

	//int AppSignalController::signalsCount() const
	//{
	//	return m_appSignalManager->signalsCount();
	//}

	//bool AppSignalController::signalExists(Hash hash) const
	//	return m_appSignalManager->signalExists(hash);
	//}

	//bool AppSignalController::signalExists(const QString& appSignalId) const
	//{
	//	return m_appSignalManager->signalExists(appSignalId);
	//}

	//AppSignalParam AppSignalController::signalParam(Hash signalHash, bool* found) const
	//{
	//	return m_appSignalManager->signalParam(signalHash, found);
	//}

	//AppSignalParam AppSignalController::signalParam(const QString& appSignalId, bool* found) const
	//{
	//	return m_appSignalManager->signalParam(appSignalId, found);
	//}

	//AppSignalState AppSignalController::signalState(Hash signalHash, bool* found) const
	//{
	//	return m_appSignalManager->signalState(signalHash, found);
	//}

	//AppSignalState AppSignalController::signalState(const QString& appSignalId, bool* found) const
	//{
	//	return m_appSignalManager->signalState(appSignalId, found);
	//}

	//void AppSignalController::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	//{
	//	return m_appSignalManager->signalState(appSignalHashes, result, found);
	//}

	//void AppSignalController::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
	//{
	//	return m_appSignalManager->signalState(appSignalIds, result, found);
	//}

	//QStringList AppSignalController::signalTags(Hash signalHash) const
	//{
	//	return m_appSignalManager->signalTags(signalHash);
	//}

	//QStringList AppSignalController::signalTags(const QString& appSignalId) const
	//{
	//	return signalTags(::calcHash(appSignalId));
	//}

	//bool AppSignalController::signalHasTag(Hash signalHash, const QString& tag) const
	//{
	//	return m_appSignalManager->signalHasTag(signalHash, tag);
	//}

	//bool AppSignalController::signalHasTag(const QString& appSignalId, const QString& tag) const
	//{
	//	return signalHasTag(::calcHash(appSignalId), tag);
	//}

	IDiagStateManager& DiagStateController::diagStateManager()
	{
		return m_diagStateManager;
	}

	const IDiagStateManager& DiagStateController::diagStateManager() const
	{
		return m_diagStateManager;
	}


	//
	//	ScriptAppSignalController
	//
	ScriptDiagStateController::ScriptDiagStateController(const IDiagStateManager& diagStateManager, QObject* parent) :
		QObject(parent),
		m_diagStateManager(diagStateManager)
	{
		qDebug() << "ScriptDiagStateController::ScriptDiagStateController";
	}

	ScriptDiagStateController::~ScriptDiagStateController()
	{
		qDebug() << "ScriptDiagStateController::~ScriptDiagStateController()";
	}
//
//	int ScriptAppSignalController::signalsCount() const
//	{
//`		return m_appSignalManager->signalsCount();
//	}
//
//	QJSValue ScriptAppSignalController::signalParam(QString signalId) const
//	{
//		return signalParam(::calcHash(signalId));
//	}
//
//	QJSValue ScriptAppSignalController::signalParam(Hash signalHash) const
//	{
//`		bool ok = false;
//		AppSignalParam s = m_appSignalManager->signalParam(signalHash, &ok);
//
//		if (ok == false)
//		{
//			return {};
//		}
//
//		QJSEngine* engine = qjsEngine(this);
//
//		if (engine == nullptr)
//		{
//			Q_ASSERT(engine);
//			return {};
//		}
//
//		return engine->toScriptValue(s);
//	}
//
//	QString ScriptAppSignalController::equipmentToAppSignalId(QString equipmentId) const
//	{
//		return m_appSignalManager->equipmentToAppSignalId(equipmentId);
//	}
//
//	QString ScriptAppSignalController::equipmentToAppSiganlId(QString equipmentId) const
//	{
//		return equipmentToAppSignalId(equipmentId);
//	}
//
//	QJSValue ScriptAppSignalController::signalState(QString signalId) const
//	{
//		return signalState(::calcHash(signalId));
//	}
//
//	QJSValue ScriptAppSignalController::signalState(Hash signalHash) const
//	{
//		bool ok = false;
//		AppSignalState s = m_appSignalManager->signalState(signalHash, &ok);
//
//		if (ok == false)
//		{
//			return {};
//		}
//
//		QJSEngine* engine = qjsEngine(this);
//
//		if (engine == nullptr)
//		{
//			Q_ASSERT(engine);
//			return {};
//		}
//
//		return engine->toScriptValue(s);
//	}
//
//	QJSValueList ScriptAppSignalController::signalStates(QStringList signalIds) const
//	{
//		QJSValueList result;
//
//		// --
//		//
//		std::vector<QString> appSignalIds{signalIds.begin(), signalIds.end()};
//		std::vector<AppSignalState> states;
//
//		m_appSignalManager->signalState(appSignalIds, &states, nullptr);
//
//		// --
//		//
//		QJSEngine* engine = qjsEngine(this);
//		if (engine == nullptr)
//		{
//			Q_ASSERT(engine);
//			return result;
//		}
//
//		result.reserve(signalIds.size());
//		for (const auto& state : states)
//		{
//			result.push_back(engine->toScriptValue(state));
//		}
//
//		return result;
//	}
//
//	bool ScriptAppSignalController::signalExists(QString signalId) const
//	{
//		return m_appSignalManager->signalExists(::calcHash(signalId));
//	}
//
//	bool ScriptAppSignalController::signalsExist(QStringList signalIds)
//	{
//		return m_appSignalManager->signalsExist(signalIds);
//	}
//
//	bool ScriptAppSignalController::isDiscrete(QString signalId) const
//	{
//		bool ok = false;
//		E::SignalType type = m_appSignalManager->signalType(::calcHash(signalId), &ok);
//
//		return ok ?
//					(type == E::SignalType::Discrete) :
//					false;
//	}
//
//	bool ScriptAppSignalController::isAnalog(QString signalId) const
//	{
//		bool ok = false;
//		E::SignalType type = m_appSignalManager->signalType(::calcHash(signalId), &ok);
//
//		return ok ?
//					(type == E::SignalType::Analog) :
//					false;
//	}
//
//	int ScriptAppSignalController::precision(QString signalId) const
//	{
//		bool ok = false;
//
//		AppSignalParam asp = m_appSignalManager->signalParam(::calcHash(signalId), &ok);
//
//		return (ok == true && asp.isAnalog() == true) ? asp.precision() : 0;
//	}
//
//	QStringList ScriptAppSignalController::signalIdsByTag(QString tag) const
//	{
//		return m_appSignalManager->signalIdsByTag(tag);
//	}

}
