#include "DevToolsGlobalScript.h"
#include "../../VFrame30/ClientSchemaWidget.h"

namespace Monitor
{
	DevToolsGlobalScript::DevToolsGlobalScript(QTabWidget* monitorCentralWidget) :
		m_monitorCentralWidget(monitorCentralWidget)
	{
		Q_ASSERT(m_monitorCentralWidget);
	}

	QString DevToolsGlobalScript::globalScript() const
	{
		QString result;

		auto clientWidget = dynamic_cast<const VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			result = clientWidget->clientSchemaView()->globalScript();
		}

		return result;
	}

	void DevToolsGlobalScript::setGlobalScript(const QString& script)
	{
		auto clientWidget = dynamic_cast<VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			clientWidget->clientSchemaView()->setGlobalScript(script);
		}
	}
} // namespace Monitor