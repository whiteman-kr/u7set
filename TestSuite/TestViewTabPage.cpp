#include "TestViewTabPage.h"

TestViewTabPage::TestViewTabPage(const TestSuite::TestScript& script, QWidget* parent):
	QWidget(parent),
	m_script(script)
{
	// Create code editor
	//
	m_codeEditor = new CodeEditor(this);
	m_codeEditor->setText(m_script.script());
	m_codeEditor->setReadOnly(true);
	m_codeEditor->setCaretLineVisible(true);
	m_codeEditor->setCaretLineBackgroundColor(0xf0f0f0);

	// Set up default font
	//
#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas", 11);
#else
		QFont f = QFont("Courier");
#endif
	m_codeEditor->setFont(f);

	// Set up lexer
	//
	JsHighlighter::createJsHighlighter(m_codeEditor);

	// Set up margins
	//
	m_codeEditor->setLineNumberAreaForegroundColor(QColor(0xc0c0c0));
	m_codeEditor->setLineNumberAreaBackgroundColor(QColor(0xf0f0f0));

	// Main layout
	//
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(m_codeEditor);
	setLayout(layout);
}

TestViewTabPage::~TestViewTabPage()
{
	qDebug() << "TestViewTabPage " << m_script.fileName() << " closed.";
}

void TestViewTabPage::setScript(const TestSuite::TestScript& script)
{
	m_script = script;
	m_codeEditor->setText(m_script.script());
}

const TestSuite::TestScript& TestViewTabPage::script() const
{
	return m_script;
}

void TestViewTabPage::scrollToFunction(const QString& functionName)
{
	QTimer::singleShot(10, [this, functionName]() 
		{
			bool result = m_codeEditor->findFirst("function " + functionName, true/*caseSensitive*/, true/*wholeWord*/);
			if (result == false)
			{
				m_codeEditor->findFirst(functionName, true/*caseSensitive*/, true/*wholeWord*/);
			}
		});
}
