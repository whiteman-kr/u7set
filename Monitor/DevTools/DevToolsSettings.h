#pragma once
#include <SchemaClientLib/IDevTools.h>

#include "../MonitorConfigController.h"

namespace Monitor
{
	class DevToolsSettings : public SchemaClientLib::IDevToolsAppSettings
	{
		// IDevToolsAppSettings implementation
		//
	public:
		virtual QString appEquipmentId() const override;
		virtual std::vector<SchemaClientLib::IDevToolsAppSettings::Record> settings() const override;

	private:
		MonitorConfigSettings m_configuration;
	};
}