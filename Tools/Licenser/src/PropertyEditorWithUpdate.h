#pragma once
#include <UiLib/PropertyEditor.h>

class PropertyEditorWithUpdate : public ExtWidgets::PropertyEditor
{
	Q_OBJECT

public:
	explicit PropertyEditorWithUpdate(QWidget* parent = nullptr);

protected:
	virtual void valueChanged(QString propertyName, QVariant value) override;

signals:
	void valueUpdated();
};