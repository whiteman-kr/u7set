#ifndef MONITORSCHEMAMANAGER_H
#define MONITORSCHEMAMANAGER_H

#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaManager.h"
#include "MonitorConfigController.h"

class MonitorSchemaManager : public VFrame30::SchemaManager
{
	Q_OBJECT

public:
	explicit MonitorSchemaManager(MonitorConfigController* configController, QObject* parent = nullptr);

public:
	bool hasSchema(QString schemaId) const;

protected:
	virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

	// Slots
	//
protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

public:
	[[nodiscard]] MonitorConfigController* monitorConfigController();
	[[nodiscard]] const MonitorConfigController* monitorConfigController() const;


	QString onConfigurationArrivedScript() const;
	void setOnConfigurationArrivedScript(QString value);

	// Data
	//
private:
	MonitorConfigController* const m_configController = nullptr;

	QString m_onConfigurationArrivedScript;
};

#endif // MONITORSCHEMAMANAGER_H
