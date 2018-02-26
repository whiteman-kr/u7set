#ifndef MONITORSCHEMAMANAGER_H
#define MONITORSCHEMAMANAGER_H

#include <QObject>
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaManager.h"
#include "MonitorConfigController.h"

class MonitorSchemaManager : public VFrame30::SchemaManager
{
	Q_OBJECT

public:
	explicit MonitorSchemaManager(MonitorConfigController* configController, QObject* parent = nullptr);

protected:
	virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

	// Slots
	//
protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

	// Data
	//
private:
	MonitorConfigController* const m_configController = nullptr;
};

#endif // MONITORSCHEMAMANAGER_H
