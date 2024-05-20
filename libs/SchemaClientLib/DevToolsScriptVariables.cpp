#include "DevToolsScriptVariables.h"
#include <VFrame30/SchemaItem.h>
#include <HardwareLib/ScriptDeviceObject.h>
#include <QDialogButtonBox>


namespace SchemaClientLib
{
	DevToolsScriptVariables::DevToolsScriptVariables(IDevToolsScriptVariables& provider, QWidget* parent) :
		QWidget{parent},
		m_provider{provider}
	{
		m_treeWidget = new QTreeWidget{this};
		m_treeWidget->setColumnCount(3);
		m_treeWidget->setHeaderLabels({tr("Variable"), tr("Type"), tr("Value")});
		connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &DevToolsScriptVariables::onItemDoubleClicked);

		auto layout = new QVBoxLayout{this};
		layout->addWidget(m_treeWidget);
		setLayout(layout);

		updateData();

		// Adjust the columns to the contents.
		//
		for (int i = 0; i < m_treeWidget->columnCount(); ++i)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		startTimer(125);

		return;
	}

	void DevToolsScriptVariables::timerEvent(QTimerEvent*)
	{
		updateData();
	}

	QString DevToolsScriptVariables::valueString(const QVariant& variant) const
	{
		// If it is a QObject* then return object pointer as a string.
		//
		if (variant.metaType().id() == QMetaType::QObjectStar)
		{
			if (auto object = variant.value<QObject*>();
				object != nullptr)
			{

				// Is this object is VFrame30::SchemaItem, then return it's label.
				//
				if (auto schemaItem = qobject_cast<VFrame30::SchemaItem*>(object);
					schemaItem != nullptr)
				{
					return schemaItem->label();
				}

				// if this object is Hardware::ScriptDeviceObject, then return it's name.
				//
				if (auto scriptDeviceObject = qobject_cast<Hardware::ScriptDeviceObject*>(object);
					scriptDeviceObject != nullptr)
				{
					return scriptDeviceObject->equipmentId();
				}

				return QString::asprintf("%p", object);
			}
			else
			{
				return QString::asprintf("%p", object);
			}
		}

		return variant.toString();
	}

	QString DevToolsScriptVariables::valueType(const QVariant& variant) const
	{
		QMetaType metaType = variant.metaType();

		// If type is QObject* then get the class name.
		//
		if (metaType.id() == QMetaType::QObjectStar)
		{
			auto object = variant.value<QObject*>();
			if (object != nullptr)
			{
				return object->metaObject()->className();
			}
		}

		// It it is a QJSValue then get the type name.
		//
		if (metaType.id() == qMetaTypeId<QJSValue>())
		{
			auto jsValue = variant.value<QJSValue>();
			QString typeName = jsValue.toString();
			if (typeName.startsWith("function"))
			{
				typeName = "function";
			}
			return typeName;
		}

		return variant.metaType().name();
	}

	void DevToolsScriptVariables::updateData()
	{
		Q_ASSERT(m_treeWidget);

		auto variables = m_provider.scriptVariables();
		std::stable_sort(variables.begin(), variables.end(), [](const auto& p1, const auto& p2) { return p1.first < p2.first; });

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

		bool same = true;
		if (std::ssize(variables) != existingTreeVariables.size())
		{
			same = false;
		}
		else
		{
			for (int i = 0; i < variables.size(); ++i)
			{
				if (variables[i].first != existingTreeVariables[i])
				{
					same = false;
					break;
				}
			}
		}

		if (same == true)
		{
			Q_ASSERT(m_treeWidget->topLevelItemCount() == variables.size());

			for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i)
			{
				auto item = m_treeWidget->topLevelItem(i);
				
				QVariant value = variables[i].second;
				item->setText(1, valueType(value));
				item->setText(2, valueString(value));
			}
		}
		else
		{
			m_treeWidget->clear();

			for (const auto& variable : variables)
			{
				auto item = new QTreeWidgetItem;

				item->setText(0, variable.first);

				QVariant value = variable.second;
				
				item->setText(1, valueType(value));
				item->setText(2, valueString(value));

				m_treeWidget->addTopLevelItem(item);
			}
		}

		return;
	}

    void DevToolsScriptVariables::onItemDoubleClicked(QTreeWidgetItem* item, [[maybe_unused]] int column)
    {
		// Get all three columns of the item.
		// Show dialog with read only text of all column text.
		//
		QString variable;
		QString type;
		QString value;

		if (item != nullptr)
		{
			variable = item->text(0);
			type = item->text(1);
			value = item->text(2);
		}

		QDialog dialog{this};
		dialog.setWindowTitle(tr("Script Variable"));
		dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

		dialog.setModal(true);

		auto layout = new QGridLayout{&dialog};

		auto variableLabel = new QLabel{tr("Variable:")};
		auto typeLabel = new QLabel{tr("Type:")};
		auto valueLabel = new QLabel{tr("Value:")};

		auto variableLineEdit = new QLineEdit{variable};
		variableLineEdit->setReadOnly(true);

		auto typeLineEdit = new QLineEdit{type};
		typeLineEdit->setReadOnly(true);

		auto valueLineEdit = new QLineEdit{value};
		valueLineEdit->setReadOnly(true);

		layout->addWidget(variableLabel, 0, 0);
		layout->addWidget(variableLineEdit, 0, 1);

		layout->addWidget(typeLabel, 1, 0);
		layout->addWidget(typeLineEdit, 1, 1);

		layout->addWidget(valueLabel, 2, 0);
		layout->addWidget(valueLineEdit, 2, 1);

		QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
		layout->addItem(spacer, 3, 0, 1, 2);

		auto buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok};
		layout->addWidget(buttonBox, 4, 0, 1, 2);

		connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

		layout->setColumnStretch(1, 2);

		dialog.setLayout(layout);
		dialog.resize(static_cast<int>(dialog.sizeHint().width() * 1.5), dialog.sizeHint().height());

		dialog.exec();

		return;
    }
} // namespace SchemaClientLib
