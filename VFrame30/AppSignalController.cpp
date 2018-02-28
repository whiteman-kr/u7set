#include "AppSignalController.h"

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
			return AppSignalParam();
		}

		return m_appSignalManager->signalParam(signalHash, found);
	}

	AppSignalParam AppSignalController::signalParam(const QString& appSignalId, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return AppSignalParam();
		}

		return m_appSignalManager->signalParam(appSignalId, found);
	}

	AppSignalState AppSignalController::signalState(Hash signalHash, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return AppSignalState();
		}

		return m_appSignalManager->signalState(signalHash, found);
	}

	AppSignalState AppSignalController::signalState(const QString& appSignalId, bool* found) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(false);
			return AppSignalState();
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
	}

	QVariant ScriptAppSignalController::signalParam(QString signalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return QVariant();
		}

		bool ok = false;
		AppSignalParam s = m_appSignalManager->signalParam(signalId, &ok);

		if (ok == false)
		{
			return QVariant();
		}

		return QVariant::fromValue(s);
	}

	QVariant ScriptAppSignalController::signalParam(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return QVariant();
		}

		bool ok = false;
		AppSignalParam s = m_appSignalManager->signalParam(signalHash, &ok);

		if (ok == false)
		{
			return QVariant();
		}

		return QVariant::fromValue(s);
	}

	QVariant ScriptAppSignalController::signalState(QString signalId) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return QVariant();
		}

		bool ok = false;
		AppSignalState s = m_appSignalManager->signalState(signalId, &ok);

		if (ok == false)
		{
			return QVariant();
		}

		return QVariant::fromValue(s);
	}

	QVariant ScriptAppSignalController::signalState(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return QVariant();
		}

		bool ok = false;
		AppSignalState s = m_appSignalManager->signalState(signalHash, &ok);

		if (ok == false)
		{
			return QVariant();
		}

		return QVariant::fromValue(s);
	}

}
