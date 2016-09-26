#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Stable.h"
#include "TuningObject.h"
#include "../Proto/network.pb.h"
#include "../Lib/Hash.h"



class ObjectManager
{
public:
	ObjectManager();

	int objectsCount();
	TuningObject object(int index);

	int tuningSourcesCount();
	QString tuningSourceEquipmentId(int index);

	bool loadSignals(const QByteArray& data, QString *errorCode);


private:

	QMutex m_mutex;

	QStringList m_tuningSourcesList;

	//std::map<quint64, TuningSource> m_tuningSourcesMap;

	std::vector<TuningObject> m_objects;

};

extern ObjectManager theObjects;

#endif // OBJECTMANAGER_H
