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

	ui->horizontalLayout->addWidget(m_propertyEditor);

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

	return;
}

void SchemaItemPropertiesDialog::setReadOnly(bool value)
{
	m_propertyEditor->setReadOnly(value);
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

void SchemaItemPropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
	if (value.isValid() == false || property == nullptr)
	{
		assert(property != nullptr);
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
		if (vi->propertyValue(property->propertyName()) == value)
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

	editEngine()->runSetProperty(property->propertyName(), value, items);

	return;
}

EditEngine::EditEngine* SchemaItemPropertyEditor::editEngine()
{
	return m_editEngine;
}
