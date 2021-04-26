#pragma once

#include "../lib/AppSignal.h"
#include "../OnlineLib/SimpleAppSignalState.h"

namespace RtTrends
{
	class Session;
}

class AppSignalState;
class AppSignals;

struct DynamicAppSignalState
{
public:
	static const int NO_INDEX = -1;
	static const int NO_AUTOARCHIVING_GROUP = -1;
	static const int NOT_INITIALIZED_AUTOARCHIVING_GROUP = -2;

public:
	DynamicAppSignalState();

	void setSignalParams(const AppSignal* signal, const AppSignals& appSignals);

	bool setState(const Times& time,
				  bool isSimPacket,
				  quint16 packetNo,
				  const char* rupData,
				  int rupDataSize,
				  int autoArchivingGroup,
				  SimpleAppSignalStatesArchiveFlagQueue& statesQueue,
				  const QThread* thread);

	void setUnavailable(const Times& time,
				  SimpleAppSignalStatesArchiveFlagQueue& statesQueue,
				  const QThread* thread);

	Hash hash() const;

	bool archive() const { return m_archive; }

	QString appSignalID() const;

	friend class DynamicAppSignalStates;

	const SimpleAppSignalState& current() const { return m_current[m_curStateIndex.load()]; }

	int autoArchivingGroup() const { return m_autoArchivingGroup; }
	void setAutoArchivingGroup(int archivingGroup);

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

	void rtSessionsProcessing(const SimpleAppSignalState& state, bool pushAnyway, const QThread* thread);

	const AppSignal* signal() const { return m_signal; }

private:
	bool getValue(const char* rupData, int rupDataSize, double& value);
	bool getBit(const char* rupData, int rupDataSize, const Address16& addr, quint32& bit);

	void setNewCurState(const SimpleAppSignalState& newCurState);
	void logState(const SimpleAppSignalState& state);

	// Real time trends support
	//
	void takeRtProcessingOwnership(const QThread* newProcessingOwner);
	void releaseRtProcessingOwnership(const QThread* currentProcessingOwner);

private:
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
	const AppSignal* m_signal = nullptr;
	Hash m_signalHash;

	// parsing parameters

	Address16 m_valueAddr;
	Address16 m_validityAddr;

	E::SignalType m_signalType = E::SignalType::Discrete;
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	double m_absCoarseAperture = 0;
	double m_absFineAperture = 0;

	int m_dataSize = 1;

	QVector<FlagSignalParceInfo> m_flagsSignalsParceInfo;		// except  Validity flag signal


	void init(const AppSignal& s, const AppSignals& appSignals);

	// paramters needed to update state
	//
	bool m_prevStateIsStored = false;

	bool m_archive = false;

	bool m_adaptiveAperture = false;

	double m_coarseAperture = 0;
	double m_fineAperture = 0;

	double m_lowLimit = 0;
	double m_highLimit = 0;

	double m_coarseStoredValue;
	double m_fineStoredValue;

	//

	//quint32 m_prevValidity = false;
	//double m_prevValue = 0;

	//

	SimpleAppSignalState m_current[2];
	std::atomic<int> m_curStateIndex = {0};


	int m_autoArchivingGroup = NOT_INITIALIZED_AUTOARCHIVING_GROUP;

	// Real time trends support

	bool m_hasRtSessions = false;		// this is not thread-safe but fast-checked flag
										// if m_hasRtQueues == true, then slow thread-safe checking will run

	std::atomic<const QThread*> m_rtProcessingOwner = { nullptr };

	QHash<int, RtSession> m_rtSessions;
};


class DynamicAppSignalStates
{
public:
	~DynamicAppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	DynamicAppSignalState* operator [] (int index);

	DynamicAppSignalState* getStateByHash(Hash signalHash);
	DynamicAppSignalState* getStateByID(const QString& signalID) { return getStateByHash(calcHash(signalID)); }

	void buidlHash2State();

	bool getCurrentState(Hash hash, AppSignalState& state) const;
//	bool getStoredState(Hash hash, AppSignalState& state) const;

	void setAutoArchivingGroups(int autoArchivingGroupsCount);

private:
	QMutex m_allMutex;

	DynamicAppSignalState* m_appSignalState = nullptr;
	int m_size = 0;

	QHash<Hash, DynamicAppSignalState*> m_hash2State;
};
