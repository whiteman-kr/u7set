#include "IdePropertyEditor.h"
#include "Settings.h"

#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerxml.h>

//
// CodeEditor
//

CodeEditor::CodeEditor(CodeType codeType, QWidget* parent) :
	ExtWidgets::PropertyTextEditor(parent)
{

	m_textEdit = new QsciScintilla();

	QHBoxLayout* l = new QHBoxLayout(this);
	l->addWidget(m_textEdit);

	if (codeType == CodeType::Cpp)
	{
		QsciLexerCPP* lexer = new QsciLexerCPP();
		m_textEdit->setLexer(lexer);
	}

	if (codeType == CodeType::Xml)
	{
		QsciLexerXML* lexer = new QsciLexerXML();
		m_textEdit->setLexer(lexer);
	}

	if (codeType == CodeType::Cpp || codeType == CodeType::Xml)
	{
		m_textEdit->setMarginType(0, QsciScintilla::NumberMargin);
		m_textEdit->setMarginWidth(0, 50);

		m_textEdit->setFolding(QsciScintilla::BoxedTreeFoldStyle);
	}
	else
	{
		m_textEdit->setMargins(0);
	}

	m_textEdit->setTabWidth(4);

	m_textEdit->setAutoIndent(true);

	connect(m_textEdit, &QsciScintilla::textChanged, this, &PropertyTextEditor::textChanged);

}

void CodeEditor::setText(const QString& text)
{
	m_textEdit->blockSignals(true);

	m_textEdit->setText(text);

	m_textEdit->blockSignals(false);
}

QString CodeEditor::text()
{
	return m_textEdit->text();
}

void CodeEditor::setReadOnly(bool value)
{
	m_textEdit->setReadOnly(value);
}


//
// IdePropertyEditor
//

IdePropertyEditor::IdePropertyEditor(QWidget* parent) :
	PropertyEditor(parent)
{
	// Set script help data
	//
	QFile file(":/ScriptHelp/scripthelp.html");

	if (file.open(QIODevice::ReadOnly) == true)
	{
		QByteArray data = file.readAll();
		if (data.size() > 0)
		{
			setScriptHelp(QString::fromUtf8(data));
		}
	}

	setScriptHelpWindowPos(theSettings.m_scriptHelpWindowPos);
	setScriptHelpWindowGeometry(theSettings.m_scriptHelpWindowGeometry);
}

IdePropertyEditor::~IdePropertyEditor()
{
}

void IdePropertyEditor::saveSettings()
{
	theSettings.m_scriptHelpWindowPos = scriptHelpWindowPos();
	theSettings.m_scriptHelpWindowGeometry = scriptHelpWindowGeometry();
}

ExtWidgets::PropertyTextEditor* IdePropertyEditor::createCodeEditor(bool script, QWidget* parent)
{
	CodeType codeType = script == true ? CodeType::Cpp : CodeType::Unknown;
	return new CodeEditor(codeType, parent);
}
