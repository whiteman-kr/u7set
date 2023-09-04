#pragma once
#include "IssueLogger.h"

namespace Builder
{
	struct ScriptChecker
	{
		static bool checkFile(const QString& script, const QString& fileName, IssueLogger& log);

		//static bool checkEquipmentProperty(const QString& script, const QString& equipmentId, const QString& property, IssueLogger& log);

		//static bool checkSchema(const QString& script, const QString& schemaId, const QString& property, IssueLogger& log);
		//static bool checkSchemaItem(const QString& script, const QString& schemaId, const QString& itemLabel, const QString& property, IssueLogger& log);
	};
} // namespace Builder
