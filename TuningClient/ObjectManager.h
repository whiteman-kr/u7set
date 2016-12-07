#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Stable.h"
#include "TuningObject.h"
#include "../Proto/network.pb.h"
#include "../lib/Hash.h"


class ObjectManager
{
public:
	ObjectManager();

	TuningObject object(int index);

    int objectCount(); // this function must be used with mutex !!!

    TuningObject* objectPtr(int index); // this function must be used with mutex !!!

    TuningObject* objectPtrByHash(quint64 hash); // this function must be used with mutex !!!

    std::vector<TuningObject> objects();

	QStringList tuningSourcesEquipmentIds();

	bool loadSignals(const QByteArray& data, QString *errorCode);


public:

    QMutex m_mutex;

private:


	QStringList m_tuningSourcesList;

    std::map<quint64, int> m_objectsHashMap;

	std::vector<TuningObject> m_objects;
};


#endif // OBJECTMANAGER_H
