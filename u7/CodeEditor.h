#ifndef IDEPROPERTYEDITOR_H
#define IDEPROPERTYEDITOR_H

#include "../QScintilla/Qt4Qt5/Qsci/qsciscintilla.h"

#include "../lib/PropertyEditor.h"

enum class CodeType
{
	Cpp,
	Xml,
	Unknown
};

//
// DialogFindReplace
//
class DialogFindReplace : public QDialog
{
	Q_OBJECT
public:
	DialogFindReplace(QWidget* parent);

signals:
	void findFirst(QString findText);
	void replace(QString findText, QString text);
	void replaceAll(QString findText, QString replaceText);

private slots:
	void onFind();
	void onReplace();
	void onReplaceAll();

private:
	QLineEdit* m_findEdit = nullptr;
	QLineEdit* m_replaceEdit = nullptr;

	QPushButton* m_findButton = nullptr;
	QPushButton* m_replaceButton = nullptr;
	QPushButton* m_replaceAllButton = nullptr;
};

//
// CodeEditor
//

class CodeEditor : public ExtWidgets::PropertyTextEditor
{
	Q_OBJECT
public:
	CodeEditor(CodeType codeType, QWidget* parent);
	~CodeEditor();

	virtual void setText(const QString& text);

	virtual QString text();

	virtual void setReadOnly(bool value);

public slots:
	void findFirst(QString findText);
	void findNext();
	void replace(QString findText, QString replaceText);
	void replaceAll(QString findText, QString replaceText);

protected:
	bool eventFilter(QObject* obj, QEvent* event);

private:
	QsciScintilla* m_textEdit = nullptr;
	QsciLexer* m_lexer = nullptr;

	QWidget* m_parent = nullptr;

	DialogFindReplace* m_findReplace = nullptr;

	QString m_findText;

};

//
// IdePropertyEditor
//

class IdePropertyEditor : public ExtWidgets::PropertyEditor
{
public:
	IdePropertyEditor(QWidget* parent);
	virtual ~IdePropertyEditor();

	virtual void saveSettings();

	virtual ExtWidgets::PropertyTextEditor* createCodeEditor(bool script, QWidget* parent);
};

#endif // IDEPROPERTYEDITOR_H
