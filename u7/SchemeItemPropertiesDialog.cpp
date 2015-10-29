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

void SchemeItemPropertiesDialog::setObjects(const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items)
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
	ExtWidgetsOld::PropertyEditorOld(parent),
	m_editEngine(editEngine)
{
	assert(m_editEngine);

	connect(m_editEngine, &EditEngine::EditEngine::propertiesChanged, this, &SchemeItemPropertyEditor::updateProperties);
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

	std::vector<std::shared_ptr<VFrame30::SchemeItem>> items;
	QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property);

	for (auto& i : objects)
	{
		std::shared_ptr<VFrame30::SchemeItem> vi = std::dynamic_pointer_cast<VFrame30::SchemeItem>(i);
		assert(vi.get() != nullptr);

		items.push_back(vi);
	}

	editEngine()->runSetProperty(property->propertyName(), value, items);

	return;
}

EditEngine::EditEngine* SchemeItemPropertyEditor::editEngine()
{
	return m_editEngine;
}
