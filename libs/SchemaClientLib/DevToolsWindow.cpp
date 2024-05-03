#include <SchemaClientLib/DevToolsWindow.h>

#include "DevToolsWidget.h"

#include <QCloseEvent>
#include <QSettings>


namespace SchemaClientLib
{
	DevToolsWindow* DevToolsWindow::instance = nullptr;

	DevToolsWindow::DevToolsWindow(SchemaClientLib::IDevToolsAppSettings& settings,
								   VFrame30::IViewVariables& viewVariables,
								   SchemaClientLib::IDevToolsSchemaStats& schemaStats,
								   SchemaClientLib::IDevToolsScriptVariables& scriptVariables,
								   SchemaClientLib::IDevToolsGlobalScript& globalScript,
								   const std::list<std::pair<QString, QWidget*>>& additionalTabs,
								   QWidget* parent) :
		QMainWindow{parent}
	{
		// Disable minimize button
		//
		setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowContextHelpButtonHint);

		setWindowTitle(qAppName() + " DevTools");

		devToolsWidget = new DevToolsWidget{settings, viewVariables, schemaStats, scriptVariables, globalScript, this};
		setCentralWidget(devToolsWidget);

		for (auto& tab : additionalTabs)
		{
			devToolsWidget->addTab(tab.second, tab.first);
		}

		int restoreWidgetIndex = QSettings{}.value(qAppName() + "/DevToolsWindow/CurrentIndex", 0).toInt();

		devToolsWidget->setCurrentIndex(restoreWidgetIndex);

		return;
	}

	DevToolsWindow::~DevToolsWindow()
	{
		DevToolsWindow::instance = nullptr;
	}

	void DevToolsWindow::show(SchemaClientLib::IDevToolsAppSettings& settings,
							  VFrame30::IViewVariables& viewVariables,
							  SchemaClientLib::IDevToolsSchemaStats& schemaStats,
							  SchemaClientLib::IDevToolsScriptVariables& scriptVariables,
							  SchemaClientLib::IDevToolsGlobalScript& globalScript,
							  const std::list<std::pair<QString, QWidget*>>& additionalTabs,
							  QWidget* parent)
	{
		if (DevToolsWindow::instance == nullptr)
		{
			DevToolsWindow::instance = new DevToolsWindow(settings, viewVariables, schemaStats, scriptVariables, globalScript, additionalTabs, parent);
		}

		static_cast<QMainWindow*>(DevToolsWindow::instance)->show();
	}

	void DevToolsWindow::closeEvent(QCloseEvent* event)
	{
		QSettings{}.setValue(qAppName() + "/DevToolsWindow/CurrentIndex", devToolsWidget->currentIndex());

		deleteLater();
		event->accept();
	}
} // namespace SchemaClientLib
