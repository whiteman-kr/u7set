#pragma once

#include "../../../VFrame30/IViewVariables.h"

namespace SchemaClientLib
{
	class DevToolsViewVariables : public QWidget
	{
		Q_OBJECT

	public:
		DevToolsViewVariables(VFrame30::IViewVariables& viewVariables, QWidget* parent);

	protected:
		void timerEvent(QTimerEvent* event) override;
		void itemDoubleClicked(QTreeWidgetItem* item, int column);

		QString valueString(const QVariant& variant) const;
		QString valueType(const QVariant& variant) const;

		void updateData();

	private:
		VFrame30::IViewVariables& m_viewVariables;

		QTreeWidget* m_treeWidget = nullptr;
	};
} // namespace SchemaClientLib
