#include "SchemaItemPropertiesDialog.h"
#include "ui_SchemaItemPropertiesDialog.h"
#include "EditEngine/EditEngine.h"


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
	ExtWidgets::PropertyEditor(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);

	connect(m_editEngine, &EditEngine::EditEngine::propertiesChanged, this, &SchemaItemPropertyEditor::updateProperties);
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

	for (auto i : m_objects)
	{
		std::shared_ptr<VFrame30::SchemaItem> vi = std::dynamic_pointer_cast<VFrame30::SchemaItem>(i);
		assert(vi.get() != nullptr);

		// Do not set property, if it has same value
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

	editEngine()->runSetProperty(property->propertyName(), value, items);
	return;
}

EditEngine::EditEngine* SchemaItemPropertyEditor::editEngine()
{
	return m_editEngine;
}