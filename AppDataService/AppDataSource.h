#pragma once

#include<queue>

#include "../lib/DataSource.h"
#include "../OnlineLib/CircularLogger.h"
#include "DynamicAppSignalState.h"

class AppDataReceiver;

class AppDataSource : public DataSourceOnline
{
public:
	AppDataSource(const DataSource& dataSource);
	AppDataSource(const Network::DataSourceInfo& proto);
	virtual ~AppDataSource();

	void prepare(const AppSignals& appSignals,
				 DynamicAppSignalStates* signalStates,
				 int autoArchivingGroupsCount,
				 CircularLoggerShared timeErrLog);

	void setStatesProcessingThreadWakeupParams(std::mutex* statesProcessigRequiredMutex,
											  std::condition_variable* statesProcessingRequiredCondition,
											  std::queue<AppDataSource*>* statesProcessingRequired);

	bool getState(Network::AppDataSourceState* proto) const;
	void setState(const Network::AppDataSourceState& proto);

	bool getSignalState(SimpleAppSignalStateArchiveFlag* state, const QThread* thread);
	bool getGatewaySignalState(GatewayAppSignalStateQueueMask* gwState, const QThread* thread);

	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }

	int signalStatesQueueCurSize() const { return m_signalStatesQueueCurSize; }
	int signalStatesQueueCurMaxSize() const { return m_signalStatesQueueCurMaxSize; }

	void invalidateSignals(const QThread* thread);

	quint32 cachedAppDataUID() const { return m_cachedAppDataUID; }

	bool statesQueueIsEmpty(QThread* thread) const;

private:
	virtual bool parseBuffer(ParsingBuffer& readBuffer, const QThread* thread) override;

	void wakeupStatesProcessingThread();

	int getAutoArchivingGroup(qint64 currentSysTime);

	void setAcquiredSignalsCount(int count) { m_acquiredSignalsCount = count; }

	void setSignalStatesQueueSize(int size) { m_signalStatesQueueSize = size; }
	void setSignalStatesQueueCurSize(int size) { m_signalStatesQueueCurSize = size; }
	void setSignalStatesQueueCurMaxSize(int size) { m_signalStatesQueueCurMaxSize = size; }

	virtual quint32 getExpectedDataUID() const override { return m_cachedAppDataUID; }

private:
	std::mutex* m_statesProcessigRequiredMutex = nullptr;
	std::condition_variable* m_statesProcessingRequiredCondition = nullptr;
	std::queue<AppDataSource*>* m_statesProcessingRequired = nullptr;

	//

	qint64 m_workcycle_ms = 0;
	QVector<DynamicAppSignalState*> m_signalStates;

	int m_acquiredSignalsCount = 0;

	SimpleAppSignalStatesArchiveFlagQueue m_signalStatesQueue;
	GatewayAppSignalStatesQueue m_gatewaySignalStatesQueue;

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
};

class AppDataSources
{
public:
	AppDataSources();
	~AppDataSources();

	bool init(const QString& profile,
			  const QVector<DataSource>& dataSources,
			  CircularLoggerShared logger);
	void clear();

	AppDataSource* getSourceByIP(quint32 ip);
	AppDataSource* getSignalSource(const QString& signalID);
	AppDataSource* getSignalSource(Hash signalHash);

	std::vector<AppDataSource*>::iterator begin();
	std::vector<AppDataSource*>::const_iterator begin() const;

	std::vector<AppDataSource*>::iterator end();
	std::vector<AppDataSource*>::const_iterator end() const;

//	const std::map<QString, AppDataSource*>& sources() const;

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
