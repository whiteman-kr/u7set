#include "DevToolsWidget.h"
#include "DevToolsAppSettings.h"
#include "DevToolsConnections.h"
#include "DevToolsGlobalScript.h"
#include "DevToolsScriptVariables.h"
#include "DevToolsViewVariables.h"
#include "DevToolsSchemaStats.h"

namespace SchemaClientLib
{
	DevToolsWidget::DevToolsWidget(IDevToolsAppSettings& settings,
								   VFrame30::IViewVariables& viewVariables,
								   SchemaClientLib::IDevToolsSchemaStats& schemaStats,
								   SchemaClientLib::IDevToolsScriptVariables& scriptVariables,
								   SchemaClientLib::IDevToolsGlobalScript& globalScript,
								   QWidget* parent) :
		QTabWidget{parent}
	{
		addTab(new DevToolsAppSettings{settings, this}, tr("App Settings"));
		addTab(new DevToolsConnections{this}, tr("Connections"));
		addTab(new DevToolsViewVariables{viewVariables, this}, tr("View Variables"));
		addTab(new DevToolsScriptVariables{scriptVariables, this}, tr("Script Variables"));
		addTab(new DevToolsGlobalScript{globalScript, this}, tr("Global Script"));
		addTab(new DevToolsSchemaStats{schemaStats, this}, tr("Schema Statistics"));
		return;
	}
} // namespace SchemaClientLib
