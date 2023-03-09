#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "../lib/DataSource.h"
#include "../OnlineLib/CircularLogger.h"
#include "DynamicAppSignalState.h"

class AppSignals : public HashedVector<QString, AppSignal*>
{
private:
	QHash<Hash, AppSignal*> m_hash2Signal;

public:
	~AppSignals();

	void clear();
	void buildHash2Signal();

	const AppSignal* getSignal(Hash hash) const;
};

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

	bool getState(Network::AppDataSourceState* proto) const;
	void setState(const Network::AppDataSourceState& proto);

	bool getSignalState(SimpleAppSignalStateArchiveFlag* state, const QThread* thread);

	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }

	int signalStatesQueueSize() const { return m_signalStatesQueueSize; }
	int signalStatesQueueCurSize() const { return m_signalStatesQueueCurSize; }
	int signalStatesQueueCurMaxSize() const { return m_signalStatesQueueCurMaxSize; }

private:
	bool parsePacket();

	virtual bool parseBuffer(ParsingBuffer& readBuffer, const QThread* thread) override;

	int getAutoArchivingGroup(qint64 currentSysTime);

	void setAcquiredSignalsCount(int count) { m_acquiredSignalsCount = count; }

	void setSignalStatesQueueSize(int size) { m_signalStatesQueueSize = size; }
	void setSignalStatesQueueCurSize(int size) { m_signalStatesQueueCurSize = size; }
	void setSignalStatesQueueCurMaxSize(int size) { m_signalStatesQueueCurMaxSize = size; }

private:
	QVector<DynamicAppSignalState*> m_signalStates;

	int m_acquiredSignalsCount = 0;

	SimpleAppSignalStatesArchiveFlagQueue m_signalStatesQueue;

	int m_signalStatesQueueSize = 0;
	int m_signalStatesQueueCurSize = 0;
	int m_signalStatesQueueCurMaxSize = 0;

	// app data parsing
	//
	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;

	static const int TIME_1S = 1000;

	int m_autoArchivingGroupsCount = 0;
	qint64 m_lastAutoArchivingTime = 0;
	int m_lastAutoArchivingGroup = DynamicAppSignalState::NOT_INITIALIZED_AUTOARCHIVING_GROUP;
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

	const std::map<QString, AppDataSource*>& sources() const;

private:
	// module EquipmentID => AppDataSource*
	// dynamic AppDataSource objects owner!
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
