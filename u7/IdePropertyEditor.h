#ifndef IDEPROPERTYEDITOR_H
#define IDEPROPERTYEDITOR_H

#include "../lib/PropertyEditor.h"

class IdePropertyEditor : public ExtWidgets::PropertyEditor
{
public:
	IdePropertyEditor(QWidget* parent);
	virtual ~IdePropertyEditor();

	virtual void saveSettings();
};

#endif // IDEPROPERTYEDITOR_H
