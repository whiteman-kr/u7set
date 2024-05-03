#pragma once

#include "../../VFrame30/ITimeStats.h"

namespace SchemaClientLib
{
	class IDevToolsAppSettings
	{
	public:
		virtual ~IDevToolsAppSettings() = default;

		virtual QString appEquipmentId() const = 0;

		struct Record
		{
			QString section; // Connection, View, Other, etc
			QString key;     // Key name
			QString value;   // Value
		};

		virtual std::vector<Record> settings() const = 0;
	};

	class IDevToolsScriptVariables
	{
	public:
		virtual ~IDevToolsScriptVariables() = default;

		virtual std::vector<std::pair<QString, QVariant>> scriptVariables() const = 0;
	};

	class IDevToolsGlobalScript
	{
	public:
		virtual ~IDevToolsGlobalScript() = default;

		virtual QString globalScript() const = 0;
		virtual void setGlobalScript(const QString& script) = 0;
	};

	class IDevToolsSchemaStats : public VFrame30::ITimeStats
	{
	public:
		virtual ~IDevToolsSchemaStats() = default;

		virtual void highlightItems(QStringList items) = 0;
	};


} // namespace SchemaClientLib