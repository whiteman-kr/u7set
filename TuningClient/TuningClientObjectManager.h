#ifndef TUNINGCLIENTOBJECTMANAGER_H
#define TUNINGCLIENTOBJECTMANAGER_H

#include "../lib/Tuning/TuningSignalManager.h"

class TuningClientObjectManager : public TuningSignalManager
{
public:
	TuningClientObjectManager();

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

};

#endif // TUNINGCLIENTOBJECTMANAGER_H
