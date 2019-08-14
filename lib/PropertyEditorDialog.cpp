#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"

PropertyEditorDialog::PropertyEditorDialog(QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &PropertyEditorDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &PropertyEditorDialog::reject);

    pe = new ExtWidgets::PropertyEditor(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pe);
	mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

	return;
}

PropertyEditorDialog::~PropertyEditorDialog()
{
}

void PropertyEditorDialog::setObjects(QList<std::shared_ptr<PropertyObject>> objects)
{
	m_objects = objects;
	pe->setObjects(m_objects);

	return;
}

void PropertyEditorDialog::setObject(std::shared_ptr<PropertyObject> object)
{
	m_objects.clear();
	m_objects.push_back(object);

	pe->setObjects(m_objects);

	return;
}

void PropertyEditorDialog::setReadOnly(bool readOnly)
{
	m_readOnly = readOnly;

	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_readOnly == false);

	pe->setReadOnly(readOnly);
}

int PropertyEditorDialog::splitterPosition()
{
    if (pe != nullptr)
    {
        return pe->splitterPosition();
    }

    return 0;
}

void PropertyEditorDialog::setSplitterPosition(int value)
{
    if (pe != nullptr)
    {
        pe->setSplitterPosition(value);
    }

	return;
}

bool PropertyEditorDialog::onPropertiesChanged(std::shared_ptr<PropertyObject> object)
{
    Q_UNUSED(object);
    return true;
}

void PropertyEditorDialog::onOk()
{
	if (pe->isReadOnly() == true)
	{
		return;
	}

	bool result = true;

	for (std::shared_ptr<PropertyObject>& object : m_objects)
	{
		if (onPropertiesChanged(object) == false)
		{
			result = false;
		}
	}

	if (result == true)
	{
		accept();
	}

	return;
}
