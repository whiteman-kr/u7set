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

	m_propertyTable = new SchemaItemPropertyTable(editEngine, this);
	m_propertyTable->setReadOnly(editEngine->readOnly());

	QTabWidget* tabWidget = new QTabWidget();
	tabWidget->addTab(m_propertyEditor, "List view");
	tabWidget->addTab(m_propertyTable, "Table view");

	tabWidget->setTabPosition(QTabWidget::South);

	connect(tabWidget, &QTabWidget::currentChanged, this, &SchemaItemPropertiesDialog::propertiesModeTabChanged);

	ui->horizontalLayout->addWidget(tabWidget);

	setWindowTitle(tr("Schema Item(s) Properties"));

	// --
	//
	QSettings settings;

	int splitterValue = settings.value("SchemaItemPropertiesDialog/Splitter").toInt();
	if (splitterValue < 100)
	{
		splitterValue = 100;
	}

	m_propertyEditor->setSplitterPosition(splitterValue);

	restoreGeometry(settings.value("SchemaItemPropertiesDialog/Geometry").toByteArray());

	ensureVisible();

	return;
}

SchemaItemPropertiesDialog::~SchemaItemPropertiesDialog()
{
	delete ui;
}

void SchemaItemPropertiesDialog::setObjects(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
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
	QSettings settings;

	settings.setValue("SchemaItemPropertiesDialog/Splitter", m_propertyEditor->splitterPosition());
	settings.setValue("SchemaItemPropertiesDialog/Geometry", saveGeometry());

	return;
}

//
void SchemaItemPropertiesDialog::propertiesModeTabChanged(int index)
{
	if (m_propertyEditor == nullptr)
	{
		Q_ASSERT(m_propertyEditor);
		return;
	}

	if (m_propertyTable == nullptr)
	{
		Q_ASSERT(m_propertyTable);
		return;
	}

	if (index == 0)
	{
		m_propertyEditor->updatePropertyValues(QString());

		m_propertyTable->closeCurrentEditor();
	}
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
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
	QList<std::shared_ptr<PropertyObject>> objectsList = objects();

	for (auto i : objectsList)
	{
		std::shared_ptr<VFrame30::SchemaItem> vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(i);
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
		std::shared_ptr<VFrame30::SchemaItem> vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(i);
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
SchemaItemPropertyTable::SchemaItemPropertyTable(EditEngine::EditEngine* editEngine, QWidget* parent) :
	IdePropertyTable(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);

	connect(m_editEngine, &EditEngine::EditEngine::propertiesChanged, this, &SchemaItemPropertyTable::updatePropertiesValues);
}

SchemaItemPropertyTable::~SchemaItemPropertyTable()
{
}

void SchemaItemPropertyTable::valueChanged(QMap<QString, std::shared_ptr<PropertyObject>> modifiedObjectsData, const QVariant& value)
{
	if (value.isValid() == false)
	{
		assert(false);
		return;
	}

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;

	for (const QString& propertyName : modifiedObjectsData.keys())
	{
		QList<std::shared_ptr<PropertyObject>> objects = modifiedObjectsData.values(propertyName);

		items.clear();

		for (std::shared_ptr<PropertyObject> object : objects)
		{

			std::shared_ptr<VFrame30::SchemaItem> vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(object);
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
			continue;
		}

		editEngine()->runSetProperty(propertyName, value, items);
	}

	return;
}

EditEngine::EditEngine* SchemaItemPropertyTable::editEngine()
{
	return m_editEngine;
}
