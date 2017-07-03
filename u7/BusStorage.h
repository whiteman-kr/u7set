#ifndef BUSSTORAGE_H
#define BUSSTORAGE_H

#include "../lib/DbController.h"
#include "DbObjectStorage.h"
#include "../VFrame30/Bus.h"

class BusStorage : public DbObjectStorage<VFrame30::Bus>
{
public:
	BusStorage() = delete;
	BusStorage(DbController* db);

	bool load(QString* errorMessage) override;
	bool save(const QUuid& uuid, QString* errorMessage) override;
};

#endif // BUSSTORAGE_H
