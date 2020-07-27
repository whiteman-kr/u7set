#ifndef IDEPROPERTYEDITOR_H
#define IDEPROPERTYEDITOR_H

#include "../QScintilla/Qt4Qt5/Qsci/qsciscintilla.h"
#include "../lib/QScintillaLexers/LexerXML.h"
#include "../lib/QScintillaLexers/LexerJavaScript.h"

#include "../lib/PropertyEditor.h"
#include "../lib/PropertyTable.h"
#include "../lib/DbController.h"
#include "../lib/Types.h"
#include "../lib/Tuning/TuningFilterEditor.h"

//
// IdePropertyEditorHelper
//

class IdePropertyEditorHelper
{
public:
	static ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent, DbController* dbController);
};


//
// IdePropertyEditor
//

class IdePropertyEditor : public ExtWidgets::PropertyEditor
{
public:
	IdePropertyEditor(QWidget* parent, DbController* dbController = nullptr);
    virtual ~IdePropertyEditor();

	virtual ExtWidgets::PropertyEditor* createChildPropertyEditor(QWidget* parent) override;

	virtual ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent) override;

private:
	DbController* m_dbController = nullptr;
};

//
// IdePropertyTable
//

class IdePropertyTable : public ExtWidgets::PropertyTable
{
public:
	IdePropertyTable(QWidget* parent, DbController* dbController = nullptr);
	virtual ~IdePropertyTable();

	virtual ExtWidgets::PropertyEditor* createChildPropertyEditor(QWidget* parent) override;

	virtual ExtWidgets::PropertyTextEditor* createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent) override;

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
	void replaceAll(QString findText, QString replaceText, bool caseSensitive);

private slots:
    void onFind();
    void onReplace();
    void onReplaceAll();

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
};

class IdeQsciScintilla : public QsciScintilla
{
	Q_OBJECT
public:
	IdeQsciScintilla();
	virtual ~IdeQsciScintilla();

	void setCustomMenuActions(QList<QAction*> actions);

private:
	virtual void contextMenuEvent (QContextMenuEvent *e) override;

signals:
	void customContextMenuAboutToBeShown();

private:
	QList<QAction*> m_customMenuActions;

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

class IdeCodeEditor : public ExtWidgets::PropertyTextEditor
{
    Q_OBJECT
public:
    IdeCodeEditor(CodeType codeType, QWidget* parent);
    ~IdeCodeEditor();

	virtual QString text() const override;
	virtual void setText(const QString& text) override;

	int lines() const;
	void getCursorPosition(int* line, int* index) const;
	void setCursorPosition(int line, int index);

	virtual bool readOnly() const override;
	virtual void setReadOnly(bool value) override;

	void activateEditor();
	void setCustomMenuActions(QList<QAction*> actions);

public slots:
	void findFirst(QString findText, bool caseSensitive);
	void findNext();
	void replace(QString findText, QString replaceText, bool caseSensitive);
	void replaceAll(QString findText, QString replaceText, bool caseSensitive);

signals:
	void customContextMenuAboutToBeShown();
	void cursorPositionChanged(int line, int index);
	void textChanged();
	void saveKeyPressed();
	void closeKeyPressed();
	void ctrlTabKeyPressed();

private:
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
	void onCustomContextMenuAboutToBeShown();
	void onCursorPositionChanged(int line, int index);
	void onTextChanged();

private:
	void adjustMarginWidth();

private:
	IdeQsciScintilla* m_textEdit = nullptr;

	LexerJavaScript m_lexerJavaScript;
	LexerXML m_lexerXml;

    QWidget* m_parent = nullptr;

    DialogFindReplace* m_findReplace = nullptr;

	bool m_findFirst = true;

	static bool m_findCaseSensitive;
	static QString m_findText;

};

//
// IdeTuningFiltersEditor
//

class IdeTuningFiltersEditor : public ExtWidgets::PropertyTextEditor
{
public:

	explicit IdeTuningFiltersEditor(DbController* dbController, QWidget* parent);
    virtual ~IdeTuningFiltersEditor();

	QString text() const override;
	void setText(const QString& text) override;

	bool readOnly() const override;
    void setReadOnly(bool value) override;

private:
    TuningFilterEditor* m_tuningFilterEditor = nullptr;

	TuningSignalManager m_signals;
	TuningFilterStorage m_filters;

	DbController* m_dbController = nullptr;

};


#endif // IDEPROPERTYEDITOR_H

