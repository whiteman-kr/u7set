#include "PropertyEditorWithUpdate.h"

PropertyEditorWithUpdate::PropertyEditorWithUpdate(QWidget* parent) :
	PropertyEditor(parent)
{
}

void PropertyEditorWithUpdate::valueChanged(QString propertyName, QVariant value)
{
	PropertyEditor::valueChanged(propertyName, value);
	emit valueUpdated();
}