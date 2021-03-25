#pragma once

#include "VFrame30Lib_global.h"
#include "../lib/IAppSignalManager.h"

class AppSignalParam;
class AppSignalState;
class Comparator;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT AppSignalController : public QObject
	{
		Q_OBJECT

	public:
		AppSignalController() = delete;
		explicit AppSignalController(IAppSignalManager* appSignalManager, QObject* parent = nullptr);

	public:
		// App Signals
		//
		[[nodiscard]] bool signalExists(Hash hash) const;
		[[nodiscard]] bool signalExists(const QString& appSignalId) const;

		[[nodiscard]] AppSignalParam signalParam(Hash signalHash, bool* found) const;
		[[nodiscard]] AppSignalParam signalParam(const QString& appSignalId, bool* found) const;

		[[nodiscard]] AppSignalState signalState(Hash signalHash, bool* found) const;
		[[nodiscard]] AppSignalState signalState(const QString& appSignalId, bool* found) const;

		void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const;
		void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const;

		[[nodiscard]] QStringList signalTags(Hash signalHash) const;
		[[nodiscard]] QStringList signalTags(const QString& appSignalId) const;
		[[nodiscard]] bool signalHasTag(Hash signalHash, const QString& tag) const;
		[[nodiscard]] bool signalHasTag(const QString& appSignalId, const QString& tag) const;

		// Setpoints AKA Comparators
		//
		[[nodiscard]] std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const;

	public:
		[[nodiscard]] IAppSignalManager* appSignalManager();
		[[nodiscard]] const IAppSignalManager* appSignalManager() const;

	private:
		IAppSignalManager* m_appSignalManager = nullptr;
	};

	// This class used for scripts, it is created to separate AppSignalController from ugly
	// script decorations like Q_INVOKABLE and from mixing QVariant with actual return types AppSignalParam/State
	//

	/*! \class ScriptAppSignalController
		\ingroup controllers
		\brief This class is used to get signal parameters and states in Monitor application

		This class is used to get signal parameters and states in Monitor application. It is accessed by global <b>signals</b> object.

		Information about signal parameters and states is requested from ApplicationDataService.

		\warning
		It is highly recommended to check function return values, because errors can occur. For example,
		connection to ApplicationDataService can be down, or signal with specified identifier could not exist.

		\n
		<b>Example:</b>

		\code
		// Get static parameters of the signal "#SIGNALID_001"
		//
		let param = signals.signalParam("#SIGNALID_001");

		// Get state of the signal "#SIGNALID_001"
		//
		let state = signals.signalState("#SIGNALID_001");

		// Check for functions result
		//
		if  (param === undefined)
		{
			// Signal static parameters request failed
			//
			...
			return;
		}
		if  (state === undefined)
		{
			// Signal state request failed
			//
			...
			return;
		}

		// Further processing
		//

		if (state.Valid === true)
		{
			let text = param.Caption;
			...
		}
		\endcode
	*/
	class ScriptAppSignalController : public QObject
	{
		Q_OBJECT

	public:
		explicit ScriptAppSignalController(const IAppSignalManager* appSignalManager, QObject* parent = nullptr);
		virtual ~ScriptAppSignalController();

		// Script Interface
		//
	public slots:
		/// \brief Returns AppSignalParam structure of signal specified by <b>signalId</b>. If error occurs, the return value is <b>undefined</b>.
		QJSValue signalParam(QString signalId) const;		// Returns AppSignalParam
		QJSValue signalParam(Hash signalHash) const;		// Returns AppSignalParam

		/// \brief Returns AppSignalState structure of signal specified by <b>signalId</b>. If error occurs, the return value is <b>undefined</b>.
		QJSValue signalState(QString signalId) const;		// Returns AppSignalState
		QJSValue signalState(Hash signalHash) const;		// Returns AppSignalState

		/// \brief Returns true if signal specified by <b>signalId</b> is exist.
		bool signalExists(QString signalId) const;

		// Data
		//
	private:
		const IAppSignalManager* m_appSignalManager = nullptr;
	};

}

