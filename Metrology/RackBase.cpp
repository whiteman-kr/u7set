#include "RackBase.h"

#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackGroup::RackGroup(const QString& caption)
{
	setCaption(caption);
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroup::isValid() const
{
	if (m_index == -1 || m_caption.isEmpty() == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::clear()
{
	m_hash = UNDEFINED_HASH;
	m_caption.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::setCaption(const QString& caption)
{
	m_caption = caption;

	if (m_caption.isEmpty() == true)
	{
		m_hash = UNDEFINED_HASH;
		return;
	}

	m_hash = calcHash(m_caption);
}

// -------------------------------------------------------------------------------------------------------------------

QString RackGroup::rackID(int channel) const
{
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		return QString();
	}

	return m_rackEquipmentID[channel];
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::setRackID(int channel, const QString& rackID)
{
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		return;
	}

	m_rackEquipmentID[channel] = rackID;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackGroupBase::RackGroupBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupBase::clear()
{
	QMutexLocker l(&m_groupMutex);

	m_groupList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::count() const
{
	QMutexLocker l(&m_groupMutex);

	return m_groupList.count();
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::append(const RackGroup& group)
{
	if (group.isValid() == false)
	{
		assert(false);
		return -1;
	}

	QMutexLocker l(&m_groupMutex);

	m_groupList.append(group);

	return m_groupList.count() - 1;
}

// -------------------------------------------------------------------------------------------------------------------

RackGroup RackGroupBase::group(int index) const
{
	QMutexLocker l(&m_groupMutex);

	if (index < 0 || index >= m_groupList.count())
	{
		return RackGroup();
	}

	return m_groupList[index];
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::setGroup(int index, const RackGroup& group)
{
	QMutexLocker l(&m_groupMutex);

	if (index < 0 || index >= m_groupList.count())
	{
		return false;
	}

	m_groupList[index] = group;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::remove(int index)
{
	QMutexLocker l(&m_groupMutex);

	if (index < 0 || index >= m_groupList.count())
	{
		return false;
	}

	m_groupList.remove(index);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::load()
{
	QElapsedTimer responseTime;
	responseTime.start();

	SqlTable* table = theDatabase.openTable(SQL_TABLE_RACK_GROUP);
	if (table == nullptr)
	{
		return false;
	}

	int readedRecordCount = 0;

	m_groupMutex.lock();

		m_groupList.resize(table->recordCount());

		readedRecordCount = table->read(m_groupList.data());

	m_groupMutex.unlock();

	table->close();

	qDebug() << "RackBase::loadGroup() - Loaded rack groups: " <<
				readedRecordCount <<
				", Time for load: " <<
				responseTime.elapsed() <<
				" ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::save()
{
	SqlTable* table = theDatabase.openTable(SQL_TABLE_RACK_GROUP);
	if (table == nullptr)
	{
		return false;
	}

	if (table->clear() == false)
	{
		table->close();
		return false;
	}

	int writtenRecordCount = 0;

	m_groupMutex.lock();

		writtenRecordCount = table->write(m_groupList.data(), m_groupList.count());

	m_groupMutex.unlock();

	table->close();

	if (writtenRecordCount != count())
	{
		return false;
	}

	qDebug() << "RackBase::saveGroup() - Written rack groups: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

RackGroupBase& RackGroupBase::operator=(const RackGroupBase& from)
{
	QMutexLocker l(&m_groupMutex);

	m_groupList = from.m_groupList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackBase::RackBase(QObject *parent) :
	QObject(parent)
{
}

 // -------------------------------------------------------------------------------------------------------------------

void RackBase::clear()
{
	QMutexLocker l(&m_rackMutex);

	m_rackHashMap.clear();
	m_rackList.clear();
}


// -------------------------------------------------------------------------------------------------------------------

int RackBase::count() const
{
	QMutexLocker l(&m_rackMutex);

	return m_rackList.count();
}

// -------------------------------------------------------------------------------------------------------------------

int RackBase::append(const Metrology::RackParam& rack)
{
	if (rack.isValid() == false)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	QMutexLocker l(&m_rackMutex);

	if (m_rackHashMap.contains(rack.hash()) == false)
	{
		m_rackList.append(rack);
		index = m_rackList.count() - 1;

		m_rackHashMap.insert(rack.hash(), index);
	}

	 return index;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam* RackBase::rackPtr(const QString& rackID)
{
	if (rackID.isEmpty() == true)
	{
		assert(false);
		return nullptr;
	}

	return rackPtr(calcHash(rackID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam* RackBase::rackPtr(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return nullptr;
	}

	Metrology::RackParam* pRack = nullptr;

	QMutexLocker l(&m_rackMutex);

	if (m_rackHashMap.contains(hash) == true)
	{
		int index = m_rackHashMap[hash];

		if (index >= 0 && index < m_rackList.count())
		{
			pRack = &m_rackList[index];
		}
	}

	return pRack;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam* RackBase::rackPtr(int index)
{
	QMutexLocker l(&m_rackMutex);

	if (index < 0 || index >= m_rackList.count())
	{
		return nullptr;
	}

	return &m_rackList[index];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam RackBase::rack(const QString& rackID)
{
	if (rackID.isEmpty() == true)
	{
		assert(false);
		return Metrology::RackParam();
	}

	return rack(calcHash(rackID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam RackBase::rack(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return Metrology::RackParam();
	}

	Metrology::RackParam rack;

	m_rackMutex.lock();

		if (m_rackHashMap.contains(hash) == true)
		{
			int index = m_rackHashMap[hash];

			if (index >= 0 && index < m_rackList.count())
			{
				rack = m_rackList[index];
			}
		}

	m_rackMutex.unlock();

	return rack;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam RackBase::rack(int index)
{
	QMutexLocker l(&m_rackMutex);

	if (index < 0 || index >= m_rackList.count())
	{
		return Metrology::RackParam();
	}

	return m_rackList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(const QString& rackID, const Metrology::RackParam& rack)
{
	if (rackID.isEmpty() == true)
	{
		assert(false);
		return;
	}

	if (rack.isValid() == false)
	{
		return;
	}

	setRack(calcHash(rackID), rack);
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(const Hash& hash, const Metrology::RackParam& rack)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return;
	}

	if (rack.isValid() == false)
	{
		return;
	}

	QMutexLocker l(&m_rackMutex);

	if (m_rackHashMap.contains(rack.hash()) == true)
	{
		int index = m_rackHashMap[rack.hash()];

		if (index >= 0 && index < m_rackList.count())
		{
			m_rackList[index] = rack;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(int index, const Metrology::RackParam& rack)
{
	if (rack.isValid() == false)
	{
		return;
	}

	QMutexLocker l(&m_rackMutex);

	if (index < 0 || index >= m_rackList.count())
	{
		return;
	}

	m_rackList[index] = rack;
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::updateParamFromGroups()
{
	// clear all racks
	//
	m_rackMutex.lock();

		int rackCount = m_rackList.count();
		for(int i = 0; i < rackCount; i++)
		{
			Metrology::RackParam& r = m_rackList[i];
			if (r.isValid() == false)
			{
				continue;
			}

			r.setGroupIndex(-1);
			r.setChannel(-1);
		}

	m_rackMutex.unlock();

	// fill group param for all racks
	//
	int count = groups().count();
	for(int i = 0; i < count; i++)
	{
		RackGroup group = groups().group(i);
		if (group.isValid() == false)
		{
			continue;
		}

		for(int channel = 0; channel < Metrology::ChannelCount; channel++)
		{
			QString rackID = group.rackID(channel);
			if (rackID.isEmpty() == true)
			{
				continue;
			}

			Metrology::RackParam r = rack(rackID);
			if (r.isValid() == false)
			{
				continue;
			}

			r.setGroupIndex(group.Index());
			r.setChannel(channel);

			setRack(r.hash(), r);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

RackBase& RackBase::operator=(const RackBase& from)
{
	QMutexLocker l(&m_rackMutex);

	m_rackHashMap = from.m_rackHashMap;
	m_rackList = from.m_rackList;

	m_groupBase = from.m_groupBase;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
