#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace SchemaClientLib
{
	class DevToolsSchemaStats : public QWidget
	{
		Q_OBJECT

	public:
		DevToolsSchemaStats(IDevToolsSchemaStats& provider, QWidget* parent);

	protected slots:
		void updateStats();
		void onTreeWidgetDoubleClicked(QTreeWidgetItem* item, int column);

	private:
		IDevToolsSchemaStats& m_provider;
		QString m_lastStats;

		QTreeWidget* m_treeWidget = nullptr;
		QPushButton* m_refreshButton = nullptr;
		QPushButton* m_copyButton = nullptr;
	};
} // namespace SchemaClientLib
