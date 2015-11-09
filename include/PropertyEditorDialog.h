#ifndef PROPERTYEDITORDIALOG_H
#define PROPERTYEDITORDIALOG_H

#include <QDialog>
#include "../include/PropertyObject.h"

class PropertyEditorDialog : public QDialog
{
public:
    PropertyEditorDialog(std::shared_ptr<PropertyObject>, QWidget *parent);

};

#endif // PROPERTYEDITORDIALOG_H
