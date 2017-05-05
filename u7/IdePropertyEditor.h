#ifndef IDEPROPERTYEDITOR_H
#define IDEPROPERTYEDITOR_H

#include <Qsci/qsciscintilla.h>

#include "../lib/PropertyEditor.h"

enum class CodeType
{
	Cpp,
	Xml,
	Unknown
};

//
// CodeEditor
//

class CodeEditor : public ExtWidgets::PropertyTextEditor
{
public:
	CodeEditor(CodeType codeType, QWidget* parent);

	virtual void setText(const QString& text);

	virtual QString text();

	virtual void setReadOnly(bool value);

private:
	QsciScintilla* m_textEdit = nullptr;

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
