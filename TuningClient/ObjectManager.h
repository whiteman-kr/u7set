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

	std::vector<TuningObject> objects();

	QStringList tuningSourcesEquipmentIds();

	bool loadSignals(const QByteArray& data, QString *errorCode);

private:

	QMutex m_mutex;

	QStringList m_tuningSourcesList;

	std::vector<TuningObject> m_objects;
};


#endif // OBJECTMANAGER_H
