#ifndef TUNINGSOURCEBASE_H
#define TUNINGSOURCEBASE_H

#include <QObject>

#include "SignalBase.h"

#include "../lib/Hash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignal.h"

#include "../Proto/network.pb.h"

// ==============================================================================================

class TuningSourceState
{
public:

	TuningSourceState() {}
	virtual ~TuningSourceState() {}

private:

	bool					m_isReply = false;
	qint64					m_requestCount = 0;
	qint64					m_replyCount = 0;
	int						m_commandQueueSize = 0;

public:

	bool					isReply() const { return m_isReply; }
	void					setIsReply(bool isReply) { m_isReply = isReply; }

	qint64					requestCount() const { return m_requestCount; }
	void					setRequestCount(qint64 count) { m_requestCount = count; }

	qint64					replyCount() const { return m_replyCount; }
	void					setReplyCount(qint64 count) { m_replyCount = count; }

	int						commandQueueSize() const { return m_commandQueueSize; }
	void					setCommandQueueSize(int size) { m_commandQueueSize = size; }
};

// ==============================================================================================

class TuningSource
{
public:

	TuningSource();
	explicit TuningSource(const Network::DataSourceInfo& info);
	virtual ~TuningSource();

private:

	qint64					m_sourceID = -1;

	QString					m_equipmentID;
	QString					m_caption;

	TuningSourceState		m_state;

public:

	qint64					sourceID() const { return m_sourceID; }
	void					setSourceID(qint64 id) { m_sourceID = id; }

	QString					equipmentID() const { return m_equipmentID; }
	void					setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString					caption() const { return m_caption; }
	void					setCaption(const QString& caption) { m_caption = caption; }

	TuningSourceState&		state() { return m_state; }
	void					setState(const TuningSourceState& state) { m_state = state; }
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
	QMap<qint64, int>		m_sourceIdMap;

public:

	void					clear();
	int						count() const;

	QStringList&			sourceEquipmentID() {  return m_tuningSourceEquipmentID; }

	int						append(const TuningSource& source);

	TuningSource			source(int index) const;

	TuningSourceState		state(qint64 sourceID);
	void					setState(qint64 sourceID, const Network::TuningSourceState& state);

	void					sortByID();
};

// ==============================================================================================

#endif // TUNINGSOURCEBASE_H
