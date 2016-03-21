#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include "../include/PropertyEditor.h"
#include "../include/PropertyEditorDialog.h"

PropertyEditorDialog::PropertyEditorDialog(std::shared_ptr<PropertyObject> object, QWidget* parent)
    :QDialog(parent)
{
    m_object = object;

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PropertyEditorDialog::onOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PropertyEditorDialog::reject);

    pe = new ExtWidgets::PropertyEditor(this);

    QList<std::shared_ptr<PropertyObject>> objList;
    objList.push_back(object);
    pe->setObjects(objList);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pe);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

PropertyEditorDialog::~PropertyEditorDialog()
{

}

int PropertyEditorDialog::splitterPosition()
{
    if (pe != nullptr)
    {
        return pe->splitterPosition();
    }
}

void PropertyEditorDialog::setSplitterPosition(int value)
{
    if (pe != nullptr)
    {
        pe->setSplitterPosition(value);
    }
}


bool PropertyEditorDialog::onPropertiesChanged(std::shared_ptr<PropertyObject> object)
{
    Q_UNUSED(object);
    return true;
}

void PropertyEditorDialog::onOk()
{
    if (onPropertiesChanged(m_object) == true)
    {
        accept();
    }
}
