#pragma once
#include "../AppSignalLib/TuningValue.h"
#include "../AppSignalLib/AppSignal.h"
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../UtilsLib/Address16.h"

namespace RtTrends
{
	class Session;
}

class AppSignalState;
class AppSignals;
class AppDataSource;

struct ApertureRecord;

struct DynamicAppSignalState
{
public:
	static const int NO_INDEX = -1;
	static const int NO_AUTOARCHIVING_GROUP = -1;
	static const int NOT_INITIALIZED_AUTOARCHIVING_GROUP = -2;

private:
	enum class AnalogValueStatus
	{
		Normal,
		Nan,
		Inf
	};

	inline AnalogValueStatus analogValueStatus(double value)
	{
		if (std::isnan(value))
		{
			return AnalogValueStatus::Nan;
		}

		if (std::isinf(value))
		{
			return AnalogValueStatus::Inf;
		}

		return AnalogValueStatus::Normal;
	}

public:
	DynamicAppSignalState();

	void setSignalParams(const AppSignal& signal, const AppSignals& appSignals);

	void setQueues(SimpleAppSignalStatesArchiveFlagQueue* signalStatesQueue,
				   GatewayAppSignalStatesQueue* gatewaySignalStatesQueue,
				   std::vector<SimpleAppSignalState>* logQueue);

	int setStateRaw(AppDataSource& source,
					const Times& time,
					bool isSimPacket,
					quint16 packetNo,
					const char* rupData,
					int rupDataSize,
					int autoArchivingGroup);

	int setStateParsed(const Times& time,
					   quint16 packetNo,
					   double value,
					   AppSignalStateFlags flags,
					   int autoArchivingGroup);

	int setUnavailable(const Times& time,
					   SimpleAppSignalStatesArchiveFlagQueue& statesQueue);

	Hash hash() const;

	bool archive() const { return m_archive; }
	E::SoftwareCalcFunction swCalcFunction() const { return m_swCalcFunction; }

	QString appSignalID() const;

	friend class DynamicAppSignalStates;

	const SimpleAppSignalState& current() const { return m_current[m_curStateIndex.load()]; }

	int autoArchivingGroup() const { return m_autoArchivingGroup; }
	void setAutoArchivingGroup(int archivingGroup);

	void setGatewayQueueMask(quint32 mask);
	void resetGatewayQueueMask(quint32 mask);

	// Real time trends support
	//
	void appendRtSession(Hash signalHash,
						const QThread* rtProcessingOwner,
						std::shared_ptr<RtTrends::Session> newSession,
						int samplePeriodCounter);

	void removeRtSession(Hash signalHash,
						const QThread* rtProcessingOwner,
						std::shared_ptr<RtTrends::Session> sessionToRemove);

	void setRtSessionSamplePeriodCounter(Hash signalHash,
						const QThread* rtProcessingOwner,
						int sessionID,
						int newSamplePeriodCounter);

	void rtSessionsProcessing(const SimpleAppSignalState& state, bool pushAnyway);

//	const AppSignal* signal() const { return m_signal; }

	E::ApertureType apertureType() const { return m_apertureType; }
	E::SignalType signalType() const { return m_signalType; }

	double fineAperture() const { return m_fineAperture; }
	double coarseAperture() const { return m_coarseAperture; }

	double absFineAperture() const { return m_absFineAperture; }
	double absCoarseAperture() const { return m_absCoarseAperture; }

	double lowEngineeringUnits() const { return m_lowLimit; }
	double highEngineeringUnits() const { return m_highLimit; }

	void overrideAperture(const ApertureRecord& ar);
	bool apertureOverrided() const { return m_apertureOverrided; }

	int onArchSignalsTimer();
	inline void clearStatesSavedCounter() { m_statesSaved = 0; }

private:
	bool getValue(const char* rupData, int rupDataSize, double& value);
	bool getBit(const char* rupData, int rupDataSize, const Address16& addr, quint32& bit);

	void setNewCurState(const SimpleAppSignalState& newCurState);
	void logState(const SimpleAppSignalState& state);

	inline bool hasGatewaySendReasone(AppSignalStateFlags flags) const;

	// Real time trends support
	//
	void takeRtProcessingOwnership(const QThread* newProcessingOwner);
	void releaseRtProcessingOwnership(const QThread* currentProcessingOwner);

	void sendAppSignalStateChangeToGateway(const SimpleAppSignalState& prevState,
										   const SimpleAppSignalState& newState);

	void setAperture(E::ApertureType type, double coarseAperture, double fineAperture);

private:
	SimpleAppSignalStatesArchiveFlagQueue* m_statesQueue = nullptr;
	GatewayAppSignalStatesQueue* m_gwStatesQueue = nullptr;
	std::vector<SimpleAppSignalState>* m_logQueue = nullptr;

	struct FlagSignalParceInfo
	{
#ifdef QT_DEBUG
		QString flagSignalID;				// required for debugging only
#endif

		E::AppSignalStateFlagType flagType = E::AppSignalStateFlagType::Validity;
		Address16 flagSignalAddr;
	};

	struct RtSession
	{
		std::shared_ptr<RtTrends::Session> session;
		int sessionID = 0;
		int samplePeriodCounter = 0;
		int sampleCounter = 0;
	};

private:
	QString m_appSignalID;
	Hash m_signalHash;

	// parsing parameters

	Address16 m_valueAddr;
//	Address16 m_validityAddr;

	E::SignalType m_signalType = E::SignalType::Discrete;
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;
	E::SoftwareCalcFunction m_swCalcFunction = E::SoftwareCalcFunction::None;
	int m_dataSize = 1;

	bool m_archive = false;
	bool m_log = false;

	double m_lowLimit = 0;
	double m_highLimit = 0;
	bool m_reverseLimits = false;
	bool m_overrideAboveHighLimitFlag = false;		// state of flag AboveHighLimit overrided by signal (set_flags used)
	bool m_overrideBelowLowLimitFlag = false;		// state of flag BelowLowLimit overrided by signal (set_flags used)

	E::ApertureType m_defaultApertureType = E::ApertureType::RangePercent;
	double m_defaultCoarseAperture = 0;
	double m_defaultFineAperture = 0;

	E::ApertureType m_apertureType = E::ApertureType::RangePercent;
	double m_coarseAperture = 0;
	double m_fineAperture = 0;
	bool m_apertureOverrided = false;

	// for E::ApertureType::RangePercent and E::ApertureType::AbsValue
	// m_absCoarseAperture and m_absFineAperture stored in abs EngineeringUnits
	//
	// for E::ApertureType::ValuePercent m_absCoarseAperture and m_absFineAperture stored in Percents
	//
	double m_absCoarseAperture = 0;
	double m_absFineAperture = 0;

	std::atomic<int> m_statesSaved = 0;

	bool m_enableTuning = false;
	TuningValue m_tuningDefaultValue;

	std::vector<FlagSignalParceInfo> m_flagsSignalsParceInfo;		// except  Validity flag signal

	// paramters needed to update state
	//
	bool m_prevStateIsStored = false;

	double m_coarseStoredValue;
	double m_fineStoredValue;

	//

	SimpleAppSignalState m_current[2];
	std::atomic<int> m_curStateIndex = {0};

	int m_autoArchivingGroup = NOT_INITIALIZED_AUTOARCHIVING_GROUP;

	// Real time trends support

	bool m_hasRtSessions = false;		// this is not thread-safe but fast-checked flag
										// if m_hasRtQueues == true, then slow thread-safe checking will run

	std::atomic<const QThread*> m_rtProcessingOwner = { nullptr };

	std::map<int, RtSession> m_rtSessions;	// RtSession.ID => RtSession

	quint32 m_gatewayQueueMask = 0;
};

class DynamicAppSignalStates
{
public:
	~DynamicAppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return TO_INT(m_appSignalState.size()); }

	DynamicAppSignalState* operator [] (int index);
	const DynamicAppSignalState* operator [] (int index) const;

	const DynamicAppSignalState* getStateByHash(Hash signalHash) const;
	DynamicAppSignalState* getStateByHash(Hash signalHash);

	const DynamicAppSignalState* getStateByID(const QString& signalID) const;
	DynamicAppSignalState* getStateByID(const QString& signalID);

	void buidlHash2State();

	bool getCurrentState(Hash hash, AppSignalState& state) const;

	void setAutoArchivingGroups(int autoArchivingGroupsCount);

	void setGatewayQueueMask(const std::set<Hash>& hashes, quint32 mask);
	void resetGatewayQueueMask(const std::set<Hash>& hashes, quint32 mask);

	void overrideAperture(const ApertureRecord& ar, QString& logMsg);
	void clearStatesSavedCounters();

private:
	std::vector<DynamicAppSignalState*> m_appSignalState;
	std::map<Hash, DynamicAppSignalState*> m_hash2State;		// calcHash(AppSignalID) => DynamicAppSignalState*
};
