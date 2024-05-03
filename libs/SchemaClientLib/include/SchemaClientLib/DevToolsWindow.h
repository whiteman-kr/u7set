#pragma once
#include "../../../VFrame30/IViewVariables.h"

#include "IDevTools.h"
#include <QMainWindow>

namespace SchemaClientLib
{
	class DevToolsWidget;

	class DevToolsWindow : public QMainWindow
	{
		Q_OBJECT

	protected:
		DevToolsWindow(SchemaClientLib::IDevToolsAppSettings& settings,
					   VFrame30::IViewVariables& viewVariables,
					   SchemaClientLib::IDevToolsSchemaStats& schemaStats,
					   SchemaClientLib::IDevToolsScriptVariables& scriptVariables,
					   SchemaClientLib::IDevToolsGlobalScript& globalScript,
					   const std::list<std::pair<QString, QWidget*>>& additionalTabs,
					   QWidget* parent = nullptr);
		~DevToolsWindow();

		DevToolsWindow(const DevToolsWindow&) = delete;
		DevToolsWindow& operator=(const DevToolsWindow&) = delete;
		DevToolsWindow(DevToolsWindow&&) = delete;
		DevToolsWindow& operator=(DevToolsWindow&&) = delete;

	public:
		static void show(SchemaClientLib::IDevToolsAppSettings& settings,
						 VFrame30::IViewVariables& viewVariables,
						 SchemaClientLib::IDevToolsSchemaStats& schemaStats,
						 SchemaClientLib::IDevToolsScriptVariables& scriptVariables,
						 SchemaClientLib::IDevToolsGlobalScript& globalScript,
						 const std::list<std::pair<QString, QWidget*>>& additionalTabs,
						 QWidget* parent);

	protected:
		void closeEvent(QCloseEvent* event) override;

	private:
		static DevToolsWindow* instance;

		DevToolsWidget* devToolsWidget = nullptr;
	};
} // namespace SchemaClientLib