#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include "../include/PropertyEditor.h"
#include "../include/PropertyEditorDialog.h"

PropertyEditorDialog::PropertyEditorDialog(std::shared_ptr<PropertyObject> object, QWidget* parent)
    :QDialog(parent)
{
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PropertyEditorDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PropertyEditorDialog::reject);

    ExtWidgets::PropertyEditor* pe = new ExtWidgets::PropertyEditor(this);

    QList<std::shared_ptr<PropertyObject>> objList;
    objList.push_back(object);
    pe->setObjects(objList);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pe);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}
