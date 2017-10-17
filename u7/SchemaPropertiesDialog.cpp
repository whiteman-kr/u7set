#include "SchemaPropertiesDialog.h"
#include "ui_SchemaPropertiesDialog.h"
#include "EditEngine/EditEngine.h"


SchemaPropertiesDialog::SchemaPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemaPropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemaPropertyEditor(editEngine, this);
    m_propertyEditor->setResizeMode(ExtWidgets::PropertyEditor::ResizeToContents);

	ui->horizontalLayout->addWidget(m_propertyEditor);
	return;
}

SchemaPropertiesDialog::~SchemaPropertiesDialog()
{
	delete ui;
}

void SchemaPropertiesDialog::setSchema(std::shared_ptr<VFrame30::Schema> schema)
{
	m_schema = schema;

	QList<std::shared_ptr<PropertyObject>> ol;
	ol.push_back(schema);

	m_propertyEditor->setObjects(ol);

	return;
}

//
//
//		SchemaPropertyBrowser
//
//
SchemaPropertyEditor::SchemaPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent) :
	PropertyEditor(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);
}

SchemaPropertyEditor::~SchemaPropertyEditor()
{
}

void SchemaPropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
	if (value.isValid() == false || property == nullptr)
	{
		assert(property != nullptr);
		return;
	}

	QList<std::shared_ptr<PropertyObject>> objectsList = objects();

	if (objectsList.size() != 1)
	{
		assert(objectsList.size() == 1);
		return;
	}

	std::shared_ptr<VFrame30::Schema> schema = std::dynamic_pointer_cast<VFrame30::Schema>(objectsList.front());
	assert(schema.get() != nullptr);

	if (schema->propertyValue(property->propertyName()) != value)
	{
		editEngine()->runSetSchemaProperty(property->propertyName(), value, schema);
	}

	return;
}

EditEngine::EditEngine* SchemaPropertyEditor::editEngine()
{
	return m_editEngine;
}
