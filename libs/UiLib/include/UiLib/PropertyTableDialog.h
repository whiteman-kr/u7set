#pragma once

// #include <CommonLib/PropertyObject.h> - must be included via precompiled header

#include <memory>

#include <QDialog>
#include <QList>

class PropertyObject;
class QDialogButtonBox;
class QWidget;

namespace ExtWidgets
{
	class PropertyTable;
}

class PropertyTableDialog : public QDialog
{
public:
	PropertyTableDialog(QWidget* parent);
	~PropertyTableDialog();

	void setObjects(QList<std::shared_ptr<PropertyObject>> objects);
	void setObject(std::shared_ptr<PropertyObject> object);

	void setReadOnly(bool readOnly);

protected:
    virtual bool onPropertiesChanged(std::shared_ptr<PropertyObject> object);

private slots:
    void onOk();

private:
	QList<std::shared_ptr<PropertyObject>> m_objects;

	bool m_readOnly = false;

	QDialogButtonBox* m_buttonBox = nullptr;
    ExtWidgets::PropertyTable* pt = nullptr;
};