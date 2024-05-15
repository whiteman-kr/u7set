#pragma once

#include <VFrame30/IViewVariables.h>
#include <SchemaClientLib/IDevTools.h>

namespace SchemaClientLib
{
	class DevToolsWidget : public QTabWidget
	{
		Q_OBJECT

	public:
		DevToolsWidget(IDevToolsAppSettings& settings,
					   VFrame30::IViewVariables& viewVariables,
					   SchemaClientLib::IDevToolsSchemaStats& schemaStats,
					   SchemaClientLib::IDevToolsScriptVariables& scriptVariables,
					   SchemaClientLib::IDevToolsGlobalScript& globalScript,
					   QWidget* parent);
	};
} // namespace SchemaClientLib
