#pragma once

#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaManager.h"
#include "MonitorConfigController.h"

class MonitorSchemaManager : public VFrame30::SchemaManager
{
	Q_OBJECT

public:
	explicit MonitorSchemaManager(MonitorConfigController* configController, QObject* parent = nullptr);

public:
	[[nodiscard]] bool hasSchema(QString schemaId) const;

protected:
	[[nodiscard]] virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

public:
	[[nodiscard]] virtual int schemaCount() const override;
	[[nodiscard]] virtual std::shared_ptr<VFrame30::Schema> schemaByIndex(int schemaIndex) override;

	[[nodiscard]] virtual QString schemaCaptionById(const QString& schemaId) const override;
	[[nodiscard]] virtual QString schemaCaptionByIndex(int schemaIndex) const override;
	[[nodiscard]] virtual QString schemaIdByIndex(int schemaIndex) const override;

	// Slots
	//
protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

public:
	[[nodiscard]] MonitorConfigController* monitorConfigController();
	[[nodiscard]] const MonitorConfigController* monitorConfigController() const;

	[[nodiscard]] QString onConfigurationArrivedScript() const;
	void setOnConfigurationArrivedScript(QString value);

	// Data
	//
private:
	MonitorConfigController* const m_configController = nullptr;

	QString m_onConfigurationArrivedScript;
};

