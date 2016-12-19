#ifndef DIALOGPROPERTIES_H
#define DIALOGPROPERTIES_H

#include <QDialog>

#include "../lib/PropertyEditorDialog.h"


class DialogProperties : public PropertyEditorDialog
{
    Q_OBJECT
public:
	DialogProperties(std::shared_ptr<PropertyObject> object, QWidget *parent);
	~DialogProperties();

private:
	virtual void closeEvent(QCloseEvent * e);
	virtual void done(int r);

	void saveSettings();


};

#endif // DIALOGPROPERTIES_H
