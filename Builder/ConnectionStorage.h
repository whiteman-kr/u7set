#pragma once
#include "../HardwareLib/Connection.h"
#include "../DbLib/DbController.h"
#include "../DbLib/DbObjectStorage.h"

namespace Builder
{
	class ConnectionStorage : public DbObjectStorage<std::shared_ptr<Hardware::Connection>>
	{
	public:
		explicit ConnectionStorage(DbController* db);
		virtual ~ConnectionStorage();

	public:
		using DbObjectStorage::get;

		std::vector<std::shared_ptr<Hardware::Connection>> get(const QStringList& masks) const;

		QStringList connectionIds() const;
		QStringList filterByMoudules(const QStringList& modules) const;

		std::shared_ptr<Hardware::Connection> getPortConnection(QString portEquipmentId) const;

		std::vector<std::shared_ptr<Hardware::Connection>> getConnections() const;

		// --
		//
		bool load(QString* errorMessage) override;
		bool save(const QUuid& uuid, QString* errorMessage) override;

		bool loadFromConnectionsFolder(QString* errorMessage);
		bool loadFromXmlDeprecated(QString* errorMessage);

		bool deleteXmlDeprecated(QString* errorMessage);
	};
}
