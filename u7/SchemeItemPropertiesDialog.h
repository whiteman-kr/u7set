#ifndef SCHEMEITEMPROPERTIESDIALOG_H
#define SCHEMEITEMPROPERTIESDIALOG_H

#include <QDialog>
#include <memory>
#include "../VFrame30/SchemeItem.h"
#include "../include/PropertyEditor.h"


namespace Ui {
	class SchemeItemPropertiesDialog;
}

namespace EditEngine
{
	class EditEngine;
}

class SchemeItemPropertyEditor;



class SchemeItemPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemeItemPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget *parent = 0);
	virtual ~SchemeItemPropertiesDialog();

public:
	void setObjects(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items);

private:
    virtual void closeEvent(QCloseEvent * e);
    virtual void done(int r);

    void saveSettings();

private:

    Ui::SchemeItemPropertiesDialog *ui;

	SchemeItemPropertyEditor* m_propertyEditor = nullptr;

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;
};

//
//
//	SchemeItemPropertyBrowser
//
//

class SchemeItemPropertyEditor : public ExtWidgets::PropertyEditor
{
	Q_OBJECT

public:
	explicit SchemeItemPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent);
	virtual ~SchemeItemPropertyEditor();

protected slots:
	virtual void valueChanged(QtProperty* property, QVariant value) override;

protected:
	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
};

#endif // SCHEMEITEMPROPERTIESDIALOG_H
