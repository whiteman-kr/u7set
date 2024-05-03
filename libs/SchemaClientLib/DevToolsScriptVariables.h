#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace SchemaClientLib
{
	class DevToolsScriptVariables : public QWidget
	{
		Q_OBJECT

	public:
		DevToolsScriptVariables(IDevToolsScriptVariables& provider, QWidget* parent);

	protected:
		void timerEvent(QTimerEvent* event) override;

		QString valueString(const QVariant& variant) const;
		QString valueType(const QVariant& variant) const;

		void updateData();

	protected slots:
		void onItemDoubleClicked(QTreeWidgetItem* item, int column);

	private:
		IDevToolsScriptVariables& m_provider;

		QTreeWidget* m_treeWidget = nullptr;
	};
} // namespace SchemaClientLib
