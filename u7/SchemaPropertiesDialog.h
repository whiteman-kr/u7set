#pragma once

#include <QDialog>
#include "../VFrame30/Schema.h"
#include "../lib/PropertyEditor.h"


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
	explicit SchemaPropertiesDialog(EditEngine::EditEngine* editEngine, QWidget *parent = 0);
	virtual ~SchemaPropertiesDialog();

public:
	void setSchema(std::shared_ptr<VFrame30::Schema> schema);

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
	explicit SchemaPropertyEditor(EditEngine::EditEngine* editEngine, QWidget* parent);
	virtual ~SchemaPropertyEditor();

protected slots:
	virtual void valueChanged(QtProperty* property, QVariant value) override;

protected:
	EditEngine::EditEngine* editEngine();

private:
	EditEngine::EditEngine* m_editEngine = nullptr;
};

