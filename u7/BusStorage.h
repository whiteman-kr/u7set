#ifndef BUSSTORAGE_H
#define BUSSTORAGE_H

#include "../lib/DbController.h"
#include "DbObjectStorage.h"
#include "../VFrame30/Bus.h"

class BusStorage : public DbObjectStorage<std::shared_ptr<VFrame30::Bus>>
{
public:
	BusStorage() = delete;
	BusStorage(DbController* db);

	bool load(QString* errorMessage) override;
	bool save(const QUuid& uuid, QString* errorMessage) override;


	bool reload(const QUuid& uuid);
};

#endif // BUSSTORAGE_H
