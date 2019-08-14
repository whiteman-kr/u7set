#pragma once

#include "../VFrame30/SchemaItem.h"
#include "IdePropertyEditor.h"


namespace Ui {
	class SchemaItemPropertiesDialog;
}

namespace EditEngine
{
	class EditEngine;
}

class SchemaItemPropertyEditor;



class SchemaItemPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaItemPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget* parent = 0);
	virtual ~SchemaItemPropertiesDialog();

public:
	void setObjects(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items);
	void setReadOnly(bool value);

	void ensureVisible();

private:
    virtual void closeEvent(QCloseEvent * e);
    virtual void done(int r);

    void saveSettings();

private:
	Ui::SchemaItemPropertiesDialog *ui;
	SchemaItemPropertyEditor* m_propertyEditor = nullptr;
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;
};

//
//
//	SchemaItemPropertyBrowser
//
//

class SchemaItemPropertyEditor : public IdePropertyEditor
{
	Q_OBJECT

public:
	explicit SchemaItemPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent);
	virtual ~SchemaItemPropertyEditor();

protected slots:
	virtual void valueChanged(QString propertyName, QVariant value) override;

protected:
	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
};

