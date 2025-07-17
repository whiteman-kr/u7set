#pragma once

#include <UiLib/CodeEditor.h>
#include <UiLib/PropertyEditor.h>
#include <UiLib/PropertyTable.h>
#include "TuningUiEditor.h"
#include <TuningLib/TuningUiItem.h>


//
// IdePropertyEditorHelper
//
class IdePropertyEditorHelper
{
public:
	static ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QString defaultSpecPropCategory, DbController* dbController, QWidget* parent);

	static bool restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog);
	static bool storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog);
};


//
// IdePropertyEditor
//
class IdePropertyEditor : public ExtWidgets::PropertyEditor
{
	Q_OBJECT

public:
	IdePropertyEditor(QWidget* parent, DbController* dbController = nullptr);
    virtual ~IdePropertyEditor();

	virtual ExtWidgets::PropertyEditor* createChildPropertyEditor(QWidget* parent) override;
	virtual ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent) override;

	virtual bool restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;
	virtual bool storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;

private:
	DbController* m_dbController = nullptr;
};

//
// IdePropertyTable
//
class IdePropertyTable : public ExtWidgets::PropertyTable
{
	Q_OBJECT

public:
	IdePropertyTable(QWidget* parent, DbController* dbController);
	virtual ~IdePropertyTable();

	virtual ExtWidgets::PropertyEditor* createChildPropertyEditor(QWidget* parent) override;
	virtual ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent) override;

	virtual bool restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;
	virtual bool storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;

private:
	DbController* m_dbController = nullptr;
};

//
// IdeCodePropertyEditor
//
class IdeCodePropertyEditor : public ExtWidgets::PropertyTextEditor
{
public:
	IdeCodePropertyEditor(UiLib::CodeEditor::CodeType codeType, QWidget* parent);
    ~IdeCodePropertyEditor();

private:
    virtual QString text() const override;
    virtual void setText(const QString& text) override;

    bool readOnly() const override;
    void setReadOnly(bool value) override;

    bool externalOkCancelButtons() const override;

private:
	bool isModified() const override;

private:
    UiLib::CodeEditor* m_textEdit = nullptr;

};

//
// IdeTuningUiEditor
//
class IdeTuningUiEditor : public ExtWidgets::PropertyTextEditor
{
public:
	explicit IdeTuningUiEditor(QWidget* parent);
    virtual ~IdeTuningUiEditor();

	QString text() const override;
	void setText(const QString& text) override;

	bool readOnly() const override;
    void setReadOnly(bool value) override;

	bool externalOkCancelButtons() const override;

private:
	bool isModified() const override;

private:
    TuningUiEditor* m_tuningUiEditor = nullptr;
	TuningLib::TuningUiStorage m_storage;
};

