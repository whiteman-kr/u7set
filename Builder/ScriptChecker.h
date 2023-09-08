#pragma once
#include "IssueLogger.h"

#include "../VFrame30/Schema.h"

namespace Builder
{
	struct ScriptChecker
	{
		static bool checkFile(const QString& script, const QString& fileName, IssueLogger& log);
		static bool checkEquipmentProperty(const QString& script, const QString& equipmentId, const QString& property, IssueLogger& log);
		
		static bool checkSchema(const VFrame30::Schema* schema, IssueLogger& log);
	};
} // namespace Builder
