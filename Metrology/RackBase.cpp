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
	m_hash = 0;
	m_caption.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::setCaption(const QString& caption)
{
	m_caption = caption;

	if (m_caption.isEmpty() == true)
	{
		m_hash = 0;
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
	m_groupMutex.lock();

		m_groupList.clear();

	m_groupMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::count() const
{
	int count = 0;

	m_groupMutex.lock();

		count = m_groupList.size();

	m_groupMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::append(const RackGroup& group)
{
	if (group.isValid() == false)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_groupMutex.lock();

		m_groupList.append(group);
		index = m_groupList.size() - 1;

	m_groupMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

RackGroup RackGroupBase::group(int index) const
{
	RackGroup group;

	m_groupMutex.lock();

		if (index >= 0 && index < m_groupList.size())
		{
			group = m_groupList[index];
		}

	m_groupMutex.unlock();

	return group;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::setGroup(int index, const RackGroup& group)
{
	bool result = false;

	m_groupMutex.lock();

		if (index >= 0 && index < m_groupList.size())
		{
			m_groupList[index] = group;

			result = true;
		}

	m_groupMutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::remove(int index)
{
	bool result = false;

	m_groupMutex.lock();

		if (index >= 0 && index < m_groupList.size())
		{
			m_groupList.remove(index);

			result = true;
		}

	m_groupMutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::load()
{
	if (thePtrDB == nullptr)
	{
		return 0;
	}

	QTime responseTime;
	responseTime.start();

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_RACK_GROUP);
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

	qDebug() << "RackBase::loadGroup() - Loaded rack groups: " << readedRecordCount << ", Time for load: " << responseTime.elapsed() << " ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::save()
{
	if (thePtrDB == nullptr)
	{
		return false;
	}

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_RACK_GROUP);
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
	m_groupMutex.lock();

		m_groupList = from.m_groupList;

	m_groupMutex.unlock();

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
	m_rackMutex.lock();

		m_rackHashMap.clear();
		m_rackList.clear();

	m_rackMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

int RackBase::count() const
{
	int count = 0;

	m_rackMutex.lock();

		count = m_rackList.size();

	m_rackMutex.unlock();

	return count;
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

	m_rackMutex.lock();

		if (m_rackHashMap.contains(rack.hash()) == false)
		{
			m_rackList.append(rack);
			index = m_rackList.size() - 1;

			m_rackHashMap.insert(rack.hash(), index);
		}

	 m_rackMutex.unlock();

	 return index;
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
	if (hash == 0)
	{
		assert(hash != 0);
		return Metrology::RackParam();
	}

	Metrology::RackParam rack;

	m_rackMutex.lock();

		if (m_rackHashMap.contains(hash) == true)
		{
			int index = m_rackHashMap[hash];

			if (index >= 0 && index < m_rackList.size())
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
	Metrology::RackParam rack;

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.size())
		{
			rack = m_rackList[index];
		}

	m_rackMutex.unlock();

	return rack;
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
	if (hash == 0)
	{
		assert(hash != 0);
		return;
	}

	if (rack.isValid() == false)
	{
		return;
	}

	m_rackMutex.lock();

		if (m_rackHashMap.contains(rack.hash()) == true)
		{
			int index = m_rackHashMap[rack.hash()];

			if (index >= 0 && index < m_rackList.size())
			{
				m_rackList[index] = rack;
			}
		}

	m_rackMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(int index, const Metrology::RackParam& rack)
{
	if (rack.isValid() == false)
	{
		return;
	}

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.size())
		{
			m_rackList[index] = rack;
		}

	m_rackMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::updateParamFromGroups()
{
	// clear all racks
	//
	m_rackMutex.lock();

		int rackCount = m_rackList.size();
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
	m_rackMutex.lock();

		m_rackHashMap = from.m_rackHashMap;
		m_rackList = from.m_rackList;

		m_groupBase = from.m_groupBase;

	m_rackMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
