#pragma once

#include "../AppSignalLib/IAppSignalManager.h"

#include <QJSValue>
#include <QObject>

#include <ranges>

class AppSignalParam;
class AppSignalState;
class Comparator;

namespace VFrame30
{
	class AppSignalController : public QObject
	{
		Q_OBJECT

	public:
		AppSignalController() = delete;
		explicit AppSignalController(IAppSignalManager& appSignalManager, QObject* parent = nullptr);

	public:
		// App Signals
		//
		[[nodiscard]] int signalsCount() const;
		[[nodiscard]] bool signalExists(Hash hash) const;
		[[nodiscard]] bool signalExists(const QString& appSignalId) const;

		[[nodiscard]] std::optional<AppSignalParam> signalParam(Hash signalHash) const;
		[[nodiscard]] std::optional<AppSignalParam> signalParam(const QString& appSignalId) const;

		[[nodiscard]] std::optional<AppSignalState> signalState(Hash signalHash) const;
		[[nodiscard]] std::optional<AppSignalState> signalState(const QString& appSignalId) const;

		void signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const;

		template<StringRange Range>
		void signalState(const Range& appSignalIds, std::vector<std::optional<AppSignalState>>* result) const
		{
			std::vector<Hash> hashes;
			hashes.reserve(appSignalIds.size());
			for (const QString& id : appSignalIds)
			{
				hashes.push_back(::calcHash(id));
			}
			return signalState(hashes, result);
		}

		[[nodiscard]] QStringList signalTags(Hash signalHash) const;
		[[nodiscard]] QStringList signalTags(const QString& appSignalId) const;
		[[nodiscard]] bool signalHasTag(Hash signalHash, const QString& tag) const;
		[[nodiscard]] bool signalHasTag(const QString& appSignalId, const QString& tag) const;

		// Setpoints AKA Comparators
		//
		[[nodiscard]] std::vector<std::shared_ptr<Comparator>> setpointsByInput(const QString& appSignalId) const;
		[[nodiscard]] std::shared_ptr<Comparator> setpointsByOutput(const QString& appSignalId) const;

	public:
		[[nodiscard]] IAppSignalManager& appSignalManager();
		[[nodiscard]] const IAppSignalManager& appSignalManager() const;

	private:
		IAppSignalManager& m_appSignalManager;
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

		\code{.js}
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
		if (state.valid === true)
		{
			let text = param.caption;
			...
		}
		\endcode

		\code{.js}
		// Getting signal list with specified tag
		//
		let ids = signals.signalIdsByTag("actuator");

		for (let i = 0; i < ids.length; i++)
		{
			// ids[i] contains AppSignalID of signal with tag "actuator"
			//
		}
		\endcode
	*/
	class ScriptAppSignalController : public QObject
	{
		Q_OBJECT

	public:
		explicit ScriptAppSignalController(const IAppSignalManager& appSignalManager, QObject* parent = nullptr);
		virtual ~ScriptAppSignalController();

		// Script Interface
		//
	public slots:
		/// \brief Returns total signals count in the database.
		int signalsCount() const;

		/// \brief Returns AppSignalParam structure of signal specified by <b>signalId</b>. If error occurs, the return value is
		/// <b>undefined</b>.
		QJSValue signalParam(QString signalId) const; // Returns AppSignalParam
		QJSValue signalParam(Hash signalHash) const;  // Returns AppSignalParam

		/// \brief Returns AppSignalID for specified EquipmentID, note: EquipmentID must start from symbol @, and such conversion is
		/// possible for input/output signals and impossible for internal LogicModule signals (returns an empty string).
		QString equipmentToAppSignalId(QString equipmentId) const;

		// This version was before the typo was fixed, left for script compatibility.
		// DO NOT MODIFY OR REMOVE IT.
		QString equipmentToAppSiganlId(QString equipmentId) const;

		/// \brief Returns AppSignalState structure of signal specified by <b>signalId</b>. If error occurs, the return value is
		/// <b>undefined</b>.
		QJSValue signalState(QString signalId) const; // Returns AppSignalState
		QJSValue signalState(Hash signalHash) const;  // Returns AppSignalState

		/// \brief Returns an array of AppSignalState structures of signals specified by <b>signalIds</b>. If signal is not found, then the
		/// <b>stateAvailable</b> is false.
		QJSValueList signalStates(QStringList signalIds) const; // Returns AppSignalState

		/// \brief Returns <b>true</b> if signal specified by <b>signalId</b> is exist.
		bool signalExists(QString signalId) const;

		/// \brief The function takes a list of signal IDs and returns true or false depending on whether all signals exist or not.
		bool signalsExist(QStringList signalIds);

		/// \brief Returns <b>true</b> for discrete signals.
		bool isDiscrete(QString signalId) const;

		/// \brief Returns <b>true</b> for analog signals.
		bool isAnalog(QString signalId) const;

		/// \brief Returns precision for analog signals. If signal is discrete or not found, returns 0.
		int precision(QString signalId) const;

		/// \brief Returns list of AppSignalIDs with specified <b>tag</b>.
		QStringList signalIdsByTag(QString tag) const;

		/// \brief Returns list of Comparator (setpoint comparators) assigned to the signal specified by <b>signalId</b>.
		QJSValueList setpointsByInput(QString signalId) const;

		/// \brief Returns Comparator (setpoint comparator) assigned where <b>signalId</b> is an output signal.
		QJSValue setpointByOutput(QString signalId) const;

		// Data
		//
	private:
		const IAppSignalManager& m_appSignalManager;
	};

} // namespace VFrame30
