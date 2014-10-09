#include "SchemeItemPropertiesDialog.h"
#include "ui_SchemeItemPropertiesDialog.h"



SchemeItemPropertiesDialog::SchemeItemPropertiesDialog(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SchemeItemPropertiesDialog)
{
	ui->setupUi(this);

	m_propertyEditor = new SchemeItemPropertyEditor(this);

	ui->horizontalLayout->addWidget(m_propertyEditor);
	//ui->horizontalLayout->addWidget(new QPushButton());

	//connect(m_propertyEditor, &PropertyEditor::propertiesChanged, this, &SchemeItemPropertiesDialog::propertiesChanged);

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
SchemeItemPropertyEditor::SchemeItemPropertyEditor(QWidget* parent) :
	PropertyEditor(parent)
{

}

SchemeItemPropertyEditor::~SchemeItemPropertyEditor()
{
}
