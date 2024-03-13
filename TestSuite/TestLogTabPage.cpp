#include "TestLogTabPage.h"
#include "AppConfigSettings.h"
#include "../TestSuiteLib/TestLog.h"
#include <QStringListModel>

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
	m_typeCombo->addItem(tr("All Messages"),	static_cast<int>(TestSuite::TestLogItemType::All));
	m_typeCombo->addItem(tr("Errors&Warnings"),	static_cast<int>(TestSuite::TestLogItemType::Error) | static_cast<int>(TestSuite::TestLogItemType::Warning));
	m_typeCombo->addItem(tr("Errors"),			static_cast<int>(TestSuite::TestLogItemType::Error));
	m_typeCombo->addItem(tr("Warnings"),		static_cast<int>(TestSuite::TestLogItemType::Warning));
	connect(m_typeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TestLogTabPage::slot_typeChanged);

	m_prevIssueButton = new QPushButton(tr("Prev Issue <Shift+F6>"));
	m_prevIssueButton->setShortcut(Qt::SHIFT | Qt::Key_F6);

	m_nextIssueButton = new QPushButton(tr("Next Issue <F6>"));
	m_nextIssueButton->setShortcut(Qt::Key_F6);

	m_findTextEdit = new QLineEdit();
	m_findTextEdit->setPlaceholderText(tr("Find Text"));
	m_findTextEdit->setMinimumWidth(300);

	QStringList outputSerachCompleter = QSettings().value("TestLogTabPage/m_buildSerachCompleter").toStringList();

	QCompleter* searchCompleter = new QCompleter(outputSerachCompleter, this);
	searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findTextEdit->setCompleter(searchCompleter);

	m_findTextButton = new QPushButton(tr("Search <F3>"));
	m_findTextButton->setShortcut(Qt::Key_F3);

	QGridLayout* rightWidgetLayout = new QGridLayout();

	rightWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 8);

	rightWidgetLayout->addWidget(m_typeCombo, 1, 0);
	rightWidgetLayout->addWidget(m_prevIssueButton, 1, 1);
	rightWidgetLayout->addWidget(m_nextIssueButton, 1, 2);

	rightWidgetLayout->addWidget(m_findTextEdit, 1, 3, 1, 2);
	rightWidgetLayout->addWidget(m_findTextButton, 1, 4);

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

void TestLogTabPage::testingWasFinished(int /*errorCount*/)
{

}

void TestLogTabPage::prevIssue()
{
	assert(m_outputWidget);

	QString regExpVal("\\b(ERR|WRN)\\b");

	//  --
	//
	if ((m_lastNavIsNextIssue == true || m_lastNavIsPrevIssue == true) &&
		m_outputWidget->textCursor() == m_lastNavCursor)
	{
		m_lastNavCursor.movePosition(QTextCursor::StartOfLine);
		m_outputWidget->setTextCursor(m_lastNavCursor);
	}

	// Find issue
	//
	QRegularExpression rx(regExpVal);
	bool found = m_outputWidget->find(rx, QTextDocument::FindBackward);

	if (found == false)
	{
		// Try to find one more time from the end
		//
		QTextCursor textCursor = m_outputWidget->textCursor();
		textCursor.movePosition(QTextCursor::End);
		m_outputWidget->setTextCursor(textCursor);

		found = m_outputWidget->find(rx, QTextDocument::FindBackward);
	}

	if (found == true)
	{
		// Set cursor int middle of the word, as now it is after selected word and backward find will give the same result
		//
		QTextCursor textCursor = m_outputWidget->textCursor();
		textCursor.movePosition(QTextCursor::PreviousCharacter);
		m_outputWidget->setTextCursor(textCursor);

		// Hightlight the line
		//
		QTextEdit::ExtraSelection highlight;
		highlight.cursor = m_outputWidget->textCursor();
		highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
		highlight.format.setBackground(Qt::yellow);

		QList<QTextEdit::ExtraSelection> extras;
		extras << highlight;

		m_outputWidget->setExtraSelections(extras);

		// Save this search data
		//
		m_lastNavIsPrevIssue = true;
		m_lastNavIsNextIssue = false;
		m_lastNavCursor = m_outputWidget->textCursor();
	}

	return;
}

void TestLogTabPage::nextIssue()
{
	assert(m_outputWidget);

	QString regExpVal("\\b(ERR|WRN)\\b");

	//  --
	//
	if (m_lastNavIsPrevIssue == true &&
		m_outputWidget->textCursor() == m_lastNavCursor)
	{
		m_lastNavCursor.movePosition(QTextCursor::EndOfLine);
		m_outputWidget->setTextCursor(m_lastNavCursor);
	}

	// Find Issue
	//
	QRegularExpression rx(regExpVal);
	bool found = m_outputWidget->find(rx);

	if (found == false)
	{
		// Try to find one more time from the beginning
		//
		QTextCursor textCursor = m_outputWidget->textCursor();
		textCursor.movePosition(QTextCursor::Start);
		m_outputWidget->setTextCursor(textCursor);

		found = m_outputWidget->find(rx);
	}

	if (found == true)
	{
		// Set cursor int middle of the word, as now it is after selected word and backward find will give the same result
		//
		QTextCursor textCursor = m_outputWidget->textCursor();
		textCursor.clearSelection();
		m_outputWidget->setTextCursor(textCursor);

		// Hightlight the line
		//
		QTextEdit::ExtraSelection highlight;
		highlight.cursor = m_outputWidget->textCursor();
		highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
		highlight.format.setBackground(Qt::yellow);

		QList<QTextEdit::ExtraSelection> extras;
		extras << highlight;

		m_outputWidget->setExtraSelections(extras);

		// Save this search data
		//
		m_lastNavIsPrevIssue = false;
		m_lastNavIsNextIssue = true;
		m_lastNavCursor = m_outputWidget->textCursor();
	}

	return;
}

void TestLogTabPage::search()
{
	assert(m_findTextEdit);
	assert(m_outputWidget);

	// Get search text
	//
	QString searchText = m_findTextEdit->text();

	if (searchText.isEmpty() == true)
	{
		m_findTextEdit->setFocus();
		return;
	}

	// Update completer
	//
	QStringList outputSerachCompleter = QSettings().value("TestLogTabPage/m_buildSerachCompleter").toStringList();
	
	if (outputSerachCompleter.contains(searchText, Qt::CaseInsensitive) == false)
	{
		outputSerachCompleter << searchText;

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_findTextEdit->completer()->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(outputSerachCompleter);
		}

		QSettings().setValue("TestLogTabPage/m_buildSerachCompleter", outputSerachCompleter);
	}

	// Find
	//
	bool found = m_outputWidget->find(searchText);

	if (found == false)
	{
		// Try to find one more time from the documnet start
		//
		QTextCursor textCursor = m_outputWidget->textCursor();
		textCursor.movePosition(QTextCursor::Start);
		m_outputWidget->setTextCursor(textCursor);

		found = m_outputWidget->find(searchText);
	}

//	if (found == true)
//	{
//		m_outputWidget->setFocus();
//	}

	return;
}

void TestLogTabPage::slot_typeChanged(int /*index*/)
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

	TestSuite::TestLogItemType showType{TestSuite::TestLogItemType::All};

	QVariant data = m_typeCombo->currentData();
	if (data.isValid() == true)
	{
		showType = static_cast<TestSuite::TestLogItemType>(data.toInt());
	}

	for (size_t i = 0; i < messages.size(); i++)
	{
		const TestSuite::TestLogItem& m = messages[i];

		// Filter by type
		//
		if ((static_cast<int>(showType) & static_cast<int>(m.type())) == 0)
		{
			continue;
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
