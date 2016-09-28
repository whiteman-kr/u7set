#ifndef DIALOGPRESETPROPERTIES_H
#define DIALOGPRESETPROPERTIES_H

#include <QDialog>

#include "../lib/PropertyEditorDialog.h"


class DialogPresetProperties : public PropertyEditorDialog
{
public:
	DialogPresetProperties(std::shared_ptr<PropertyObject> object, QWidget *parent);
	~DialogPresetProperties();

private:
	virtual bool onPropertiesChanged(std::shared_ptr<PropertyObject> object);
	virtual void closeEvent(QCloseEvent * e);
	virtual void done(int r);

	void saveSettings();


};

#endif // DIALOGPRESETPROPERTIES_H
