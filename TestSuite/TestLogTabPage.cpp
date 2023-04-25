#include "TestLogTabPage.h"
#include "AppConfigSettings.h"

TestLogTabPage::TestLogTabPage(TestSuiteTestLogOutput& testLogOutput, QWidget* parent):
	QWidget(parent),
	m_testLogOutput(testLogOutput)
{
	// Output windows
	//
	m_outputWidget = new QTextBrowser();
	m_outputWidget->setReadOnly(true);
	m_outputWidget->setLineWrapMode(QTextEdit::NoWrap);
	m_outputWidget->setAutoFormatting(QTextEdit::AutoNone);
	m_outputWidget->document()->setUndoRedoEnabled(false);
	m_outputWidget->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_outputWidget->setOpenExternalLinks(true);

	auto p = qApp->palette("QListView");

	QColor highlight = p.highlight().color();
	QColor highlightText = p.highlightedText().color();

	QString selectionColor = QString("QTextEdit { selection-background-color: %1; selection-color: %2; }")
							 .arg(highlight.name())
							 .arg(highlightText.name());

	m_outputWidget->setStyleSheet(selectionColor);

	m_prevIssueButton = new QPushButton(tr("Prev Issue <Shift+F6>"));
	//m_prevIssueButton->setShortcut(Qt::SHIFT + Qt::Key_F6);	// Too slow, use usual QAction

	m_nextIssueButton = new QPushButton(tr("Next Issue <F6>"));
	//m_nextIssueButton->setShortcut(Qt::Key_F6);				// Too slow, use usual QAction

	m_findTextEdit = new QLineEdit();
	m_findTextEdit->setPlaceholderText("Find Text");
	m_findTextEdit->setMinimumWidth(300);

	QCompleter* searchCompleter = new QCompleter(theSettings.outputSearchCompleter(), this);
	searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findTextEdit->setCompleter(searchCompleter);

	m_findTextButton = new QPushButton(tr("Search <F3>"));
	//m_findTextButton->setShortcut(Qt::Key_F3);				// Too slow, use usual QAction

	QGridLayout* rightWidgetLayout = new QGridLayout();

	rightWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 8);

	rightWidgetLayout->addWidget(m_prevIssueButton, 1, 0);
	rightWidgetLayout->addWidget(m_nextIssueButton, 1, 1);

	rightWidgetLayout->addWidget(m_findTextEdit, 1, 2, 1, 2);
	rightWidgetLayout->addWidget(m_findTextButton, 1, 4);

	rightWidgetLayout->setColumnStretch(5, 100);

	rightWidgetLayout->setColumnStretch(0, 1);

	//
	// Layouts
	//

	setLayout(rightWidgetLayout);

	//
	connect(m_prevIssueButton, &QPushButton::clicked, this, &TestLogTabPage::prevIssue);
	connect(m_nextIssueButton, &QPushButton::clicked, this, &TestLogTabPage::nextIssue);

	connect(m_findTextEdit, &QLineEdit::returnPressed, this, &TestLogTabPage::search);
	connect(m_findTextButton, &QPushButton::clicked, this, &TestLogTabPage::search);

	m_logTimerId = startTimer(10);
}

void TestLogTabPage::clearOutputLog()
{
	m_outputWidget->clear();
}

void TestLogTabPage::testingWasStarted()
{

}

void TestLogTabPage::testingWasFinished(int errorCount)
{

}

void TestLogTabPage::prevIssue()
{

}

void TestLogTabPage::nextIssue()
{

}

void TestLogTabPage::search()
{

}

void TestLogTabPage::createActions()
{
	m_findNextAction = new QAction(tr("Find Text"), this);
	m_findNextAction->setShortcut(Qt::Key_F3);
	connect(m_findNextAction, &QAction::triggered, this, &TestLogTabPage::search);
	addAction(m_findNextAction);

	m_prevIssueAction = new QAction(tr("Prev Issue"), this);
	m_prevIssueAction->setShortcut(Qt::SHIFT | Qt::Key_F6);
	connect(m_prevIssueAction, &QAction::triggered, this, &TestLogTabPage::prevIssue);
	addAction(m_prevIssueAction);

	m_nextIssueAction = new QAction(tr("Next Issue"), this);
	m_nextIssueAction->setShortcut(Qt::Key_F6);
	connect(m_nextIssueAction, &QAction::triggered, this, &TestLogTabPage::nextIssue);
	addAction(m_nextIssueAction);

	return;
}

void TestLogTabPage::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == m_logTimerId &&
		m_testLogOutput.queueIsEmpty() == false &&
		m_outputWidget != nullptr)
	{
		std::vector<TestSuite::TestLogItem> messages;
		messages.reserve(20);

		if (m_testLogOutput.queueIsEmpty() == false)
		{
			m_testLogOutput.popQueue(&messages, 40);
		}

		QString outputMessagesBuffer;
		outputMessagesBuffer.reserve(128000);

		for (size_t i = 0; i < messages.size(); i++)
		{
			const TestSuite::TestLogItem& m = messages[i];

//			if (warningShowLevel == WarningShowLevel::HideAll &&
//				m.isWarning() == true)
//			{
//				continue;
//			}

//			if (warningShowLevel == WarningShowLevel::Important &&
//				(m.isWarning1() == true || m.isWarning2()))
//			{
//				continue;
//			}

//			if (warningShowLevel == WarningShowLevel::Middle &&
//				m.isWarning2())
//			{
//				continue;
//			}

			outputMessagesBuffer.append(m.toHtml());

			if (i != messages.size() - 1)
			{
				outputMessagesBuffer += QLatin1String("<br>");
			}
		}

		if (outputMessagesBuffer.isEmpty() == false)
		{
			m_outputWidget->append(outputMessagesBuffer);
		}

		return;
	}
}
