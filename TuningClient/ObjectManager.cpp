#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
	for (int i = 0; i < 100; i++)
	{
		TuningObject o;

		o.setAppSignalID(QString("APPSIGNALID%1").arg(i));
		o.setCustomAppSignalID(QString("CUSTAPPSIGNALID%1").arg(i));
		o.setEquipmentID(QString("SYSTEM_RACK00_CH00_MD00_CTRLIN_IN%1").arg(i));
		o.setCaption(QString("Signal%1").arg(i));
		o.setUnits("%%");

		m_objects.push_back(o);
	}

	for (int i = 100; i < 200; i++)
	{
		TuningObject o;

		o.setAppSignalID(QString("APPSIGNALID%1").arg(i));
		o.setCustomAppSignalID(QString("CUSTAPPSIGNALID%1").arg(i));
		o.setEquipmentID(QString("SYSTEM_RACK00_CH00_MD00_CTRLOUT_IN%1").arg(i));
		o.setCaption(QString("Signal%1").arg(i));
		o.setUnits("%%");

		m_objects.push_back(o);
	}

}

int ObjectManager::objectsCount() const
{
	return (int)m_objects.size();

}

TuningObject* ObjectManager::object(int index)
{
	return &m_objects[index];
}

ObjectManager theObjects;

