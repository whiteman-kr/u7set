#include "SchemaItemPropertiesDialog.h"
#include "ui_SchemaItemPropertiesDialog.h"
#include "EditEngine/EditEngine.h"
#include "../VFrame30/SchemaItemSignal.h"


SchemaItemPropertiesDialog::SchemaItemPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemaItemPropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemaItemPropertyEditor(editEngine, this);

	m_propertyEditor->setSplitterPosition(theSettings.m_schemaItemSplitterState);
	if (theSettings.m_schemaItemPropertiesWindowPos.x() != -1 && theSettings.m_schemaItemPropertiesWindowPos.y() != -1)
    {
	   move(theSettings.m_schemaItemPropertiesWindowPos);
	   restoreGeometry(theSettings.m_schemaItemPropertiesWindowGeometry);
	}
    else
    {
        QRect scr = QApplication::desktop()->screenGeometry();
        move( scr.center() - rect().center() );
    }

	ui->horizontalLayout->addWidget(m_propertyEditor);

	setWindowTitle(tr("Schema Item(s) Properties"));

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

void SchemaItemPropertiesDialog::closeEvent(QCloseEvent * e)
{
    Q_UNUSED(e);
    saveSettings();

}

void SchemaItemPropertiesDialog::done(int r)
{
    saveSettings();
    QDialog::done(r);
}

void SchemaItemPropertiesDialog::saveSettings()
{
	theSettings.m_schemaItemSplitterState = m_propertyEditor->splitterPosition();
	theSettings.m_schemaItemPropertiesWindowPos = pos();
	theSettings.m_schemaItemPropertiesWindowGeometry = saveGeometry();

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
	//bool updateRequired = false;

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

		/*if (dynamic_cast<VFrame30::SchemaItemSignal*>(vi.get()) != nullptr &&
			property->propertyName() == "ColumnCount")
		{
			// If SchemaItemSignal::ColumnCount changed, new properties are created
			//
			updateRequired = true;
		}*/
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

	/*if (updateRequired == true)
	{
		setObjects(objectsList);	// Copy of m_objects, it's not a reference
	}*/

	return;
}

EditEngine::EditEngine* SchemaItemPropertyEditor::editEngine()
{
	return m_editEngine;
}
