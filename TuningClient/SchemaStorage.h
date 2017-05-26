#ifndef SCHEMASTORAGE_H
#define SCHEMASTORAGE_H

#include <QObject>
#include "ConfigController.h"
#include "../VFrame30/Schema.h"

class SchemaStorage : public QObject
{
public:
	explicit SchemaStorage(ConfigController* configController);

	std::shared_ptr<VFrame30::Schema> schema(QString schemaId);

protected:
	std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId);

private:
	std::map<QString, std::shared_ptr<VFrame30::Schema>> m_schemas;		// map by SchemaID

	ConfigController* m_configController = nullptr;

};

#endif // SCHEMASTORAGE_H
