#pragma once

#include "../lib/Signal.h"
#include "../lib/DataSource.h"


class SimpleAppSignalStatesQueue : public LockFreeQueue<SimpleAppSignalState>
{
public:
	SimpleAppSignalStatesQueue(int queueSize);

	bool pushAutoPoint(SimpleAppSignalState state);
};

typedef std::shared_ptr<SimpleAppSignalStatesQueue> SimpleAppSignalStatesQueueShared;


namespace RtTrends
{
	class Session;
}

struct AppSignalStateEx
{
public:
	static const int NO_INDEX = -1;
	static const int NO_AUTOARCHIVING_GROUP = -1;
	static const int NOT_INITIALIZED_AUTOARCHIVING_GROUP = -2;

public:
	AppSignalStateEx();

	void setSignalParams(int index, Signal* signal);

	bool setState(const Times& time, quint32 validity, double value, int autoArchivingGroup, SimpleAppSignalStatesQueue& statesQueue);
	void invalidate() { m_current[0].flags.all = m_current[1].flags.all = m_stored.flags.all = 0; }

	Hash hash() const;

	bool archive() const { return m_archive; }

	QString appSignalID() const;

	friend class AppSignalStates;

	const SimpleAppSignalState& current() const { return m_current[m_curStateIndex.load()]; }
	const SimpleAppSignalState& stored() const { return m_stored; }

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

	void rtSessionsProcessing(const SimpleAppSignalState& state, bool pushAnyway);

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

	int getSamplePeriodCounter(E::RtTrendsSamplePeriod period, int lmWorkcycle_ms);

private:
	SimpleAppSignalState m_current[2];
	SimpleAppSignalState m_stored;

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


class AppSignalStates
{
public:
	~AppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	AppSignalStateEx* operator [] (int index);

	AppSignalStateEx* getStateByHash(Hash signalHash);

	void buidlHash2State();

	bool getCurrentState(Hash hash, AppSignalState& state) const;
	bool getStoredState(Hash hash, AppSignalState& state) const;

	void setAutoArchivingGroups(int autoArchivingGroupsCount);

private:
	QMutex m_allMutex;

	AppSignalStateEx* m_appSignalState = nullptr;
	int m_size = 0;

	QHash<Hash, AppSignalStateEx*> m_hash2State;
};


class AppSignals : public HashedVector<QString, Signal*>
{
private:
	QHash<Hash, Signal*> m_hash2Signal;

public:
	~AppSignals();

	void clear();
	void buildHash2Signal();

	const Signal* getSignal(Hash hash) const;
};

class AppDataSource : public DataSourceOnline
{
private:
	struct SignalParseInfo
	{
	public:
		Address16 valueAddr;
		Address16 validityAddr;

		E::SignalType type = E::SignalType::Discrete;
		E::AnalogAppSignalFormat analogSignalFormat = E::AnalogAppSignalFormat::Float32;
		E::ByteOrder byteOrder = E::ByteOrder::BigEndian;

		int dataSize = 1;

		int index = AppSignalStateEx::NO_INDEX;		// index of signal in AppSignals and AppSignalStates

		QString appSignalID;

		void setSignalParams(int i, const Signal& s);
	};

public:
	AppDataSource();
	AppDataSource(const DataSource& dataSource);

	void prepare(const AppSignals& appSignals, AppSignalStates* signalStates, int autoArchivingGroupsCount);

	bool parsePacket();

	bool getState(Network::AppDataSourceState* proto) const;
	void setState(const Network::AppDataSourceState& proto);

	bool getSignalState(SimpleAppSignalState* state);

private:
	int getAutoArchivingGroup(qint64 currentSysTime);

	bool getDoubleValue(const char* rupData, int rupDataSize, const SignalParseInfo& parseInfo, double& value);
	bool getValidity(const char* rupData, int rupDataSize, const SignalParseInfo& parseInfo, quint32& validity);

	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }
	void setAcquiredSignalsCount(int count) { m_acquiredSignalsCount = count; }

	int signalStatesQueueSize() const { return m_signalStatesQueueSize; }
	void setSignalStatesQueueSize(int size) { m_signalStatesQueueSize = size; }

	int signalStatesQueueMaxSize() const { return m_signalStatesQueueMaxSize; }
	void setSignalStatesQueueMaxSize(int size) { m_signalStatesQueueMaxSize = size; }

private:
	AppSignalStates* m_signalStates = nullptr;

	AppSignals* m_appSignals = nullptr;

	QVector<SignalParseInfo> m_signalsParseInfo;

	int m_acquiredSignalsCount = 0;

	SimpleAppSignalStatesQueue m_signalStatesQueue;

	int m_signalStatesQueueSize = 0;
	int m_signalStatesQueueMaxSize = 0;

	// app data parsing
	//
	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;

	static const int TIME_1S = 1000;

	int m_autoArchivingGroupsCount = 0;
	qint64 m_lastAutoArchivingTime = 0;
	int m_lastAutoArchivingGroup = AppSignalStateEx::NOT_INITIALIZED_AUTOARCHIVING_GROUP;
};

typedef std::shared_ptr<AppDataSource> AppDataSourceShared;

typedef QHash<QString, AppDataSourceShared> AppDataSources;		// app data source EquipmentID => AppDataSourceShared

typedef QHash<quint32, AppDataSourceShared> AppDataSourcesIP;	// app data source IP => AppDataSourceShared

typedef QHash<Hash, AppDataSourceShared> SignalsToSources;		// signal Hash => AppDataSourceShared
