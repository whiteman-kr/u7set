#include "SchemeItemPropertiesDialog.h"
#include "ui_SchemeItemPropertiesDialog.h"
#include "EditEngine/EditEngine.h"


SchemeItemPropertiesDialog::SchemeItemPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemeItemPropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemeItemPropertyEditor(editEngine, this);

	ui->horizontalLayout->addWidget(m_propertyEditor);


	return;
}

SchemeItemPropertiesDialog::~SchemeItemPropertiesDialog()
{
	delete ui;
}

void SchemeItemPropertiesDialog::setObjects(const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items)
{
	m_items = items;

	QList<std::shared_ptr<QObject>> ol;

	for (const auto& item : m_items)
	{
		ol.push_back(item);
	}

	m_propertyEditor->setObjects(ol);

	return;
}


//
//
//		SchemeItemPropertyBrowser
//
//
SchemeItemPropertyEditor::SchemeItemPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent) :
	PropertyEditor(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);
}

SchemeItemPropertyEditor::~SchemeItemPropertyEditor()
{
}

void SchemeItemPropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
	if (value.isValid() == false || property == nullptr)
	{
		assert(property != nullptr);
		return;
	}

	std::vector<std::shared_ptr<VFrame30::CVideoItem>> items;
	QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property->propertyName());

	for (auto& i : objects)
	{
		std::shared_ptr<VFrame30::CVideoItem> vi = std::dynamic_pointer_cast<VFrame30::CVideoItem>(i);
		assert(vi.get() != nullptr);

		items.push_back(vi);
	}

	editEngine()->runSetProperty(property->propertyName(), value, items);


	// Set the new property value in all objects
	//

	//update();

	//QList<QObject*> objects = m_propToClassMap.values(property->propertyName());

	/*QString errorString;
	QMetaProperty writeProperty;

	for (auto i = objects.begin(); i != objects.end(); i++)
	{
		QObject* pObject = *i;

		if (propertyByName(pObject, property->propertyName(), writeProperty) == false)
		{
			Q_ASSERT(false);
			continue;
		}

		if (writeProperty.type() == QVariant::Bool)
		{
			if (value == Qt::Unchecked)
				value = false;
			else
				value = true;
		}

		writeProperty.write(pObject, value);

		if (writeProperty.read(pObject) != value && errorString.isEmpty() == true)
		{
			errorString = QString("Property: %1 - incorrect input value")
						  .arg(property->propertyName());
		}
	}

	if (errorString.isEmpty() == false)
	{
	}*/

	return;
}

EditEngine::EditEngine* SchemeItemPropertyEditor::editEngine()
{
	return m_editEngine;
}
