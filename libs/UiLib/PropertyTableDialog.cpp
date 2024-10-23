#include <UiLib/PropertyTableDialog.h>
#include <UiLib/PropertyTable.h>


PropertyTableDialog::PropertyTableDialog(QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &PropertyTableDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &PropertyTableDialog::reject);

    pt = new ExtWidgets::PropertyTable(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pt);
	mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

	return;
}

PropertyTableDialog::~PropertyTableDialog() = default;

void PropertyTableDialog::setObjects(QList<std::shared_ptr<PropertyObject>> objects)
{
	m_objects = objects;
	pt->setObjects(m_objects);

	return;
}

void PropertyTableDialog::setObject(std::shared_ptr<PropertyObject> object)
{
	m_objects.clear();
	m_objects.push_back(object);

	pt->setObjects(m_objects);

	return;
}

void PropertyTableDialog::setReadOnly(bool readOnly)
{
	m_readOnly = readOnly;

	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_readOnly == false);

	pt->setReadOnly(readOnly);
}

bool PropertyTableDialog::onPropertiesChanged(std::shared_ptr<PropertyObject> object)
{
    Q_UNUSED(object);
    return true;
}

void PropertyTableDialog::onOk()
{
	if (pt->isReadOnly() == true)
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
