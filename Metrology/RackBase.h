#pragma once

#include "../UtilsLib/Hash.h"
#include "MetrologySignal.h"

// ==============================================================================================

class RackGroup
{
public:

	RackGroup() {}
	RackGroup(const QString& caption);
	virtual ~RackGroup() {}

public:

	bool isValid() const;
	void clear();

	int Index() const { return m_index; }
	void setIndex(int index) { m_index = index; }

	Hash hash() const { return m_hash; }

	QString caption() const { return m_caption; }
	void setCaption(const QString& caption);

	QString rackID(int channel) const;
	void setRackID(int channel, const QString& rackID);

private:

	int m_index = -1;

	Hash m_hash = UNDEFINED_HASH;			// hash calcHash from rack group caption
	QString m_caption;							// rack group caption

	QString m_rackEquipmentID[Metrology::ChannelCount];
};

// ==============================================================================================

class RackGroupBase : public QObject
{
	Q_OBJECT

public:

	explicit RackGroupBase(QObject* parent = nullptr);
	virtual ~RackGroupBase() override {}

public:

	void clear();
	int count() const;

	int append(const RackGroup& group);

	RackGroup group(int index) const;
	bool setGroup(int index, const RackGroup& group);

	bool remove(int index);

	int load();
	bool save();

	RackGroupBase& operator=(const RackGroupBase& from);

private:

	mutable QMutex m_groupMutex;
	std::vector<RackGroup> m_groupList;
};

// ==============================================================================================

class RackBase : public QObject
{
	Q_OBJECT

public:

	explicit RackBase(QObject* parent = nullptr);
	virtual ~RackBase() override {}

public:

	void clear();
	int count() const;

	int append(const Metrology::RackParam& rack);

	Metrology::RackParam* rackPtr(const QString& rackID);
	Metrology::RackParam* rackPtr(const Hash& hash);
	Metrology::RackParam* rackPtr(int index);

	Metrology::RackParam rack(const QString& rackID);
	Metrology::RackParam rack(const Hash& hash);
	Metrology::RackParam rack(int index);

	void setRack(const QString& rackID, const Metrology::RackParam& rack);
	void setRack(const Hash& hash, const Metrology::RackParam& rack);
	void setRack(int index, const Metrology::RackParam& rack);

	RackGroupBase& groups() { return m_groupBase; }
	void updateParamFromGroups();

	RackBase& operator=(const RackBase& from);

private:

	mutable QMutex m_rackMutex;
	QMap<Hash, int> m_rackHashMap;
	std::vector<Metrology::RackParam> m_rackList;

	RackGroupBase m_groupBase;
};

// ==============================================================================================
