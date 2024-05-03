#include "DevToolsViewVariables.h"
#include <QInputDialog>

namespace SchemaClientLib
{
	DevToolsViewVariables::DevToolsViewVariables(VFrame30::IViewVariables& viewVariables, QWidget* parent) :
		QWidget{parent},
		m_viewVariables{viewVariables}
	{
		m_treeWidget = new QTreeWidget{this};
		m_treeWidget->setColumnCount(3);
		m_treeWidget->setHeaderLabels({tr("Variable"), tr("Type"), tr("Value")});
		connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &DevToolsViewVariables::itemDoubleClicked);

		auto layout = new QVBoxLayout{this};
		layout->addWidget(m_treeWidget);
		setLayout(layout);

		startTimer(125);

		return;
	}

	void DevToolsViewVariables::timerEvent(QTimerEvent*)
	{
		updateData();
	}

	void DevToolsViewVariables::itemDoubleClicked(QTreeWidgetItem* item, [[maybe_unused]] int column)
	{
		Q_ASSERT(item);

		QString variable = item->text(0);
		QString value = item->text(2); // Value is in the third column.

		bool ok = false;
		QString enteredValue = QInputDialog::getText(this, tr("Set Variable Value"), QString{"Variable: %1"}.arg(variable), QLineEdit::Normal, value, &ok);

		if (ok == true && enteredValue != value)
		{
			m_viewVariables.setViewVariable(variable, enteredValue);
			updateData();
		}

		return;
	}

	QString DevToolsViewVariables::valueString(const QVariant& variant) const
	{
		return variant.toString();
	}

	QString DevToolsViewVariables::valueType(const QVariant& variant) const
	{
		return variant.metaType().name();
	}

	void DevToolsViewVariables::updateData()
	{
		Q_ASSERT(m_treeWidget);

		auto variables = m_viewVariables.viewVariables();
		std::stable_sort(variables.begin(), variables.end());

		// Get the first column of m_treeWidget,
		// compare it with the view variables,
		// if it is the same then update values,
		// if not then fill the tree widget with the new variables.
		//
		QStringList existingTreeVariables;
		existingTreeVariables.reserve(m_treeWidget->topLevelItemCount());

		for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i)
		{
			auto item = m_treeWidget->topLevelItem(i);
			existingTreeVariables.push_back(item->text(0));
		}

		if (variables == existingTreeVariables)
		{
			for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i)
			{
				auto item = m_treeWidget->topLevelItem(i);
				auto value = m_viewVariables.viewVariable(item->text(0));
				item->setText(2, valueString(value));
			}
		}
		else
		{
			m_treeWidget->clear();

			for (const auto& variable : variables)
			{
				auto item = new QTreeWidgetItem;

				QVariant value = m_viewVariables.viewVariable(variable);

				item->setText(0, variable);
				item->setText(1, valueType(value));
				item->setText(2, valueString(value));

				m_treeWidget->addTopLevelItem(item);
			}
		}

		return;
	}
} // namespace SchemaClientLib
