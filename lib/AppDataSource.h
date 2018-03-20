#pragma once

#include "../lib/Signal.h"
#include "../lib/DataSource.h"

struct AppSignalStateEx
{
public:
	AppSignalStateEx();

	void setSignalParams(int index, Signal* signal);
	bool setState(Times time, quint32 validity, double value, int autoArchivingGroup);

	void invalidate() { m_current.flags.all = 0; }

	Hash hash() const;

	QString appSignalID() const;

	friend class AppSignalStates;

	const SimpleAppSignalState& current() const { return m_current; }
	const SimpleAppSignalState& stored() const { return m_stored; }

	void setAutoArchivingGroup(int groupsCount);

private:
	SimpleAppSignalState m_current;
	SimpleAppSignalState m_stored;

	// paramters needed to update state
	//
	bool m_initialized = false;
	bool m_isDiscreteSignal = false;

	bool m_adaptiveAperture = false;

	double m_coarseAperture = 0;
	double m_fineAperture = 0;

	double m_lowLimit = 0;
	double m_highLimit = 0;

	//

	double m_absRoughAperture = 0;
	double m_absSmoothAperture = 0;

	int m_autoArchivingGroup = -2;

	Signal* m_signal = nullptr;
	int m_index = 0;
};


class AppSignalStates
{
private:
	QMutex m_allMutex;

	AppSignalStateEx* m_appSignalState = nullptr;
	int m_size = 0;

	QHash<Hash, const AppSignalStateEx*> m_hash2State;

public:
	~AppSignalStates();

	void clear();

	void setSize(int size);

	int size() const { return m_size; }

	AppSignalStateEx* operator [] (int index);

	void buidlHash2State();

	bool getCurrentState(Hash hash, AppSignalState& state) const;
	bool getStoredState(Hash hash, AppSignalState& state) const;

	void setAutoArchivingGroups(int autoArchivingGroupsCount);
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
	AppDataSource();
	AppDataSource(AppSignalStates* signalStates, AppSignalStatesQueue* signalStatesQueue);

	void prepare(const AppSignals& appSignals);

	bool parsePacket();

	bool getState(Network::AppDataSourceState* protoState) const;
	bool setState(const Network::AppDataSourceState& protoState);

private:
	int getAutoArchivingGroup(qint64 currentSysTime);

private:
	AppSignalStatesQueue* m_signalStatesQueue = nullptr;
	AppSignalStates* m_signalStates = nullptr;

	AppSignals* m_appSignals = nullptr;

	QVector<SignalParseInfo> m_signalsParseInfo;

	// app data parsing
	//
	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;

private:
	static const int TIME_1S = 1000;

	int m_autoArchivingGroupsCount = 0;
	qint64 m_lastAutoArchivingTime = 0;
	int m_lastAutoArchivingGroup = NOT_INITIALIZED_AUTOARCHIVING_GROUP;

};

typedef std::shared_ptr<AppDataSource> AppDataSourceShared;

typedef QHash<QString, AppDataSourceShared> AppDataSources;		// app data source EquipmentID => AppDataSourceShared

typedef QHash<quint32, AppDataSourceShared> AppDataSourcesIP;	// app data source IP => AppDataSourceShared
