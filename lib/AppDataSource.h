#pragma once

#include "../lib/DataSource.h"
#include "AppSignalStateEx.h"


class AppDataSource : public DataSource
{
public:
	static const int NO_INDEX = -1;
	static const int NO_AUTOARCHIVING_GROUP = -1;
	static const int NOT_INITIALIZED_AUTOARCHIVING_GROUP = -2;

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

		int index = NO_INDEX;		// index of signal in AppSignals and AddSignalStates

		QString appSignalID;

		void setSignalParams(int i, const Signal& s);
	};

public:
	AppDataSource(AppSignalStates& signalStates, AppSignalStatesQueue& signalStatesQueue);

	void prepare(const AppSignals& appSignals);

	bool parsePacket();

	bool getState(Network::AppDataSourceState* protoState) const;
	bool setState(const Network::AppDataSourceState& protoState);

private:
	int AppDataSource::getAutoArchivingGroup(qint64 currentSysTime);

private:
	AppSignalStatesQueue& m_signalStatesQueue;
	AppSignalStates& m_signalStates;

	AppSignals* m_appSignals = nullptr;

	QVector<SignalParseInfo> m_signalsParseInfo;


	// app data parsing
	//

	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;

	int getAutoArchivingGroup(qint64 currentSysTime);

private:
	static const int TIME_1S = 1000;

	int m_autoArchivingGroupsCount = 0;
	qint64 m_lastAutoArchivingTime = 0;
	int m_lastAutoArchivingGroup = NOT_INITIALIZED_AUTOARCHIVING_GROUP;

};

typedef std::shared_ptr<AppDataSource> AppDataSourceShared;

typedef QHash<QString, AppDataSourceShared> AppDataSources;		// app data source EquipmentID => AppDataSourceShared

typedef QHash<quint32, AppDataSourceShared> AppDataSourcesIP;	// app data source IP => AppDataSourceShared
