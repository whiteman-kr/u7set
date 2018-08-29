#ifndef APPSIGNALCONTROLLER_H
#define APPSIGNALCONTROLLER_H

#include <QObject>
#include "VFrame30Lib_global.h"
#include "../lib/IAppSignalManager.h"

class AppSignalParam;
class AppSignalState;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT AppSignalController : public QObject
	{
		Q_OBJECT

	public:
		AppSignalController() = delete;
		explicit AppSignalController(IAppSignalManager* appSignalManager, QObject* parent = nullptr);

	public:
		bool signalExists(Hash hash) const;
		bool signalExists(const QString& appSignalId) const;

		AppSignalParam signalParam(Hash signalHash, bool* found) const;
		AppSignalParam signalParam(const QString& appSignalId, bool* found) const;

		AppSignalState signalState(Hash signalHash, bool* found) const;
		AppSignalState signalState(const QString& appSignalId, bool* found) const;

		void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const;
		void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const;

	public:
		IAppSignalManager* appSignalManager();
		const IAppSignalManager* appSignalManager() const;

	private:
		IAppSignalManager* m_appSignalManager = nullptr;
	};

	// This class used for scripts, it is created to separate AppSignalController from ugly
	// script decorations like Q_INVOKABLE and from mixing QVariant with actual return types AppSignalParam/State
	//
	class ScriptAppSignalController : public QObject
	{
		Q_OBJECT

	public:
		explicit ScriptAppSignalController(const IAppSignalManager* appSignalManager, QObject* parent = nullptr);

		// Script Interface
		//
	public slots:
		QVariant signalParam(QString signalId) const;		// Returns AppSignalParam
		QVariant signalParam(Hash signalHash) const;		// Returns AppSignalParam

		QVariant signalState(QString signalId) const;		// Returns AppSignalState
		QVariant signalState(Hash signalHash) const;		// Returns AppSignalState

		// Data
		//
	private:
		const IAppSignalManager* m_appSignalManager = nullptr;
	};

}

#endif // APPSIGNALCONTROLLER_H
