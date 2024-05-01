#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace Monitor
{
	class DevToolsGlobalScript : public SchemaClientLib::IDevToolsGlobalScript
	{
	public:
		explicit DevToolsGlobalScript(QTabWidget* monitorCentralWidget);

		virtual QString globalScript() const override;
		virtual void setGlobalScript(const QString& script) override;

	private:
		QTabWidget* m_monitorCentralWidget = nullptr;
	};
} // namespace Monitor