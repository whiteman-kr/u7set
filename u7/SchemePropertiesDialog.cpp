#include "SchemePropertiesDialog.h"
#include "ui_SchemePropertiesDialog.h"
#include "EditEngine/EditEngine.h"


SchemePropertiesDialog::SchemePropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemePropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemePropertyEditor(editEngine, this);
    m_propertyEditor->setResizeMode(ExtWidgets::PropertyEditor::ResizeToContents);

	ui->horizontalLayout->addWidget(m_propertyEditor);
	return;
}

SchemePropertiesDialog::~SchemePropertiesDialog()
{
	delete ui;
}

void SchemePropertiesDialog::setScheme(std::shared_ptr<VFrame30::Schema> scheme)
{
	m_scheme = scheme;

	QList<std::shared_ptr<PropertyObject>> ol;
	ol.push_back(scheme);

	m_propertyEditor->setObjects(ol);

	return;
}

//
//
//		SchemePropertyBrowser
//
//
SchemePropertyEditor::SchemePropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent) :
	PropertyEditor(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);
}

SchemePropertyEditor::~SchemePropertyEditor()
{
}

void SchemePropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
	if (value.isValid() == false || property == nullptr)
	{
		assert(property != nullptr);
		return;
	}

	QList<std::shared_ptr<PropertyObject>> objects = m_objects;

	if (objects.size() != 1)
	{
		assert(objects.size() == 1);
		return;
	}

	std::shared_ptr<VFrame30::Schema> scheme = std::dynamic_pointer_cast<VFrame30::Schema>(objects.front());
	assert(scheme.get() != nullptr);

	if (scheme->propertyValue(property->propertyName()) != value)
	{
		editEngine()->runSetSchemeProperty(property->propertyName(), value, scheme);
	}

	return;
}

EditEngine::EditEngine* SchemePropertyEditor::editEngine()
{
	return m_editEngine;
}
