#include "DevToolsScriptVariables.h"

#include <VFrame30/ClientSchemaWidget.h>

namespace Monitor
{
	DevToolsScriptVariables::DevToolsScriptVariables(QTabWidget* monitorCentralWidget) :
		m_monitorCentralWidget(monitorCentralWidget)
	{
		Q_ASSERT(m_monitorCentralWidget);
	}

	std::vector<std::pair<QString, QVariant>> DevToolsScriptVariables::scriptVariables() const
	{
		std::vector<std::pair<QString, QVariant>> result;

		auto clientWidget = dynamic_cast<const VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			result = clientWidget->clientSchemaView()->scriptVariables();
		}

		return result;
	}
} // namespace Monitor