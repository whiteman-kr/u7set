#ifndef CLIENTFILTERSTORAGE_H
#define CLIENTFILTERSTORAGE_H

#include "../lib/Tuning/TuningFilter.h"

class TuningClientFilterStorage : public TuningFilterStorage
{
public:
	TuningClientFilterStorage();

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

};

#endif // CLIENTFILTERSTORAGE_H
