#ifndef BUSSTORAGE_H
#define BUSSTORAGE_H

#include "../lib/DbController.h"
#include "DbObjectStorage.h"
#include "../VFrame30/Bus.h"

class BusStorage : public DbObjectStorage<VFrame30::Bus>
{
public:
	BusStorage() = delete;
	BusStorage(DbController* db, QWidget* parentWidget);

	bool load() override;
	bool save(const QUuid& uuid) override;
};

#endif // BUSSTORAGE_H
