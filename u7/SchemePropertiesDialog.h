#ifndef SCHEMEPROPERTIESDIALOG_H
#define SCHEMEPROPERTIESDIALOG_H

#include <QDialog>
#include "../VFrame30/Scheme.h"
#include "../include/PropertyEditor.h"


namespace Ui {
	class SchemePropertiesDialog;
}

namespace EditEngine
{
	class EditEngine;
}

class SchemePropertyEditor;


class SchemePropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemePropertiesDialog(EditEngine::EditEngine* editEngine, QWidget *parent = 0);
	virtual ~SchemePropertiesDialog();

public:
	void setScheme(std::shared_ptr<VFrame30::Scheme> scheme);

private:
	Ui::SchemePropertiesDialog *ui;

	SchemePropertyEditor* m_propertyEditor = nullptr;
	std::shared_ptr<VFrame30::Scheme> m_scheme;
};

//
//
//	SchemePropertyBrowser
//
//

class SchemePropertyEditor : public PropertyEditor
{
	Q_OBJECT

public:
	explicit SchemePropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent);
	virtual ~SchemePropertyEditor();

protected slots:
	virtual void valueChanged(QtProperty* property, QVariant value) override;

protected:
	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
};

#endif // SCHEMEITEMPROPERTIESDIALOG_H
