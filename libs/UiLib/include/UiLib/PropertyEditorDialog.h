#pragma once

// #include "../CommonLib/PropertyObject.h" - must be included via precompiled header

#include <memory>

#include <QDialog>
#include <QList>

class PropertyObject;
class QDialogButtonBox;
class QWidget;

namespace ExtWidgets
{
	class PropertyEditor;
}

class PropertyEditorDialog : public QDialog
{
public:
	PropertyEditorDialog(QWidget* parent);
	~PropertyEditorDialog();

	void setObjects(QList<std::shared_ptr<PropertyObject>> objects);
	void setObject(std::shared_ptr<PropertyObject> object);

	void setReadOnly(bool readOnly);

    int splitterPosition();
    void setSplitterPosition(int value);

protected:
    virtual bool onPropertiesChanged(std::shared_ptr<PropertyObject> object);

private slots:
    void onOk();

private:
	QList<std::shared_ptr<PropertyObject>> m_objects;

	bool m_readOnly = false;

	QDialogButtonBox* m_buttonBox = nullptr;
    ExtWidgets::PropertyEditor* pe = nullptr;
};