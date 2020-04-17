#include "TuningSourceBase.h"

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSource class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSource::TuningSource()
{
}

TuningSource::TuningSource(const Network::DataSourceInfo& info) :
	m_sourceID (static_cast<qint64>(info.id())),
	m_equipmentID (QString::fromStdString(info.lmequipmentid())),
	m_caption (QString::fromStdString(info.lmcaption()))
{
}

TuningSource::~TuningSource()
{
}

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSourceBase class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSourceBase::TuningSourceBase(QObject *parent) :
	QObject(parent)
{
}

TuningSourceBase::~TuningSourceBase()
{
}

void TuningSourceBase::clear()
{
	m_sourceMutex.lock();

		m_tuningSourceEquipmentID.clear();

		m_sourceList.clear();
		m_sourceIdMap.clear();

	m_sourceMutex.unlock();
}

int TuningSourceBase::count() const
{
	int count = 0;

	m_sourceMutex.lock();

		count = m_sourceList.count();

	m_sourceMutex.unlock();

	return count;
}

int TuningSourceBase::append(const TuningSource& source)
{
	int index = -1;

	m_sourceMutex.lock();

		if (m_sourceIdMap.contains(source.sourceID()) == false)
		{
			m_sourceList.append(source);

			index = m_sourceList.count() - 1;

			m_sourceIdMap[source.sourceID() ] = index;
		}

	m_sourceMutex.unlock();

	return index;
}

TuningSource TuningSourceBase::source(int index) const
{
	TuningSource source;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			source = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return source;
}

TuningSourceState TuningSourceBase::state(qint64 sourceID)
{
	TuningSourceState state;

	m_sourceMutex.lock();

		if (m_sourceIdMap.contains(sourceID) == true)
		{
			int index = m_sourceIdMap[sourceID];

			if (index >= 0 && index < m_sourceList.count())
			{
				state = m_sourceList[index].state();
			}
		}

	m_sourceMutex.unlock();

	return state;
}

void TuningSourceBase::setState(qint64 sourceID, const Network::TuningSourceState& state)
{
	m_sourceMutex.lock();

		if (m_sourceIdMap.contains(sourceID) == true)
		{
			int index = m_sourceIdMap[sourceID];

			if (index >= 0 && index < m_sourceList.count())
			{
				TuningSourceState& sourceState = m_sourceList[index].state();

				sourceState.setIsReply(state.isreply());
				sourceState.setRequestCount(state.requestcount());
				sourceState.setReplyCount(state.replycount());
				sourceState.setCommandQueueSize(state.commandqueuesize());
			}
		}

	m_sourceMutex.unlock();
}

void TuningSourceBase::sortByID()
{
	std::sort(m_sourceList.begin(), m_sourceList.end(),
			[](const TuningSource& ts1, const TuningSource& ts2) -> bool
			{
				return ts1.equipmentID() < ts2.equipmentID();
			});
}
