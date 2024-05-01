#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace Monitor
{
	class DevToolsScriptVariables : public SchemaClientLib::IDevToolsScriptVariables
	{
	public:
		explicit DevToolsScriptVariables(QTabWidget* monitorCentralWidget);

		virtual std::vector<std::pair<QString, QVariant>> scriptVariables() const override;

	private:
		QTabWidget* m_monitorCentralWidget = nullptr;
	};
}