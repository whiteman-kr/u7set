#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Stable.h"
#include "TuningObject.h"



class TuningSource
{
public:
	QString m_equipmentId;

};

class ObjectManager
{
public:
	ObjectManager();

	int objectsCount();
	TuningObject object(int index);

	int tuningSourcesCount();
	TuningSource tuningSource(int index);

	bool loadSignals(const QByteArray& data, QString *errorCode);

private:

	QMutex m_mutex;

	std::vector<TuningSource> m_tuningSources;
	std::vector<TuningObject> m_objects;

};

extern ObjectManager theObjects;

#endif // OBJECTMANAGER_H
