#include "SchemeItemPropertiesDialog.h"
#include "ui_SchemeItemPropertiesDialog.h"
#include "EditEngine/EditEngine.h"


SchemeItemPropertiesDialog::SchemeItemPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemeItemPropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemeItemPropertyEditor(editEngine, this);

    m_propertyEditor->setSplitterPosition(theSettings.m_schemeItemSplitterState);
    if (theSettings.m_schemeItemPropertiesWindowPos.x() != -1 && theSettings.m_schemeItemPropertiesWindowPos.y() != -1)
    {
       move(theSettings.m_schemeItemPropertiesWindowPos);
       restoreGeometry(theSettings.m_schemeItemPropertiesWindowGeometry);
    }
    else
    {
        QRect scr = QApplication::desktop()->screenGeometry();
        move( scr.center() - rect().center() );
    }

	ui->horizontalLayout->addWidget(m_propertyEditor);

	return;
}

SchemeItemPropertiesDialog::~SchemeItemPropertiesDialog()
{
	delete ui;
}

void SchemeItemPropertiesDialog::setObjects(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
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

void SchemeItemPropertiesDialog::closeEvent(QCloseEvent * e)
{
    Q_UNUSED(e);
    saveSettings();

}

void SchemeItemPropertiesDialog::done(int r)
{
    saveSettings();
    QDialog::done(r);
}

void SchemeItemPropertiesDialog::saveSettings()
{
    theSettings.m_schemeItemSplitterState = m_propertyEditor->splitterPosition();
    theSettings.m_schemeItemPropertiesWindowPos = pos();
    theSettings.m_schemeItemPropertiesWindowGeometry = saveGeometry();
}


//
//
//		SchemeItemPropertyBrowser
//
//
SchemeItemPropertyEditor::SchemeItemPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent) :
	ExtWidgets::PropertyEditor(parent),
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

EditEngine::EditEngine* SchemeItemPropertyEditor::editEngine()
{
	return m_editEngine;
}
