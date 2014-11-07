#include "SchemePropertiesDialog.h"
#include "ui_SchemePropertiesDialog.h"
#include "EditEngine/EditEngine.h"


SchemePropertiesDialog::SchemePropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemePropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemePropertyEditor(editEngine, this);

	ui->horizontalLayout->addWidget(m_propertyEditor);
	return;
}

SchemePropertiesDialog::~SchemePropertiesDialog()
{
	delete ui;
}

void SchemePropertiesDialog::setScheme(std::shared_ptr<VFrame30::Scheme> scheme)
{
	m_scheme = scheme;

	QList<std::shared_ptr<QObject>> ol;
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

	QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property);

	if (objects.size() != 1)
	{
		assert(objects.size() == 1);
		return;
	}

	std::shared_ptr<VFrame30::Scheme> scheme = std::dynamic_pointer_cast<VFrame30::Scheme>(objects.front());
	assert(scheme.get() != nullptr);

	editEngine()->runSetSchemeProperty(property->propertyName(), value, scheme);
	return;
}

EditEngine::EditEngine* SchemePropertyEditor::editEngine()
{
	return m_editEngine;
}
