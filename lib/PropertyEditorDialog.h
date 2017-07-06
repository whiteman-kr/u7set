#ifndef PROPERTYEDITORDIALOG_H
#define PROPERTYEDITORDIALOG_H

#include <QDialog>
#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"

class PropertyEditorDialog : public QDialog
{
public:
	PropertyEditorDialog(QWidget *parent);
	~PropertyEditorDialog();

	void setObjects(QList<std::shared_ptr<PropertyObject> > objects);

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

    ExtWidgets::PropertyEditor* pe = nullptr;

};

#endif // PROPERTYEDITORDIALOG_H
