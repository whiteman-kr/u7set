#pragma once

#include "../libs/AppSignalLists/include/AppSignalLists/SignalListEditor.h"
#include <DbLib/DbObjectStorage.h>

class AppSignalListStorage : public DbObjectStorage<std::shared_ptr<AppSignalLists::AppSignalList>>
{
public:
	AppSignalListStorage() = delete;
	AppSignalListStorage(DbController* db);

	bool load(QString* errorMessage) override;
	bool save(const QUuid& uuid, QString* errorMessage) override;

};
