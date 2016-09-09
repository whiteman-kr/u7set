#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Stable.h"
#include "TuningObject.h"



class ObjectManager
{
public:
	ObjectManager();

	int objectsCount() const;
	TuningObject *object(int index);

private:
	std::vector<TuningObject> m_objects;
};

extern ObjectManager theObjects;

#endif // OBJECTMANAGER_H
