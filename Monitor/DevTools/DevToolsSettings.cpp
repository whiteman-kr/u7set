#include "DevToolsSettings.h"
#include "../MonitorAppSettings.h"

namespace Monitor
{
	QString Monitor::DevToolsSettings::appEquipmentId() const
	{
		auto currentSettings = MonitorAppSettings::instance().get();
		return currentSettings.equipmentId;
	}

	std::vector<SchemaClientLib::IDevToolsAppSettings::Record> DevToolsSettings::settings() const
	{
		auto currentSettings = MonitorAppSettings::instance().get();

		std::vector<SchemaClientLib::IDevToolsAppSettings::Record> result;

		// windowCaption
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "General";
			record.key = "windowCaption";
			record.value = currentSettings.windowCaption;
			result.push_back(record);
		}

		// language
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "General";
			record.key = "language";
			record.value = currentSettings.language;
			result.push_back(record);
		}

		// cfgSrvIpAddress1
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "Configuration Service";
			record.key = "cfgSrvIpAddress1";
			record.value = QString{"%1:%2"}.arg(currentSettings.cfgSrvIpAddress1).arg(currentSettings.cfgSrvPort1);
			result.push_back(record);
		}

		// cfgSrvIpAddress2
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "Configuration Service";
			record.key = "cfgSrvIpAddress2";
			record.value = QString{"%1:%2"}.arg(currentSettings.cfgSrvIpAddress2).arg(currentSettings.cfgSrvPort2);
			result.push_back(record);
		}

		// requestTimeIntervalMs
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "Connection";
			record.key = "requestTimeIntervalMs";
			record.value = QString::number(currentSettings.requestTimeIntervalMs);
			result.push_back(record);
		}

		// showSchemasTabBar
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "View";
			record.key = "showSchemasTabBar";
			record.value = currentSettings.showSchemasTabBar ? "true" : "false";
			result.push_back(record);
		}

		// showLogo
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "View";
			record.key = "showLogo";
			record.value = currentSettings.showLogo ? "true" : "false";
			result.push_back(record);
		}

		// showItemsLabels
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "View";
			record.key = "showItemsLabels";
			record.value = currentSettings.showItemsLabels ? "true" : "false";
			result.push_back(record);
		}

		// zoomMode
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "View";
			record.key = "zoomMode";
			record.value = QString::number(static_cast<int>(currentSettings.zoomMode));
			result.push_back(record);
		}

		// singleInstance
		{
			SchemaClientLib::IDevToolsAppSettings::Record record;
			record.section = "General";
			record.key = "singleInstance";
			record.value = currentSettings.singleInstance ? "true" : "false";
			result.push_back(record);
		}
		
		return result;
	}
} // namespace Monitor