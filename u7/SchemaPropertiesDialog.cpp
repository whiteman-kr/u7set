#include "SchemaPropertiesDialog.h"
#include "ui_SchemaPropertiesDialog.h"
#include "EditEngine/EditEngine.h"
#include "Settings.h"


SchemaPropertiesDialog::SchemaPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::SchemaPropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemaPropertyEditor(editEngine, this);

	m_propertyEditor->setReadOnly(editEngine->readOnly());

	ui->horizontalLayout->addWidget(m_propertyEditor);

	if (QSize s = QSettings().value("SchemaPropertiesDialog/size").toSize();
		s.isValid() == true)
	{
		resize(s);
	}
	else
	{
		resize(sizeHint() * 2);
	}

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
	m_propertyEditor->autoAdjustSplitterPosition();

	return;
}

void SchemaPropertiesDialog::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);
	QSettings().setValue("SchemaPropertiesDialog/size", event->size());
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

void SchemaPropertyEditor::valueChanged(QString propertyName, QVariant value)
{
	if (value.isValid() == false)
	{
		assert(false);
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

	if (schema->propertyValue(propertyName) != value)
	{
		editEngine()->runSetSchemaProperty(propertyName, value, schema);
	}

	return;
}

EditEngine::EditEngine* SchemaPropertyEditor::editEngine()
{
	return m_editEngine;
}
