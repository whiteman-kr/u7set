#include "Stable.h"
#include "BuildTabPage.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"
#include <QTextBlock>
#include <QComboBox>
#include <QCompleter>

//
//
// BuildTabPage
//
//
//BuildTabPage* BuildTabPage::m_this = nullptr;
const char* BuildTabPage::m_buildLogFileName = "buildlog.html";


BuildTabPage::BuildTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent),
	m_builder(&m_outputLog)
{
	assert(dbcontroller != nullptr);

	//	BuildTabPage::m_this = this;

	//
	// Controls
	//

	// Create Actions
	//
	CreateActions();

	// Right Side Widget
	//
	m_rightSideWidget = new QWidget();

	// Output windows
	//
	m_outputWidget = new QTextEdit();
	m_outputWidget->setReadOnly(true);
	m_outputWidget->setLineWrapMode(QTextEdit::NoWrap);
	m_outputWidget->setAutoFormatting(QTextEdit::AutoNone);
	m_outputWidget->document()->setUndoRedoEnabled(false);

	m_buildButton = new QPushButton(tr("Build... <F7>"));

	m_cancelButton = new QPushButton(tr("Cancel"));
	m_cancelButton->setEnabled(false);

	m_prevIssueButton = new QPushButton(tr("Prev Issue <Shift+F6>"));
	m_prevIssueButton->setShortcut(Qt::SHIFT + Qt::Key_F6);

	m_nextIssueButton = new QPushButton(tr("Next Issue <F6>"));
	m_nextIssueButton->setShortcut(Qt::Key_F6);

	m_findTextEdit = new QLineEdit();
	m_findTextEdit->setPlaceholderText("Find Text");
	m_findTextEdit->setMinimumWidth(300);

	QCompleter* searchCompleter = new QCompleter(theSettings.buildSearchCompleter(), this);
	searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findTextEdit->setCompleter(searchCompleter);

	m_findTextButton = new QPushButton(tr("Search <F3>"));
	m_findTextButton->setShortcut(Qt::Key_F3);

	QGridLayout* rightWidgetLayout = new QGridLayout();

	rightWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 8);

	rightWidgetLayout->addWidget(m_prevIssueButton, 1, 0);
	rightWidgetLayout->addWidget(m_nextIssueButton, 1, 1);

	rightWidgetLayout->addWidget(m_findTextEdit, 1, 2, 1, 2);
	rightWidgetLayout->addWidget(m_findTextButton, 1, 4);

	rightWidgetLayout->setColumnStretch(5, 100);

	rightWidgetLayout->addWidget(m_buildButton, 1, 6);
	rightWidgetLayout->addWidget(m_cancelButton, 1, 7);

	rightWidgetLayout->setColumnStretch(0, 1);

	m_rightSideWidget->setLayout(rightWidgetLayout);

	// Left Side
	//
	m_vsplitter = new QSplitter(this);

	m_settingsWidget = new QWidget(m_vsplitter);
	QVBoxLayout* settingsWidgetLayout = new QVBoxLayout();

	m_debugCheckBox = new QCheckBox(tr("Debug build"), m_settingsWidget);
	m_debugCheckBox->setChecked(true);
	settingsWidgetLayout->addWidget(m_debugCheckBox);

	m_warningsLevelComboBox = new QComboBox(m_settingsWidget);

	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::ShowAll), tr("Show All Warnings"));
	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::Middle), tr("Mid Warnings"));
	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::Important), tr("Important Warnings"));
	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::HideAll), tr("Hide All Warning"));
	m_warningsLevelComboBox->setCurrentIndex(theSettings.buildWarningLevel());
	settingsWidgetLayout->addWidget(m_warningsLevelComboBox);

	settingsWidgetLayout->addStretch();

	m_settingsWidget->setLayout(settingsWidgetLayout);

	// V Splitter
	//
	m_vsplitter->addWidget(m_settingsWidget);
	m_vsplitter->addWidget(m_rightSideWidget);

	m_vsplitter->setStretchFactor(0, 2);
	m_vsplitter->setStretchFactor(1, 1);

	m_vsplitter->restoreState(theSettings.m_buildTabPageSplitterState);

	//
	// Layouts
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_vsplitter);

	setLayout(pMainLayout);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &BuildTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &BuildTabPage::projectClosed);

	connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::build);
	connect(m_cancelButton, &QAbstractButton::clicked, this, &BuildTabPage::cancel);

	connect(GlobalMessanger::instance(), &GlobalMessanger::buildStarted, this, &BuildTabPage::buildWasStarted);
	connect(GlobalMessanger::instance(), &GlobalMessanger::buildFinished, this, &BuildTabPage::buildWasFinished);

	connect(m_warningsLevelComboBox , static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[](int index)
			{
				theSettings.setBuildWarningLevel(index);
			}
			);

	connect(m_prevIssueButton, &QPushButton::clicked, this, &BuildTabPage::prevIssue);
	connect(m_nextIssueButton, &QPushButton::clicked, this, &BuildTabPage::nextIssue);

	connect(m_findTextEdit, &QLineEdit::returnPressed, this, &BuildTabPage::search);
	connect(m_findTextButton, &QPushButton::clicked, this, &BuildTabPage::search);

	////connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::buildStarted);	// On button clicked event!!!
	//connect(&m_builder, &Builder::Builder::buildFinished, this, &BuildTabPage::buildFinished);

	// Output Log
	//
	m_logTimerId = startTimer(10);

	m_outputLog.setHtmlFont("Verdana");

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

BuildTabPage::~BuildTabPage()
{
	theSettings.m_buildTabPageSplitterState = m_vsplitter->saveState();
	theSettings.writeUserScope();
}

bool BuildTabPage::isBuildRunning() const
{
	return m_builder.isRunning();
}

const std::map<QUuid, OutputMessageLevel>* BuildTabPage::itemsIssues() const
{
	return &m_itemsIssues;
}

void BuildTabPage::cancelBuild()
{
	if (m_builder.isRunning() == true)
	{
		m_builder.stop();

		// wait for 20 seconds while bild stops
		//
		for (int i = 0; i < 2000 && m_builder.isRunning() == true; i++)
		{
			QThread::msleep(10);
		}

		if (m_builder.isRunning() == true)
		{
			qDebug() << "WARNING: Exit while the build thread is still running!";
		}
	}
}

void BuildTabPage::CreateActions()
{
//	m_addSystemAction = new QAction(tr("System"), this);
//	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
//	m_addSystemAction->setEnabled(false);
//	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

	return;
}

void BuildTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void BuildTabPage::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == m_logTimerId &&
		m_outputLog.isEmpty() == false &&
		m_outputWidget != nullptr)
	{
		WarningShowLevel warningShowLevel = static_cast<WarningShowLevel>(theSettings.buildWarningLevel());

		std::vector<OutputLogItem> messages;
		messages.reserve(20);

		if (m_outputLog.isEmpty() == false)
		{
			m_outputLog.popMessages(&messages, 40);
		}

		QString outputMessagesBuffer;
		outputMessagesBuffer.reserve(128000);

		for (size_t i = 0; i < messages.size(); i++)
		{
			const OutputLogItem& m = messages[i];

			if (warningShowLevel == WarningShowLevel::HideAll &&
				m.isWarning() == true)
			{
				continue;
			}

			if (warningShowLevel == WarningShowLevel::Important &&
				(m.isWarning1() == true || m.isWarning2()))
			{
				continue;
			}

			if (warningShowLevel == WarningShowLevel::Middle &&
				m.isWarning2())
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

	return;
}

void BuildTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void BuildTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

void BuildTabPage::build()
{
	m_outputLog.clear();
	m_outputWidget->clear();

	// init build log file
	//
    QString logFileName = QDir::fromNativeSeparators(theSettings.buildOutputPath());

	if (logFileName.endsWith('/') == false)
	{
		logFileName += '/';
	}
	logFileName += m_buildLogFileName;

	if (m_logFile.isOpen() == true && m_logFile.fileName() == logFileName)
	{
		// it is ok, file is open and has right name
		//
	}
	else
	{
		if (m_logFile.isOpen())
		{
			m_logFile.close();
		}

		m_logFile.setFileName(logFileName);
		m_logFile.open(QIODevice::Append | QIODevice::Text);
	}

	if (m_logFile.isOpen() == true)
	{
		LOG_MESSAGE((&m_outputLog), tr("Build log file: %1").arg(logFileName));

	}
	else
	{
		LOG_WARNING_OBSOLETE((&m_outputLog), Builder::IssueType::NotDefined,  tr("Cannot open output log file (%1) for writing").arg(logFileName));
	}

	// --
	//
	GlobalMessanger::instance()->fireBuildStarted();

	bool debug = m_debugCheckBox->isChecked();

	m_builder.start(
		db()->currentProject().projectName(),
		db()->host(),
		db()->port(),
		db()->serverUsername(),
		db()->serverPassword(),
		db()->currentUser().username(),
		db()->currentUser().password(),
		debug);

	return;
}

void BuildTabPage::cancel()
{
	m_builder.stop();
}

void BuildTabPage::buildWasStarted()
{
	m_buildButton->setEnabled(false);
	m_cancelButton->setEnabled(true);
}

void BuildTabPage::buildWasFinished()
{
	m_buildButton->setEnabled(true);
	m_cancelButton->setEnabled(false);

	m_itemsIssues.clear();

	return;
}

void BuildTabPage::prevIssue()
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
	QRegExp rx(regExpVal);
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

void BuildTabPage::nextIssue()
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
	QRegExp rx(regExpVal);
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

void BuildTabPage::search()
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
	if (theSettings.buildSearchCompleter().contains(searchText, Qt::CaseInsensitive) == false)
	{
		theSettings.buildSearchCompleter() << searchText;

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_findTextEdit->completer()->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.buildSearchCompleter());
		}
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

	if (found == true)
	{
		m_outputWidget->setFocus();
	}

	return;
}

