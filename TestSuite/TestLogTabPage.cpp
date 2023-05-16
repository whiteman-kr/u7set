#include "TestLogTabPage.h"
#include "AppConfigSettings.h"
#include "../TestSuiteLib/TestLog.h"

TestLogTabPage::TestLogTabPage(TestSuite::TestLog& testLog, TestSuiteTestLogOutput& testLogOutput, QWidget* parent):
	QWidget(parent),
	m_testLog(testLog),
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

	m_typeCombo = new QComboBox();
	m_typeCombo->addItem("All Messages", static_cast<int>(TestSuite::TestLogItemType::All));
	m_typeCombo->addItem("Errors", static_cast<int>(TestSuite::TestLogItemType::Error));
	m_typeCombo->addItem("Warnings", static_cast<int>(TestSuite::TestLogItemType::Warning));
	connect(m_typeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TestLogTabPage::slot_typeChanged);

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

	rightWidgetLayout->addWidget(m_typeCombo, 1, 0);
	rightWidgetLayout->addWidget(m_prevIssueButton, 1, 1);
	rightWidgetLayout->addWidget(m_nextIssueButton, 1, 2);

	rightWidgetLayout->addWidget(m_findTextEdit, 1, 3, 1, 2);
	rightWidgetLayout->addWidget(m_findTextButton, 1, 5);

	rightWidgetLayout->setColumnStretch(6, 100);

	//rightWidgetLayout->setColumnStretch(0, 1);

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

void TestLogTabPage::clearOutputWidget()
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

void TestLogTabPage::slot_typeChanged(int index)
{
	TestSuite::TestLogItemType type{TestSuite::TestLogItemType::All};

	QVariant data = m_typeCombo->currentData();
	if (data.isValid() == true)
	{
		type = static_cast<TestSuite::TestLogItemType>(data.toInt());
	}

	if (type != TestSuite::TestLogItemType::All)
	{
		m_typeCombo->setStyleSheet("QComboBox { color: red }");
	}
	else
	{
		m_typeCombo->setStyleSheet(QString());
	}

	m_outputWidget->clear();
	appendLogMessages(m_testLog.items());
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

		appendLogMessages(messages);
	}

	return;
}

void TestLogTabPage::appendLogMessages(const std::vector<TestSuite::TestLogItem>& messages)
{
	QString outputMessagesBuffer;
	outputMessagesBuffer.reserve(128000);

	TestSuite::TestLogItemType warningLevel{TestSuite::TestLogItemType::All};

	QVariant data = m_typeCombo->currentData();
	if (data.isValid() == true)
	{
		warningLevel = static_cast<TestSuite::TestLogItemType>(data.toInt());
	}

	for (size_t i = 0; i < messages.size(); i++)
	{
		const TestSuite::TestLogItem& m = messages[i];

		if (warningLevel != TestSuite::TestLogItemType::All)
		{
			if (warningLevel == TestSuite::TestLogItemType::Error && m.isError() == false )
			{
				continue;
			}
			if (warningLevel == TestSuite::TestLogItemType::Warning && m.isWarning() == false )
			{
				continue;
			}
		}

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
