#pragma once

#include "../CommonLib/Times.h"
#include "../UtilsLib/Address16.h"
#include <DiagStateLib/SimpleDiagSignalState.h>

namespace RtTrends
{
	class Session;
}

class AppSignalState;
class AppSignals;

struct DynamicDiagSignalState
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
	DynamicDiagSignalState();

//	void setSignalParams(const AppSignal* signal, const AppSignals& appSignals);

//	void setQueues(SimpleAppSignalStatesArchiveFlagQueue* signalStatesQueue,
//				   GatewayAppSignalStatesQueue* gatewaySignalStatesQueue);

	int setState(const Times& time,
				  bool isSimPacket,
				  quint16 packetNo,
				  const char* rupData,
				  int rupDataSize,
				  int autoArchivingGroup,
				  const QThread* thread);

/*	int setUnavailable(const Times& time,
				  SimpleAppSignalStatesArchiveFlagQueue& statesQueue,
				  const QThread* thread);*/

	Hash hash() const;

	bool archive() const { return m_archive; }

//	QString appSignalID() const;

	friend class DynamicAppSignalStates;

//	const SimpleAppSignalState& current() const { return m_current[m_curStateIndex.load()]; }

//	int autoArchivingGroup() const { return m_autoArchivingGroup; }
//	void setAutoArchivingGroup(int archivingGroup);

	void setGatewayQueueMask(quint32 mask);
	void resetGatewayQueueMask(quint32 mask);

	// Real time trends support
	//
//	void appendRtSession(Hash signalHash,
//						const QThread* rtProcessingOwner,
//						std::shared_ptr<RtTrends::Session> newSession,
//						int samplePeriodCounter);

//	void removeRtSession(Hash signalHash,
//						const QThread* rtProcessingOwner,
//						std::shared_ptr<RtTrends::Session> sessionToRemove);

//	void setRtSessionSamplePeriodCounter(Hash signalHash,
//						const QThread* rtProcessingOwner,
//						int sessionID,
//						int newSamplePeriodCounter);

//	void rtSessionsProcessing(const SimpleAppSignalState& state, bool pushAnyway, const QThread* thread);

//	const AppSignal* signal() const { return m_signal; }

private:
	bool getValue(const char* rupData, int rupDataSize, double& value);
	bool getBit(const char* rupData, int rupDataSize, const Address16& addr, quint32& bit);

//	void setNewCurState(const SimpleAppSignalState& newCurState);
//	void logState(const SimpleAppSignalState& state);

//	inline bool hasGatewaySendReasone(AppSignalStateFlags flags) const;

	// Real time trends support
	//
	void takeRtProcessingOwnership(const QThread* newProcessingOwner);
	void releaseRtProcessingOwnership(const QThread* currentProcessingOwner);

//	void sendAppSignalStateChangeToGateway(const SimpleAppSignalState& prevState,
//										   const SimpleAppSignalState& newState,
//										   const QThread* thread);

private:
//	SimpleAppSignalStatesArchiveFlagQueue* m_statesQueue = nullptr;
//	GatewayAppSignalStatesQueue* m_gwStatesQueue = nullptr;

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
//	const AppSignal* m_signal = nullptr;
	Hash m_signalHash;

	// parsing parameters

	Address16 m_valueAddr;
	Address16 m_validityAddr;

	E::DiagSignalType m_signalType = E::DiagSignalType::Discrete;
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;
	int m_dataSize = 1;

	bool m_archive = false;

	double m_lowLimit = 0;
	double m_highLimit = 0;
	bool m_reverseLimits = false;

	E::ApertureType m_apertureType = E::ApertureType::RangePercent;

	// for E::ApertureType::RangePercent and E::ApertureType::AbsValue
	// m_absCoarseAperture and m_absFineAperture stored in abs EngineeringUnits
	//
	// for E::ApertureType::ValuePercent m_absCoarseAperture and m_absFineAperture stored in Percents
	//
	double m_absCoarseAperture = 0;
	double m_absFineAperture = 0;

	std::vector<FlagSignalParceInfo> m_flagsSignalsParceInfo;		// except  Validity flag signal

	// paramters needed to update state
	//
	bool m_prevStateIsStored = false;

	double m_coarseStoredValue;
	double m_fineStoredValue;

	//

	SimpleDiagSignalState m_state[2];
	std::atomic<int> m_curStateIndex = {0};

	// Real time trends support

	bool m_hasRtSessions = false;		// this is not thread-safe but fast-checked flag
										// if m_hasRtQueues == true, then slow thread-safe checking will run

	std::atomic<const QThread*> m_rtProcessingOwner = { nullptr };

	std::map<int, RtSession> m_rtSessions;	// RtSession.ID => RtSession

	quint32 m_gatewayQueueMask = 0;
};

class DynamicDiagSignalStates
{
public:
	~DynamicDiagSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	DynamicDiagSignalState* operator [] (int index);

	const DynamicDiagSignalState* getStateByHash(Hash signalHash) const;
	DynamicDiagSignalState* getStateByHash(Hash signalHash);

	const DynamicDiagSignalState* getStateByID(const QString& signalID) const;
	DynamicDiagSignalState* getStateByID(const QString& signalID);

	void buidlHash2State();

	bool getCurrentState(Hash hash, AppSignalState& state) const;
//	bool getStoredState(Hash hash, AppSignalState& state) const;

	void setAutoArchivingGroups(int autoArchivingGroupsCount);

	void setGatewayQueueMask(const std::set<Hash>& hashes, quint32 mask);
	void resetGatewayQueueMask(const std::set<Hash>& hashes, quint32 mask);

private:
	DynamicDiagSignalState* m_diagSignalState = nullptr;
	int m_size = 0;

	QHash<Hash, DynamicDiagSignalState*> m_hash2State;
};
