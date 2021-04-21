#pragma once

#include "../VFrame30/Schema.h"
#include "../lib/PropertyEditor.h"
#include "../DbLib/DbController.h"


namespace Ui {
	class SchemaPropertiesDialog;
}

namespace EditEngine
{
	class EditEngine;
}

class SchemaPropertyEditor;


class SchemaPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaPropertiesDialog(EditEngine::EditEngine* editEngine, DbController* dbController, QWidget* parent);
	virtual ~SchemaPropertiesDialog();

	void setSchema(std::shared_ptr<VFrame30::Schema> schema);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

private:
	Ui::SchemaPropertiesDialog *ui;

	SchemaPropertyEditor* m_propertyEditor = nullptr;
	std::shared_ptr<VFrame30::Schema> m_schema;
};

//
//
//	SchemaPropertyBrowser
//
//

class SchemaPropertyEditor : public ExtWidgets::PropertyEditor
{
	Q_OBJECT

public:
	explicit SchemaPropertyEditor(EditEngine::EditEngine* editEngine, DbController* dbController, QWidget* parent);
	virtual ~SchemaPropertyEditor();

protected slots:
	virtual void valueChanged(QString propertyName, QVariant value) override;

protected:
	virtual ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent) override;
	virtual bool restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;
	virtual bool storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;

	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
	DbController* m_dbController = nullptr;
};

