#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "../lib/AppSignalManager.h"
#include "../TrendView/TrendSignal.h"
#include "SimRam.h"
#include "SimScopedLog.h"


namespace Sim
{
	class Simulator;


	struct FlagsReadStruct
	{
		size_t flagCount = 0;
		std::array<std::pair<E::AppSignalStateFlagType, Address16>, sizeof AppSignalStateFlags::all * 8> flagsSignalAddresses;

		size_t flagConstsCount = 0;
		std::array<std::pair<E::AppSignalStateFlagType, quint32>, sizeof AppSignalStateFlags::all * 8> flagsConsts;

		bool create(const AppSignal& s, const std::unordered_map<Hash, AppSignal>& signalParams, ScopedLog& log);

		AppSignalStateFlags signalFlags(const Ram& ram, ScopedLog& log) const;
	};


	//
	//
	//	AppSignalManager
	//
	//
	class AppSignalManager : public QObject, public IAppSignalManager
	{
		Q_OBJECT

	public:
		explicit AppSignalManager(Simulator* simulator, QObject* parent = nullptr);
		virtual ~AppSignalManager();

	public:
		QString ramDump(QString logicModuleId) const;

		void resetAll();
		void resetSignalParam();
		void resetRam();

		bool load(QString fileName);

		void setData(const QString& equipmentId, const Sim::Ram& ram, TimeStamp plantTime, TimeStamp localTime, TimeStamp systemTime);
		std::shared_ptr<TrendLib::RealtimeData> trendData(const QString& trendId,
														  const std::vector<Hash>& trendSignals,
														  TrendLib::TrendStateItem* minState,
														  TrendLib::TrendStateItem* maxState);

		std::optional<AppSignal> signalParamExt(const QString& appSignalId) const;
		std::optional<AppSignal> signalParamExt(Hash hash) const;

		Hash customToAppSignal(Hash customSignalHash) const;

		AppSignalState signalState(const QString& appSignalId, bool* found, bool applyOverride) const;
		AppSignalState signalState(Hash signalHash, bool* found, bool applyOverride) const;			// <<<< GETTING STATE CODE HERE

		bool getUpdateForRam(const QString& equipmentId, Sim::Ram* ram) const;

	public:
		// Implementing IAppSignalManager - AppSignals
		//
		virtual std::vector<AppSignalParam> signalList() const override;

		virtual bool signalExists(Hash hash) const override;
		virtual bool signalExists(const QString& appSignalId) const override;

		virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
		virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

		virtual AppSignalState signalState(Hash signalHash, bool* found) const override;
		virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override;

		virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const override;
		virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const override;

		virtual QStringList signalTags(Hash signalHash) const override;
		virtual QStringList signalTags(const QString& appSignalId) const override;

		virtual bool signalHasTag(Hash signalHash, const QString& tag) const override;
		virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const override;

		virtual QString equipmentToAppSiganlId(const QString& equipmentId) const final;

		// Implementing IAppSignalManager - Setpoints/Comparators
		//
		virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const override;

		// Data Acess
		//
	public:
		const Simulator* simulator() const;
		Simulator* simulator();

		// Data
		//
	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		mutable QReadWriteLock m_signalParamLock{QReadWriteLock::Recursive};
		std::unordered_map<Hash, AppSignalParam> m_signalParams;
		std::unordered_map<Hash, AppSignal> m_signalParamsExt;			// Except AppSignalParam, we need Signal as it has more information (like offset in memory)
		std::unordered_map<Hash, Hash> m_customToAppSignalId;

		// SimRuntime data
		//
		mutable QReadWriteLock m_ramLock{QReadWriteLock::Recursive};
		std::map<Hash, Ram> m_ram;			// key is hash EquipmentID
		std::map<Hash, Times> m_ramTimes;	// RAM memory update time - key is hash EquipmentID
		std::unordered_map<Hash, FlagsReadStruct> m_flagsStruct;	// Signnal has a set of flags, which are signals itself, this structure contains addressed for such flag signals

		// Realtime trends data
		//
		struct TrendSignal
		{
			QString appSignalId;
			Hash appSignalHash;

			QString lmEquipmentId;
			Hash lmEquipmentIdHash;

			std::vector<AppSignalState> states;
		};

		struct Trend
		{
			QString trendId;
			QDateTime lastAccess;		// If data was not fetched for some time this trend will be removed
			std::vector<TrendSignal> trendSignals;
		};

		mutable QMutex m_trendMutex;
		std::list<Trend> m_trends;
	};


	/*! \class ScriptAppSignalManager
		\ingroup controllers
		\brief This class is used to get signal parameters and states in simulation

		This class is used to get signal parameters and states. It is accessed by global <b>signals</b> object.

		\warning
		It is highly recommended to check function return values, because errors can occur.

		\n
		<b>Example:</b>

		\code
		// Get static parameters of the signal "#SIGNALID_001"
		//
		var param = signals.signalParam("#SIGNALID_001");

		// Get state of the signal "#SIGNALID_001"
		//
		var state = signals.signalState("#SIGNALID_001");

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
			var text = param.Caption;
			...
		}
		\endcode
	*/
	class ScriptAppSignalManager : public QObject
	{
		Q_OBJECT

	public:
		explicit ScriptAppSignalManager(const IAppSignalManager* appSignalManager, QObject* parent = nullptr);

		// Script Interface
		//
	public slots:
		/// \brief Returns AppSignalParam structure of signal specified by <b>signalId</b>. If error occurs, the return value is <b>undefined</b>.
		QJSValue signalParam(QString signalId) const;		// Returns AppSignalParam
		QJSValue signalParam(Hash signalHash) const;		// Returns AppSignalParam

		/// \brief Returns AppSignalState structure of signal specified by <b>signalId</b>. If error occurs, the return value is <b>undefined</b>.
		QJSValue signalState(QString signalId) const;		// Returns AppSignalState
		QJSValue signalState(Hash signalHash) const;		// Returns AppSignalState

		// Data
		//
	private:
		const IAppSignalManager* m_appSignalManager = nullptr;
	};

}


