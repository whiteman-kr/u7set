#ifndef BUSSTORAGE_H
#define BUSSTORAGE_H

#include "../AppSignalLib/Bus.h"
#include "../DbLib/DbObjectStorage.h"

class BusStorage : public DbObjectStorage<std::shared_ptr<AppSignalLib::Bus>>
{
public:
	BusStorage() = delete;
	BusStorage(DbController* db);

	bool load(QString* errorMessage) override;
	bool save(const QUuid& uuid, QString* errorMessage) override;


	bool reload(const QUuid& uuid);
};

#endif // BUSSTORAGE_H
