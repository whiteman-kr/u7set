#pragma once

#include<queue>

#include "../OnlineLib/DataSource.h"
#include "../OnlineLib/CircularLogger.h"
#include "DynamicAppSignalState.h"
#include "DiscretesLog.h"

class AppDataReceiver;

class AppDataSource : public OnlineLib::DataSourceOnline
{
public:
	AppDataSource(const OnlineLib::DataSource& dataSource, CircularLoggerShared logger);
	AppDataSource(const Network::DataSourceInfo& proto);
	virtual ~AppDataSource();

	void prepare(const AppSignals& appSignals,
				 DynamicAppSignalStates* signalStates,
				 std::shared_ptr<DiscretesLogWriter> discretesLog,
				 int autoArchivingGroupsCount,
				 CircularLoggerShared timeErrLog);

	void setStatesProcessingThreadWakeupParams(std::mutex* statesProcessigRequiredMutex,
											  std::condition_variable* statesProcessingRequiredCondition,
											  std::queue<AppDataSource*>* statesProcessingRequired);

	bool getState(Network::AppDataSourceState* proto) const;
	void setState(const Network::AppDataSourceState& proto);

	bool getSignalState(SimpleAppSignalStateArchiveFlag* state);
	bool getGatewaySignalState(GatewayAppSignalStateQueueMask* gwState);

	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }

	int signalStatesQueueCurSize() const { return m_signalStatesQueueCurSize; }
	int signalStatesQueueCurMaxSize() const { return m_signalStatesQueueCurMaxSize; }

	void invalidateSignals();

	quint32 cachedAppDataUID() const { return m_cachedAppDataUID; }

	bool statesQueueIsEmpty() const;

	void incBlockFlagsCount() { m_blockFlagsCount++; }
	void incSimFlagsCount() { m_simFlagsCount++; }
	void incMismatchFlagsCount() { m_mismatchFlagsCount++; }

	// for testing purposes only!
	//
	void pushState(const SimpleAppSignalState& state);
	void resizeSignalStatesQueue(int size);

	void checkInputPlantTime(Rup::TimeStamp plantTime);

private:
	virtual bool parseBuffer(ParsingBuffer& readBuffer) override;

	void wakeupStatesProcessingThread();

	int getAutoArchivingGroup(qint64 currentSysTime);

	void setAcquiredSignalsCount(int count) { m_acquiredSignalsCount = count; }

	void setSignalStatesQueueSize(int size) { m_signalStatesQueueSize = size; }
	void setSignalStatesQueueCurSize(int size) { m_signalStatesQueueCurSize = size; }
	void setSignalStatesQueueCurMaxSize(int size) { m_signalStatesQueueCurMaxSize = size; }

	virtual quint32 getExpectedDataUID() const override { return m_cachedAppDataUID; }

private:
	static constexpr int SIGNAL_STATES_QUEUE_MIN_SIZE = 1000;

	CircularLoggerShared m_log;

	std::mutex* m_statesProcessigRequiredMutex = nullptr;
	std::condition_variable* m_statesProcessingRequiredCondition = nullptr;
	std::queue<AppDataSource*>* m_statesProcessingRequired = nullptr;

	std::shared_ptr<DiscretesLogWriter> m_discretesLog;

	// states of source aquired signals, excluding software calculated signals
	//
	std::vector<DynamicAppSignalState*> m_signalStates;

	// states of software calculated siganals
	//
	std::map<E::SoftwareCalcFunction, std::vector<DynamicAppSignalState*>> m_swCalcSignalsStates;

	int m_blockFlagsCount = 0;
	int m_simFlagsCount = 0;
	int m_mismatchFlagsCount = 0;

	int m_acquiredSignalsCount = 0;

	SimpleAppSignalStatesArchiveFlagQueue m_signalStatesQueue;
	GatewayAppSignalStatesQueue m_gatewaySignalStatesQueue;
	std::vector<SimpleAppSignalState> m_logStatesQueue;

	int m_signalStatesQueueSize = 0;
	int m_signalStatesQueueCurSize = 0;
	int m_signalStatesQueueCurMaxSize = 0;

	int m_gatewaySignalStatesQueueCurSize = 0;

	// app data parsing
	//
	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;

	static const int TIME_1S = 1000;

	int m_autoArchivingGroupsCount = 0;
	qint64 m_lastAutoArchivingTime = 0;
	int m_lastAutoArchivingGroup = DynamicAppSignalState::NOT_INITIALIZED_AUTOARCHIVING_GROUP;

	quint32 m_cachedAppDataUID = 0;

	//

	Rup::TimeStamp m_lastPlantTime;
};

class AppDataSources
{
public:
	AppDataSources();
	~AppDataSources();

	bool init(const QString& profile,
			  const QVector<OnlineLib::DataSource>& dataSources,
			  CircularLoggerShared logger);
	void clear();

	AppDataSource* getSourceByIP(quint32 ip);
	AppDataSource* getSourceByEquipmentID(const QString& equipmentId);

	AppDataSource* getSignalSource(const QString& signalID);
	AppDataSource* getSignalSource(Hash signalHash);
	const AppDataSource* getSignalSource(Hash signalHash) const;

	std::vector<AppDataSource*>::iterator begin();
	std::vector<AppDataSource*>::const_iterator begin() const;

	std::vector<AppDataSource*>::iterator end();
	std::vector<AppDataSource*>::const_iterator end() const;

private:
	const AppDataSource* privateGetSignalSource(Hash signalHash) const;

private:
	// dynamic AppDataSource objects owner!
	//
	std::vector<AppDataSource*> m_sources;

	// module EquipmentID => AppDataSource*
	//
	std::map<QString, AppDataSource*> m_moduleToSource;

	// lan controller EquipmentID => AppDataSource*
	//
	std::map<QString, AppDataSource*> m_lanControllerToSource;

	// module ethernet adapter IP => AppDataSource*
	//
	std::map<quint32, AppDataSource*> m_ipToSource;

	// signal Hash => AppDataSource*
	//
	std::map<Hash, AppDataSource*> m_signalToSource;
};
