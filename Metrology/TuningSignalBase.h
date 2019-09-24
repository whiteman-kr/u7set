#ifndef TUNINGSIGNALBASE_H
#define TUNINGSIGNALBASE_H

#include <QObject>

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/MetrologySignal.h"

namespace Network
{
	class DataSourceInfo;
	class TuningSourceState;
	class TuningSignalState;
}

// ==============================================================================================

class TuningSourceState
{
public:

	TuningSourceState() {}
	virtual ~TuningSourceState() {}

private:

	bool					m_isReply = false;
	quint64					m_requestCount = 0;
	quint64					m_replyCount = 0;
	int						m_commandQueueSize = 0;

public:

	bool					isReply() const { return m_isReply; }
	void					setIsReply(bool isReply) { m_isReply = isReply; }

	quint64					requestCount() const { return m_requestCount; }
	void					setRequestCount(quint64 count) { m_requestCount = count; }

	quint64					replyCount() const { return m_replyCount; }
	void					setReplyCount(quint64 count) { m_replyCount = count; }

	int						commandQueueSize() const { return m_commandQueueSize; }
	void					setCommandQueueSize(int size) { m_commandQueueSize = size; }
};

// ==============================================================================================

class TuningSource
{
public:

	TuningSource();
	explicit TuningSource(const Network::DataSourceInfo& info);
	virtual ~TuningSource() {}

private:

	quint64					m_sourceID = 0;

	QString					m_equipmentID;
	QString					m_caption;

	QString					m_serverIP;
	int						m_serverPort = 0;

	QString					m_channel;

	QString					m_subSystem;

	int						m_lmNumber = -1;

	TuningSourceState		m_state;

public:

	quint64					sourceID() const { return m_sourceID; }
	void					setSourceID(quint64 id) { m_sourceID = id; }

	QString					equipmentID() const { return m_equipmentID; }
	void					setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString					caption() const { return m_caption; }
	void					setCaption(const QString& caption) { m_caption = caption; }

	QString					serverIP() const { return m_serverIP; }
	void					setServerIP(const QString& ip) { m_serverIP = ip; }

	int						serverPort() const { return m_serverPort; }
	void					setServerPort(int port) { m_serverPort = port; }

	QString					channel() const { return m_channel; }
	void					setChannel(const QString& channel) { m_channel = channel; }

	QString					subSystem() const { return m_subSystem; }
	void					setSubSystem(const QString& subSystem) { m_subSystem = subSystem; }

	int						lmNumber() const { return m_lmNumber; }
	void					setLmNumber(int lmNumber) { m_lmNumber = lmNumber; }

	TuningSourceState&		state() { return m_state; }
	void					setState(TuningSourceState state) { m_state = state; }
};

// ==============================================================================================

class TuningSourceBase : public QObject
{
	Q_OBJECT

public:

	explicit TuningSourceBase(QObject *parent = nullptr);
	virtual ~TuningSourceBase();

private:

	QStringList				m_tuningSourceEquipmentID;

	mutable QMutex			m_sourceMutex;
	QVector<TuningSource>	m_sourceList;
	QMap<quint64, int>		m_sourceIdMap;

public:

	void					clear();
	int						count() const;

	QStringList&			sourceEquipmentID() {  return m_tuningSourceEquipmentID; }

	int						append(const TuningSource& source);

	TuningSource			source(int index) const;

	TuningSourceState		state(quint64 sourceID);
	void					setState(quint64 sourceID, const Network::TuningSourceState& state);

	void					sortByID();
};

// ==============================================================================================

class TuningSignalBase : public QObject
{
	Q_OBJECT

public:

	explicit TuningSignalBase(QObject *parent = nullptr);
	virtual ~TuningSignalBase();

private:

	mutable QMutex			m_signalMutex;
	QVector<Metrology::Signal*> m_signalList;
	QMap<Hash, int>			m_signalHashMap;


public:

	void					clear();
	int						count() const;

	void					createSignalList();

	int						append(Metrology::Signal* pSignal);

	Metrology::Signal*		signal(const Hash& hash) const;
	Metrology::Signal*		signal(int index) const;

	Metrology::SignalState	state(const Hash& hash);
	void					setState(const Network::TuningSignalState& state);

	void					setNovalid();

signals:

	void					signalsCreated();
};

// ==============================================================================================

class TuningWriteCmd
{
public:

	TuningWriteCmd() { clear(); }
	TuningWriteCmd(const Hash &signalHash, TuningValueType type, double value) : m_signalHash (signalHash), m_type (type), m_value (value) {}
	virtual ~TuningWriteCmd() {}

private:

	Hash					m_signalHash = UNDEFINED_HASH;
	TuningValueType			m_type = TuningValueType::Discrete;
	QVariant				m_value;

public:

	void					clear()
	{
		m_signalHash = UNDEFINED_HASH;
		m_type = TuningValueType::Discrete;
		m_value.clear();
	}

	bool					isEmpty()
	{
		if (m_signalHash == UNDEFINED_HASH)
		{
			return true;
		}

		return false;
	}

	Hash					signalHash() const { return m_signalHash; }
	void					setSignalHash(Hash hash) { m_signalHash = hash; }

	TuningValueType			type() const { return m_type; }
	int						typeInt() const { return TO_INT(m_type); }
	void					setType(TuningValueType valueType) { m_type = valueType; }

	QVariant				value() const { return m_value; }
	void					setValue(QVariant value) { m_value = value; }
};

// ==============================================================================================

class TuningBase: public QObject
{
	Q_OBJECT

public:

	explicit TuningBase(QObject *parent = nullptr);
	virtual ~TuningBase();

private:

	TuningSourceBase		m_sourceBase;
	TuningSignalBase		m_signalsBase;

	mutable QMutex			m_cmdFowWriteMutex;
	QVector<TuningWriteCmd>	m_cmdFowWriteList;

public:

	void					clear();

	// sources and signal
	//
	TuningSourceBase&		Sources() { return m_sourceBase; }
	TuningSignalBase&		Signals() { return m_signalsBase; }

	// Commands for write
	//
	int						cmdFowWriteCount() const;

	void					appendCmdFowWrite(const TuningWriteCmd& cmd);
	void					appendCmdFowWrite(const Hash& signalHash, TuningValueType type, QVariant value);

	TuningWriteCmd			cmdFowWrite();

signals:

public slots:


};

// ==============================================================================================

#endif // SIGNALBASE_H
