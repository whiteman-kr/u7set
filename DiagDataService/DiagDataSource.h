#pragma once

#include<queue>

#include "../lib/DataSource.h"
#include "../OnlineLib/CircularLogger.h"
#include "DynamicDiagSignalState.h"

class DiagDataReceiver;

class DiagDataSource : public DataSourceOnline
{
public:
	DiagDataSource(const DataSource& dataSource);
	DiagDataSource(const Network::DataSourceInfo& proto);
	virtual ~DiagDataSource();

	void prepare(const AppSignals& appSignals,
				 DynamicDiagSignalStates* signalStates,
				 int autoArchivingGroupsCount,
				 CircularLoggerShared timeErrLog);

	void setStatesProcessingThreadWakeupParams(std::mutex* statesProcessigRequiredMutex,
											  std::condition_variable* statesProcessingRequiredCondition,
											  std::queue<DiagDataSource*>* statesProcessingRequired);

	bool getState(Network::DiagDataSourceState* proto) const;
	void setState(const Network::DiagDataSourceState& proto);

//	bool getSignalState(SimpleAppSignalStateArchiveFlag* state, const QThread* thread);
//	bool getGatewaySignalState(GatewayAppSignalStateQueueMask* gwState, const QThread* thread);

	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }

	int signalStatesQueueCurSize() const { return m_signalStatesQueueCurSize; }
	int signalStatesQueueCurMaxSize() const { return m_signalStatesQueueCurMaxSize; }

	void invalidateSignals(const QThread* thread);

	quint32 cachedAppDataUID() const { return m_cachedDiagDataUID; }

	bool statesQueueIsEmpty(QThread* thread) const;

private:
	virtual bool parseBuffer(ParsingBuffer& readBuffer, const QThread* thread) override;

	void wakeupStatesProcessingThread();

	int getAutoArchivingGroup(qint64 currentSysTime);

	void setAcquiredSignalsCount(int count) { m_acquiredSignalsCount = count; }

	void setSignalStatesQueueSize(int size) { m_signalStatesQueueSize = size; }
	void setSignalStatesQueueCurSize(int size) { m_signalStatesQueueCurSize = size; }
	void setSignalStatesQueueCurMaxSize(int size) { m_signalStatesQueueCurMaxSize = size; }

	virtual quint32 getExpectedDataUID() const override { return m_cachedDiagDataUID; }

private:
	std::mutex* m_statesProcessigRequiredMutex = nullptr;
	std::condition_variable* m_statesProcessingRequiredCondition = nullptr;
	std::queue<DiagDataSource*>* m_statesProcessingRequired = nullptr;

	//

	qint64 m_workcycle_ms = 0;
	QVector<DynamicDiagSignalState*> m_signalStates;

	int m_acquiredSignalsCount = 0;

//	SimpleAppSignalStatesArchiveFlagQueue m_signalStatesQueue;
//	GatewayAppSignalStatesQueue m_gatewaySignalStatesQueue;

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
//	int m_lastAutoArchivingGroup = DynamicAppSignalState::NOT_INITIALIZED_AUTOARCHIVING_GROUP;

	quint32 m_cachedDiagDataUID = 0;
};

class DiagDataSources
{
public:
	DiagDataSources();
	~DiagDataSources();

	bool init(const QString& profile,
			  const QVector<DataSource>& dataSources,
			  CircularLoggerShared logger);
	void clear();

	DiagDataSource* getSourceByIP(quint32 ip);
	DiagDataSource* getSignalSource(const QString& signalID);
	DiagDataSource* getSignalSource(Hash signalHash);

	std::vector<DiagDataSource*>::iterator begin();
	std::vector<DiagDataSource*>::const_iterator begin() const;

	std::vector<DiagDataSource*>::iterator end();
	std::vector<DiagDataSource*>::const_iterator end() const;

//	const std::map<QString, AppDataSource*>& sources() const;

private:
	// dynamic AppDataSource objects owner!
	//
	std::vector<DiagDataSource*> m_sources;

	// module EquipmentID => AppDataSource*
	//
	std::map<QString, DiagDataSource*> m_moduleToSource;

	// lan controller EquipmentID => AppDataSource*
	//
	std::map<QString, DiagDataSource*> m_lanControllerToSource;

	// module ethernet adapter IP => AppDataSource*
	//
	std::map<quint32, DiagDataSource*> m_ipToSource;

	// signal Hash => AppDataSource*
	//
	std::map<Hash, DiagDataSource*> m_signalToSource;
};
