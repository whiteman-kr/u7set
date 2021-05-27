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
class SchemaItemPropertyTable;


class SchemaItemPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaItemPropertiesDialog(EditEngine::EditEngine* editEngine, DbController* db, QWidget* parent = 0);
	virtual ~SchemaItemPropertiesDialog();

public:
	const std::vector<SchemaItemPtr> objects() const;
	void setObjects(const std::vector<SchemaItemPtr>& items);

	void setReadOnly(bool value);

	void ensureVisible();

private:
	virtual void closeEvent(QCloseEvent * e);
	virtual void done(int r);

	void saveSettings();

private:
	Ui::SchemaItemPropertiesDialog *ui;

	SchemaItemPropertyEditor* m_propertyEditor = nullptr;
	SchemaItemPropertyTable* m_propertyTable = nullptr;

	std::vector<SchemaItemPtr> m_items;
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
	explicit SchemaItemPropertyEditor(EditEngine::EditEngine* editEngine, DbController* db, QWidget* parent);
	virtual ~SchemaItemPropertyEditor();

protected slots:
	virtual void valueChanged(QString propertyName, QVariant value) override;

protected:
	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
};


//
//
//	SchemaItemPropertyTable
//
//

class SchemaItemPropertyTable : public IdePropertyTable
{
	Q_OBJECT

public:
	explicit SchemaItemPropertyTable(EditEngine::EditEngine* editEngine, SchemaItemPropertiesDialog* schemaItemPropertiesDialog, QWidget* parent);
	virtual ~SchemaItemPropertyTable();

protected slots:
	virtual void valueChanged(const ExtWidgets::ModifiedObjectsData& modifiedObjectsData) override;

protected:
	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
	SchemaItemPropertiesDialog* m_schemaItemPropertiesDialog = nullptr;
};
