#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Stable.h"
#include "TuningObject.h"



class ObjectManager
{
public:
	ObjectManager();

	int objectsCount();
	const std::shared_ptr<TuningObject> const_object(int index);

	//bool load(const QString& fileName);

	bool load(const QByteArray& data, QString *errorCode);

private:

	QMutex m_mutex;

	std::vector<std::shared_ptr<TuningObject>> m_objects;

};

extern ObjectManager theObjects;

#endif // OBJECTMANAGER_H
