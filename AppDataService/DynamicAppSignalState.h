#pragma once

#include "../lib/Signal.h"
#include "../lib/SimpleAppSignalState.h"

namespace RtTrends
{
	class Session;
}

class AppSignalState;

struct DynamicAppSignalState
{
public:
	static const int NO_INDEX = -1;
	static const int NO_AUTOARCHIVING_GROUP = -1;
	static const int NOT_INITIALIZED_AUTOARCHIVING_GROUP = -2;

public:
	DynamicAppSignalState();

	void setSignalParams(int index, Signal* signal);

	bool setState(const Times& time,
				  quint16 packetNo,
				  quint32 validity,
				  double value,
				  int autoArchivingGroup,
				  SimpleAppSignalStatesQueue& statesQueue,
				  const QThread* thread);

	void setUnavailable(const Times& time,
				  SimpleAppSignalStatesQueue& statesQueue,
				  const QThread* thread);

//	void invalidate() { m_current[0].flags.all = m_current[1].flags.all = 0; }

	Hash hash() const;

	bool archive() const { return m_archive; }

	QString appSignalID() const;

	friend class DynamicAppSignalStates;

	const SimpleAppSignalState& current() const { return m_current[m_curStateIndex.load()]; }

	int autoArchiningGroup() const { return m_autoArchivingGroup; }
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

	const Signal* signal() const { return m_signal; }

private:

	struct RtSession
	{
		std::shared_ptr<RtTrends::Session> session;
		int sessionID = 0;
		int samplePeriodCounter = 0;
		int sampleCounter = 0;
	};

private:
	void setNewCurState(const SimpleAppSignalState& newCurState);
	void logState(const SimpleAppSignalState& state);

	// Real time trends support
	//
	void takeRtProcessingOwnership(const QThread* newProcessingOwner);
	void releaseRtProcessingOwnership(const QThread* currentProcessingOwner);

private:
	SimpleAppSignalState m_current[2];
	double m_coarseStoredValue;
	double m_fineStoredValue;

	std::atomic<int> m_curStateIndex = {0};

	// paramters needed to update state
	//
	bool m_prevStateIsStored = false;
	bool m_isDiscreteSignal = false;

	bool m_archive = false;

	bool m_adaptiveAperture = false;

	double m_coarseAperture = 0;
	double m_fineAperture = 0;

	double m_lowLimit = 0;
	double m_highLimit = 0;

	//

	double m_absCoarseAperture = 0;
	double m_absFineAperture = 0;

	int m_autoArchivingGroup = NOT_INITIALIZED_AUTOARCHIVING_GROUP;

	Signal* m_signal = nullptr;
	Hash m_signalHash;

	int m_index = 0;

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
