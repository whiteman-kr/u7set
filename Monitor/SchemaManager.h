#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QObject>
#include "../VFrame30/Schema.h"
#include "MonitorConfigController.h"

class SchemaManager : public QObject
{
	Q_OBJECT
public:
	explicit SchemaManager(MonitorConfigController* configController);
	virtual ~SchemaManager();

	// Methods to use
	//
public:
	std::shared_ptr<VFrame30::Schema> schema(QString schemaId);

protected:
	std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId);

	// Signals
	//
signals:

	// Is emmited when SchemaManager resets all schemas and wants everybody to set current schema to start schema
	//
	void resetSchema(QString startSchemaID);

	// slots
	//
public slots:

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

	// Properties
	//
public:
	const QString& globalScript() const;

	// Data
	//
private:
	std::map<QString, std::shared_ptr<VFrame30::Schema>> m_schemas;		// map by SchemaID

	MonitorConfigController* const m_configController = nullptr;

	ConfigSettings m_configuration;
};

#endif // SCHEMAMANAGER_H
