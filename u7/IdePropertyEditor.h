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
	IdePropertyTable(QWidget* parent, DbController* dbController = nullptr);
	virtual ~IdePropertyTable();

	virtual ExtWidgets::PropertyEditor* createChildPropertyEditor(QWidget* parent) override;
	virtual ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent) override;

	virtual bool restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;
	virtual bool storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog) override;

private:
	DbController* m_dbController = nullptr;
};


//
// DialogFindReplace
//
class DialogFindReplace : public QDialog
{
    Q_OBJECT
public:
    DialogFindReplace(QWidget* parent);
    ~DialogFindReplace();

signals:
    void findFirst(QString findText, bool caseSensitive);
    void replace(QString findText, QString text, bool caseSensitive);
    void replaceAll(QString findText, QString replaceText, bool selectedOnly, bool caseSensitive);

    void hasSelectedText(bool* result);	// Use Qt::DirectConnection for this

private slots:
    void onFind();
    void onReplace();
    void onReplaceAllButton();
    void onReplaceAll(bool selectedOnly);

private:
    void saveCompleters();

private:
    QLineEdit* m_findEdit = nullptr;
    QLineEdit* m_replaceEdit = nullptr;

    QPushButton* m_findButton = nullptr;
    QPushButton* m_replaceButton = nullptr;
    QPushButton* m_replaceAllButton = nullptr;

    QCompleter* m_findCompleter = nullptr;
    QCompleter* m_replaceCompleter = nullptr;

    QCheckBox* m_caseSensitiveCheck = nullptr;
    static bool m_caseSensitive;

    QMenu m_replaceMenu;
    QAction* m_replaceSelectedAction = nullptr;
    QAction* m_replaceAllAction = nullptr;
};

//
// IdeCodeEditor
//
enum class CodeType
{
    JavaScript,
    Xml,
    Unknown
};

class IdeCodeEditor : public UiLib::CodeEditor
{
    Q_OBJECT

public:
    IdeCodeEditor(CodeType codeType, QWidget* parent);
    ~IdeCodeEditor();

public slots:
    void onFind(QString findText, bool caseSensitive);
    void onReplace(QString findText, QString replaceText, bool caseSensitive);
    void onReplaceAll(QString findText, QString replaceText, bool selectedOnly, bool caseSensitive);
    void onHasSelectedText(bool* result);

signals:
    void cursorPositionChangedTo(int line, int index);
    void saveKeyPressed();
    void closeKeyPressed();
    void ctrlTabKeyPressed();
    void escapePressed();

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onCursorPositionChanged();

private:
    QWidget* m_parent = nullptr;

    CodeType m_codeType = CodeType::Unknown;

    DialogFindReplace* m_findReplace = nullptr;

    bool m_findCaseSensitive = false;
    QString m_findText;
};

//
// IdeCodePropertyEditor
//
class IdeCodePropertyEditor : public ExtWidgets::PropertyTextEditor
{
public:
    IdeCodePropertyEditor(CodeType codeType, QWidget* parent);
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
    IdeCodeEditor* m_textEdit = nullptr;

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

