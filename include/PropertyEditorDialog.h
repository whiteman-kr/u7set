#ifndef PROPERTYEDITORDIALOG_H
#define PROPERTYEDITORDIALOG_H

#include <QDialog>
#include "../include/PropertyObject.h"

class PropertyEditorDialog : public QDialog
{
public:
    PropertyEditorDialog(std::shared_ptr<PropertyObject> object, QWidget *parent);
    ~PropertyEditorDialog();


    int splitterPosition();
    void setSplitterPosition(int value);

protected:
    virtual bool onPropertiesChanged(std::shared_ptr<PropertyObject> object);

private slots:
    void onOk();

private:
    std::shared_ptr<PropertyObject> m_object;

    ExtWidgets::PropertyEditor* pe = nullptr;

};

#endif // PROPERTYEDITORDIALOG_H
