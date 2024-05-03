#pragma once
#include "../../VFrame30/IViewVariables.h"

namespace Monitor
{
	class DevToolsViewVariables : public VFrame30::IViewVariables
	{
	public:
		explicit DevToolsViewVariables(QTabWidget* monitorCentralWidget);

		virtual QStringList viewVariables() const override;
		virtual bool viewVariableExists(const QString& name) const override;

		virtual QVariant viewVariable(const QString& name) const override;
		virtual void setViewVariable(const QString& name, const QVariant& value) override;

	private:
		QTabWidget* m_monitorCentralWidget = nullptr;
	};
}