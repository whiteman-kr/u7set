#include "SchemaItemPropertiesDialog.h"
#include "ui_SchemaItemPropertiesDialog.h"
#include "EditEngine/EditEngine.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "Settings.h"


SchemaItemPropertiesDialog::SchemaItemPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint),
	ui(new Ui::SchemaItemPropertiesDialog)
{
	ui->setupUi(this);


	m_propertyEditor = new SchemaItemPropertyEditor(editEngine, this);
	m_propertyEditor->setReadOnly(editEngine->readOnly());

	m_propertyTable = new SchemaItemPropertyTable(editEngine, this, this);
	m_propertyTable->setReadOnly(editEngine->readOnly());

	QTabWidget* tabWidget = new QTabWidget();
	tabWidget->addTab(m_propertyEditor, "Tree view");
	tabWidget->addTab(m_propertyTable, "Table view");
	tabWidget->setTabPosition(QTabWidget::South);

	ui->horizontalLayout->addWidget(tabWidget);

	setWindowTitle(tr("Schema Item(s) Properties"));

	// --
	//
	m_propertyEditor->setSplitterPosition(theSettings.m_schemaItemPropertiesSplitterPosition);
	m_propertyTable->setPropertyMask(theSettings.m_schemaItemPropertiesPropertyMask);
	m_propertyTable->setExpandValuesToAllRows(theSettings.m_schemaItemPropertiesExpandValuesToAllRows);
	m_propertyTable->setColumnsWidth(theSettings.m_schemaItemPropertiesColumnsWidth);
	m_propertyTable->setGroupByCategory(theSettings.m_schemaItemPropertiesGroupByCategory);
	restoreGeometry(theSettings.m_schemaItemPropertiesGeometry);

	ensureVisible();

	return;
}

SchemaItemPropertiesDialog::~SchemaItemPropertiesDialog()
{
	delete ui;
}

const std::vector<SchemaItemPtr> SchemaItemPropertiesDialog::objects() const
{
	return m_items;
}

void SchemaItemPropertiesDialog::setObjects(const std::vector<SchemaItemPtr>& items)
{
	m_items = items;

	QList<std::shared_ptr<PropertyObject>> ol;

	for (const auto& item : m_items)
	{
		ol.push_back(item);
	}

	m_propertyEditor->setObjects(ol);
	m_propertyTable->setObjects(ol);

	return;
}

void SchemaItemPropertiesDialog::setReadOnly(bool value)
{
	m_propertyEditor->setReadOnly(value);
	m_propertyTable->setReadOnly(value);
}

void SchemaItemPropertiesDialog::ensureVisible()
{
	if (QScreen* screen = QGuiApplication::screenAt(geometry().center());
		screen == nullptr)
	{
		QScreen* newScreen = QGuiApplication::screens().at(0);

		if (QScreen* parentScreen = parentWidget()->window()->windowHandle()->screen();
			parentScreen != nullptr)
		{
			newScreen = parentScreen;
		}

		QRect screenGeometry = newScreen->geometry();

		move(screenGeometry.left() + screenGeometry.width() / 2 - width() / 2,
			 screenGeometry.top() + screenGeometry.height() / 2 - height() / 2);
	}
	else
	{
	}

	return;
}

void SchemaItemPropertiesDialog::closeEvent(QCloseEvent*)
{
	saveSettings();
}

void SchemaItemPropertiesDialog::done(int r)
{
	saveSettings();
	QDialog::done(r);
}

void SchemaItemPropertiesDialog::saveSettings()
{
	theSettings.m_schemaItemPropertiesSplitterPosition = m_propertyEditor->splitterPosition();
	theSettings.m_schemaItemPropertiesPropertyMask = m_propertyTable->propertyMask();
	theSettings.m_schemaItemPropertiesExpandValuesToAllRows = m_propertyTable->expandValuesToAllRows();
	theSettings.m_schemaItemPropertiesColumnsWidth = m_propertyTable->getColumnsWidth();
	theSettings.m_schemaItemPropertiesGroupByCategory = m_propertyTable->groupByCategory();
	theSettings.m_schemaItemPropertiesGeometry = saveGeometry();

	return;
}

//
//		SchemaItemPropertyBrowser
//
//
SchemaItemPropertyEditor::SchemaItemPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent) :
	IdePropertyEditor(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);

	connect(m_editEngine, &EditEngine::EditEngine::propertiesChanged, this, &SchemaItemPropertyEditor::updatePropertiesValues);
}

SchemaItemPropertyEditor::~SchemaItemPropertyEditor()
{
}

void SchemaItemPropertyEditor::valueChanged(QString propertyName, QVariant value)
{
	if (value.isValid() == false)
	{
		assert(false);
		return;
	}

	// Set the new property value in all objects
	//
	std::vector<SchemaItemPtr> items;
	QList<std::shared_ptr<PropertyObject>> objectsList = objects();

	for (auto i : objectsList)
	{
		SchemaItemPtr vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(i);
		assert(vi.get() != nullptr);

		// Do not set property, if it has the same value
		//
		if (vi->propertyValue(propertyName) == value)
		{
			continue;
		}

		items.push_back(vi);
	}

	if (items.empty() == true)
	{
		return;
	}

	//editEngine()->runSetProperty(property->propertyName(), value, items);	// Is two objects with the diff  BOOL values are selected,
																			// and then values changed, editEngine()->runSetProperty
																			// will select ONLY item with changed value, not good ((
																			// items changed to m_objects

	items.clear();
	for (auto i : objectsList)
	{
		SchemaItemPtr vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(i);
		items.push_back(vi);
	}

	editEngine()->runSetProperty(propertyName, value, items);

	return;
}

EditEngine::EditEngine* SchemaItemPropertyEditor::editEngine()
{
	return m_editEngine;
}


//
//		SchemaItemPropertyBrowser
//
//
SchemaItemPropertyTable::SchemaItemPropertyTable(EditEngine::EditEngine* editEngine, SchemaItemPropertiesDialog* schemaItemPropertiesDialog, QWidget* parent) :
	IdePropertyTable(parent),
	m_editEngine(editEngine),
	m_schemaItemPropertiesDialog(schemaItemPropertiesDialog)
{
	assert(m_editEngine);

	connect(m_editEngine, &EditEngine::EditEngine::propertiesChanged, this, &SchemaItemPropertyTable::updatePropertiesValues);
}

SchemaItemPropertyTable::~SchemaItemPropertyTable()
{
}

void SchemaItemPropertyTable::valueChanged(const ExtWidgets::ModifiedObjectsData& modifiedObjectsData)
{
	bool result = editEngine()->startBatch();
	if (result == false)
	{
		return;
	}

	// Select all items to keep selection even if properties of some items were not modified

	editEngine()->runNopItem(m_schemaItemPropertiesDialog->objects());

	// Modify properties

	for (const QString& propertyName : modifiedObjectsData.keys())
	{
		QList<std::pair<std::shared_ptr<PropertyObject>, QVariant>> objectsData = modifiedObjectsData.values(propertyName);

		for (auto objectData : objectsData)
		{
			std::vector<SchemaItemPtr> items;

			SchemaItemPtr vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(objectData.first);
			assert(vi.get() != nullptr);

			QVariant value = objectData.second;

			if (value.isValid() == false)
			{
				Q_ASSERT(false);
				continue;
			}

			// Do not set property, if it has the same value
			//
			if (vi->propertyValue(propertyName) == value)
			{
				continue;
			}

			items.push_back(vi);

			editEngine()->runSetProperty(propertyName, value, items);
		}
	}

	editEngine()->endBatch();

	return;
}

EditEngine::EditEngine* SchemaItemPropertyTable::editEngine()
{
	return m_editEngine;
}
