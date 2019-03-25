#pragma once

#include "../lib/Signal.h"
#include "../lib/DataSource.h"
#include "DynamicAppSignalState.h"

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

		int index = DynamicAppSignalState::NO_INDEX;		// index of signal in AppSignals and AppSignalStates

		QString appSignalID;

		void setSignalParams(int i, const Signal& s);
	};

public:
	AppDataSource();
	AppDataSource(const DataSource& dataSource);

	void prepare(const AppSignals& appSignals, DynamicAppSignalStates* signalStates, int autoArchivingGroupsCount);

	bool parsePacket();

	bool getState(Network::AppDataSourceState* proto) const;
	void setState(const Network::AppDataSourceState& proto);

	bool getSignalState(SimpleAppSignalState* state, const QThread* thread);

	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }

	int signalStatesQueueSize() const { return m_signalStatesQueueSize; }
	int signalStatesQueueMaxSize() const { return m_signalStatesQueueMaxSize; }

private:
	int getAutoArchivingGroup(qint64 currentSysTime);

	bool getDoubleValue(const char* rupData, int rupDataSize, const SignalParseInfo& parseInfo, double& value);
	bool getValidity(const char* rupData, int rupDataSize, const SignalParseInfo& parseInfo, quint32& validity);

	void setAcquiredSignalsCount(int count) { m_acquiredSignalsCount = count; }

	void setSignalStatesQueueSize(int size) { m_signalStatesQueueSize = size; }
	void setSignalStatesQueueMaxSize(int size) { m_signalStatesQueueMaxSize = size; }

private:
	DynamicAppSignalStates* m_signalStates = nullptr;

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
	int m_lastAutoArchivingGroup = DynamicAppSignalState::NOT_INITIALIZED_AUTOARCHIVING_GROUP;
};

typedef std::shared_ptr<AppDataSource> AppDataSourceShared;

typedef QHash<QString, AppDataSourceShared> AppDataSources;		// app data source EquipmentID => AppDataSourceShared

typedef QHash<quint32, AppDataSourceShared> AppDataSourcesIP;	// app data source IP => AppDataSourceShared

typedef QHash<Hash, AppDataSourceShared> SignalsToSources;		// signal Hash => AppDataSourceShared
