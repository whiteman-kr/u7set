#include "DevToolsViewVariables.h"
#include "../../VFrame30/ClientSchemaWidget.h"

namespace Monitor
{
	DevToolsViewVariables::DevToolsViewVariables(QTabWidget* monitorCentralWidget) :
		m_monitorCentralWidget(monitorCentralWidget)
	{
		Q_ASSERT(m_monitorCentralWidget);
	}

	QStringList Monitor::DevToolsViewVariables::viewVariables() const
	{
		QStringList result;

		auto clientWidget = dynamic_cast<const VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			result = clientWidget->clientSchemaView()->viewVariables();
		}

		return result;
	}

	bool DevToolsViewVariables::viewVariableExists(const QString& name) const
	{
		bool result = false;

		auto clientWidget = dynamic_cast<const VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			result = clientWidget->clientSchemaView()->viewVariableExists(name);
		}

		return result;
	}

	QVariant DevToolsViewVariables::viewVariable(const QString& name) const
	{
		QVariant result;

		auto clientWidget = dynamic_cast<const VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			result = clientWidget->clientSchemaView()->viewVariable(name);
		}

		return result;
	}

	void DevToolsViewVariables::setViewVariable(const QString& name, const QVariant& value)
	{
		auto clientWidget = dynamic_cast<VFrame30::ClientSchemaWidget*>(m_monitorCentralWidget->currentWidget());
		if (clientWidget != nullptr)
		{
			clientWidget->clientSchemaView()->setViewVariable(name, value);
		}

		return;
	}
} // namespace Monitor