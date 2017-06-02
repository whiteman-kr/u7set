#include "CodeEditor.h"
#include "Settings.h"

#include "../QScintilla/Qt4Qt5/Qsci/qscilexercpp.h"
#include "../QScintilla/Qt4Qt5/Qsci/qscilexerxml.h"

//
// DialogFindReplace
//

DialogFindReplace::DialogFindReplace(QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	QLabel* labelFind = new QLabel("Find What:");
	QLabel* labelReplace = new QLabel("Replace Width:");

	m_findEdit = new QLineEdit();
	m_replaceEdit = new QLineEdit();

	m_findButton = new QPushButton(tr("Find"));
	m_replaceButton = new QPushButton(tr("Replace"));
	m_replaceAllButton = new QPushButton(tr("Replace All"));

	connect(m_findButton, &QPushButton::clicked, this, &DialogFindReplace::onFind);
	connect(m_replaceButton, &QPushButton::clicked, this, &DialogFindReplace::onReplace);
	connect(m_replaceAllButton, &QPushButton::clicked, this, &DialogFindReplace::onReplaceAll);

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->addWidget(labelFind, 0, 0);
	gridLayout->addWidget(m_findEdit, 0, 1);

	gridLayout->addWidget(labelReplace, 1, 0);
	gridLayout->addWidget(m_replaceEdit, 1, 1);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(m_findButton);
	buttonsLayout->addWidget(m_replaceButton);
	buttonsLayout->addWidget(m_replaceAllButton);

	QVBoxLayout* vl = new QVBoxLayout();
	vl->addLayout(gridLayout);
	vl->addLayout(buttonsLayout);

	setLayout(vl);

	setMinimumWidth(400);
	setMinimumHeight(100);
}

void DialogFindReplace::onFind()
{
	QString text = m_findEdit->text();
	if (text.isEmpty() == true)
	{
		return;
	}

	emit findFirst(text);
}

void DialogFindReplace::onReplace()
{
	QString textFind = m_findEdit->text();
	if (textFind.isEmpty() == true)
	{
		return;
	}

	QString textReplace = m_replaceEdit->text();
	if (textReplace.isEmpty() == true)
	{
		return;
	}

	emit replace(textFind, textReplace);
}

void DialogFindReplace::onReplaceAll()
{
	QString textFind = m_findEdit->text();
	if (textFind.isEmpty() == true)
	{
		return;
	}

	QString textReplace = m_replaceEdit->text();
	if (textReplace.isEmpty() == true)
	{
		return;
	}

	emit replaceAll(textFind, textReplace);
}

//
// CodeEditor
//

CodeEditor::CodeEditor(CodeType codeType, QWidget* parent) :
	PropertyTextEditor(parent),
	m_parent(parent)
{
	m_textEdit = new QsciScintilla();

	m_textEdit->installEventFilter(this);

	QHBoxLayout* l = new QHBoxLayout(this);
	l->addWidget(m_textEdit);

	// Set up default font
	//
#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
		f.setPixelSize(font().pixelSize());
#elif defined(Q_OS_MAC)
		QFont f = QFont("Courier");
		f.setPixelSize(font().pixelSize());
#else
		QFont f = QFont("Courier");
		f.setPixelSize(font().pixelSize());
#endif

	// Set up lexer
	//
	QsciLexer* lexer = nullptr;

	if (codeType == CodeType::Cpp)
	{
		lexer = new QsciLexerCPP();
	}

	if (codeType == CodeType::Xml)
	{
		lexer = new QsciLexerXML();
	}

	if (lexer != nullptr)
	{
		lexer->setDefaultFont(f);
		m_textEdit->setFont(f);
		m_textEdit->setLexer(lexer);
	}

	// Set up margins

	if (codeType == CodeType::Cpp || codeType == CodeType::Xml)
	{
		m_textEdit->setMarginType(0, QsciScintilla::NumberMargin);
		m_textEdit->setMarginWidth(0, 40);
	}
	else
	{
		m_textEdit->setMargins(0);
		m_textEdit->setFont(f);
	}

	m_textEdit->setTabWidth(4);
	m_textEdit->setAutoIndent(true);

	connect(m_textEdit, &QsciScintilla::textChanged, this, &PropertyTextEditor::textChanged);
}

bool CodeEditor::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == m_textEdit)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

			if (keyEvent->key() == Qt::Key_Escape)
			{
				emit escapePressed();
				return true;
			}


			if (keyEvent->key() == Qt::Key_F && (keyEvent->modifiers() & Qt::ControlModifier))
			{
				if (m_findReplace == nullptr)
				{
					m_findReplace = new DialogFindReplace(this);

					connect(m_findReplace, &DialogFindReplace::findFirst, this, &CodeEditor::findFirst);
					connect(m_findReplace, &DialogFindReplace::replace, this, &CodeEditor::replace);
					connect(m_findReplace, &DialogFindReplace::replaceAll, this, &CodeEditor::replaceAll);
				}

				m_findReplace->show();

				return true;
			}

			if (keyEvent->key() == Qt::Key_F3)
			{
				findNext();
				return true;
			}
		}
	}

	// pass the event on to the parent class
	return PropertyTextEditor::eventFilter(obj, event);
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


void CodeEditor::findFirst(QString findText)
{
	if (findText.isEmpty())
	{
		return;
	}

	bool result = false;

	if (m_findText == findText)
	{
		result = m_textEdit->findNext();
	}
	else
	{
		result = m_textEdit->findFirst(findText, false/*regular*/, false/*caseSens*/, false/*whole*/, true/*wrap*/);

		m_findText = findText;
	}

	if (result == false)
	{
		QMessageBox::information(this, qAppName(), tr("Text was not found."));
	}
}

void CodeEditor::findNext()
{
	m_textEdit->findNext();
}

void CodeEditor::replace(QString findText, QString replaceText)
{
	if (findText.isEmpty() || replaceText.isEmpty())
	{
		return;
	}

	if (findText == m_findText)
	{
		m_textEdit->replace(replaceText);
		return;
	}

	m_findText = findText;

	if (m_textEdit->findFirst(findText, false/*regular*/, false/*caseSens*/, false/*whole*/, true/*wrap*/) == false)
	{
		return;
	}

	m_textEdit->replace(replaceText);
}

void CodeEditor::replaceAll(QString findText, QString replaceText)
{
	if (findText.isEmpty() || replaceText.isEmpty())
	{
		return;
	}

	int counter = 0;

	m_findText = findText;

	if (m_textEdit->findFirst(findText, false/*regular*/, false/*caseSens*/, false/*whole*/, true/*wrap*/) == false)
	{
		return;
	}

	while (counter < 1000)
	{
		m_textEdit->replace(replaceText);

		counter++;

		if (m_textEdit->findNext() == false)
		{
			break;
		}

		if (counter >= 1000)
		{
			// for not to hang
			assert(false);
			break;
		}
	}

	QMessageBox::information(this, qAppName(), tr("%1 replacements occured.").arg(counter));
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