#pragma once

namespace VFrame30
{
	class Schema;
}

namespace Builder
{
	class IssueLogger;

	struct ScriptChecker
	{
		static bool checkFile(const QString& script, const QString& fileName, IssueLogger& log);
		static bool checkEquipmentProperty(const QString& script, const QString& equipmentId, const QString& property, IssueLogger& log);
		
		static bool checkSchema(const VFrame30::Schema* schema, IssueLogger& log);
	};
} // namespace Builder
