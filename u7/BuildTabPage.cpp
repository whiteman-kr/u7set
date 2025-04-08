#include "BuildTabPage.h"
#include "../Builder/Builder.h"
#include "AppSettings.h"
#include "GlobalMessanger.h"
#include "Settings.h"


//
//
// BuildTabPage
//
//
BuildTabPage::BuildTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent),
	m_builder{std::make_unique<Builder::Builder>(&GlobalMessanger::instance().buildIssues())}
{
	assert(dbcontroller != nullptr);

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

	QString selectionColor =
		QString("QTextEdit { selection-background-color: %1; selection-color: %2; }").arg(highlight.name()).arg(highlightText.name());

	m_outputWidget->setStyleSheet(selectionColor);

	//--
	//
	m_buildButton = new QPushButton(tr("Build... <F7>"));

	m_cancelButton = new QPushButton(tr("Cancel"));
	m_cancelButton->setEnabled(false);

	m_prevIssueButton = new QPushButton(tr("Prev Issue <Shift+F6>"));
	// m_prevIssueButton->setShortcut(Qt::SHIFT + Qt::Key_F6);	// Too slow, use usual QAction

	m_nextIssueButton = new QPushButton(tr("Next Issue <F6>"));
	// m_nextIssueButton->setShortcut(Qt::Key_F6);				// Too slow, use usual QAction

	m_findTextEdit = new QLineEdit();
	m_findTextEdit->setPlaceholderText("Find Text");
	m_findTextEdit->setMinimumWidth(300);

	QCompleter* searchCompleter = new QCompleter(theSettings.buildSearchCompleter(), this);
	searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findTextEdit->setCompleter(searchCompleter);

	m_findTextButton = new QPushButton(tr("Search <F3>"));
	// m_findTextButton->setShortcut(Qt::Key_F3);				// Too slow, use usual QAction

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
	m_vsplitter = new QSplitter{this};
	m_vsplitter->setChildrenCollapsible(false);

	m_settingsWidget = new QWidget{m_vsplitter};
	auto settingsWidgetLayout = new QFormLayout();

	QLabel* buildLabel = new QLabel("Build:");
	buildLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
	settingsWidgetLayout->addRow(buildLabel);

	for (int i = 0; i < 2; i++)
	{
		m_buildLabel[i] = new QLabel(i == 0 ? "Project is not opened" : QString());
		m_buildLabel[i]->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
		m_buildLabel[i]->setTextFormat(Qt::RichText);
		m_buildLabel[i]->setTextInteractionFlags(Qt::TextBrowserInteraction);
		m_buildLabel[i]->setOpenExternalLinks(true);
		settingsWidgetLayout->addRow(m_buildLabel[i]);
	}

	m_warningsLevelComboBox = new QComboBox(m_settingsWidget);

	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::ShowAll), tr("Show All Warnings"));
	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::Middle), tr("Mid Warnings"));
	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::Important), tr("Important Warnings"));
	m_warningsLevelComboBox->insertItem(static_cast<int>(WarningShowLevel::HideAll), tr("Hide All Warning"));
	m_warningsLevelComboBox->setCurrentIndex(theSettings.buildWarningLevel());
	settingsWidgetLayout->addRow(m_warningsLevelComboBox);

	// Option: Generate App Logic Drawings
	//
	QFrame* buildOptionLine = new QFrame;
	buildOptionLine->setFrameShape(QFrame::HLine);
	buildOptionLine->setFrameShadow(QFrame::Sunken);
	settingsWidgetLayout->addRow(buildOptionLine);

	// Option: Generate App Logic Drawings
	//
	auto buildOptions = new QLabel("Build Options:");
	buildOptions->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
	settingsWidgetLayout->addRow(buildOptions);

	auto createBuildOptionComboBox = [layout = settingsWidgetLayout, this](QString name, QString toolTip, QWidget* parent) -> QComboBox*
	{
		QLabel* label = new QLabel(name + QString(":"));
		label->setToolTip(toolTip);

		auto cb = new QComboBox{parent};
		cb->setEditable(false);
		cb->insertItem(static_cast<int>(E::BuildOptionValue::Inherit),
					   E::valueToString(E::BuildOptionValue::Inherit),
					   static_cast<int>(E::BuildOptionValue::Inherit));
		cb->insertItem(static_cast<int>(E::BuildOptionValue::True),
					   E::valueToString(E::BuildOptionValue::True),
					   static_cast<int>(E::BuildOptionValue::True));
		cb->insertItem(static_cast<int>(E::BuildOptionValue::False),
					   E::valueToString(E::BuildOptionValue::False),
					   static_cast<int>(E::BuildOptionValue::False));
		cb->setToolTip(toolTip);

		connect(cb,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				[this, name](int index)
				{
					QString strValue = E::valueToString(static_cast<E::BuildOptionValue>(index));
					db()->setUserProperty(name, strValue, this);
				});

		layout->addRow(label, cb);

		return cb;
	};

	m_generateAppLogicDrawings =
		createBuildOptionComboBox(Db::ProjectProperty::GenerateAppLogicDrawings,
								  tr("Inherit: Inherit value from the project setting\nTrue: %1\nFalse: Do not generate data")
									  .arg(Db::ProjectProperty::GenerateAppLogicDrawingsDescription),
								  m_settingsWidget);

	m_generateAppSignalsXml =
		createBuildOptionComboBox(Db::ProjectProperty::GenerateAppSignalsXml,
								  tr("Inherit: Inherit value from the project setting\nTrue: %1\nFalse: Do not generate data")
									  .arg(Db::ProjectProperty::GenerateAppSignalsXmlDescription),
								  m_settingsWidget);

	m_generateAppSignalsExtXml =
		createBuildOptionComboBox(Db::ProjectProperty::GenerateAppSignalsExtXml,
								  tr("Inherit: Inherit value from the project setting\nTrue: %1\nFalse: Do not generate data")
									  .arg(Db::ProjectProperty::GenerateAppSignalsExtXmlDescription),
								  m_settingsWidget);

	m_generateExtraDebugInfo =
		createBuildOptionComboBox(Db::ProjectProperty::GenerateExtraDebugInfo,
								  tr("Inherit: Inherit value from the project setting\nTrue: %1\nFalse: Do not generate data")
									  .arg(Db::ProjectProperty::GenerateExtraDebugInfo),
								  m_settingsWidget);

	QFrame* testOptionLine = new QFrame;
	testOptionLine->setFrameShape(QFrame::HLine);
	testOptionLine->setFrameShadow(QFrame::Sunken);
	settingsWidgetLayout->addRow(testOptionLine);

	m_runSimTestsOnBuild =
		createBuildOptionComboBox(Db::ProjectProperty::RunSimTestsOnBuild,
								  tr("Inherit: Inherit value from the project setting\nTrue: %1\nFalse: Do not tun tests")
									  .arg(Db::ProjectProperty::RunSimTestsOnBuild),
								  m_settingsWidget);

	// --
	//
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
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &BuildTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &BuildTabPage::projectClosed);

	connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::build);
	connect(m_cancelButton, &QAbstractButton::clicked, this, &BuildTabPage::cancel);

	connect(m_builder.get(), &Builder::Builder::started, this, &BuildTabPage::buildWasStarted);
	connect(m_builder.get(), &Builder::Builder::finished, this, &BuildTabPage::buildWasFinished);

	connect(m_builder.get(), &Builder::Builder::started, this, &BuildTabPage::buildStarted);
	connect(m_builder.get(), &Builder::Builder::finished, this, &BuildTabPage::buildFinished);

	connect(m_warningsLevelComboBox,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&BuildTabPage::warningsLevelChanged);

	connect(m_prevIssueButton, &QPushButton::clicked, this, &BuildTabPage::prevIssue);
	connect(m_nextIssueButton, &QPushButton::clicked, this, &BuildTabPage::nextIssue);

	connect(m_findTextEdit, &QLineEdit::returnPressed, this, &BuildTabPage::search);
	connect(m_findTextButton, &QPushButton::clicked, this, &BuildTabPage::search);

	// Output Log
	//
	m_logTimerId = startTimer(20);

	m_builder->log().setHtmlFont("Verdana");

	m_messages.reserve(1024 * 10);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);

	return;
}

BuildTabPage::~BuildTabPage()
{
	theSettings.m_buildTabPageSplitterState = m_vsplitter->saveState();
	theSettings.writeUserScope();
}

bool BuildTabPage::isBuildRunning() const
{
	return m_builder->isRunning();
}

const std::map<QUuid, OutputMessageLevel>* BuildTabPage::itemsIssues() const
{
	return &m_itemsIssues;
}

void BuildTabPage::cancelBuild()
{
	if (m_builder->isRunning() == true)
	{
		m_builder->stop();

		// wait for 20 seconds while build stops
		//
		for (int i = 0; i < 20000 && m_builder->isRunning() == true; i++)
		{
			QThread::msleep(10);
		}

		if (m_builder->isRunning() == true)
		{
			qDebug() << "WARNING: Exit while the build thread is still running!";
		}
	}
}

int BuildTabPage::progress() const
{
	return m_builder->progress();
}

void BuildTabPage::CreateActions()
{
	m_findNextAction = new QAction(tr("Find Text"), this);
	m_findNextAction->setShortcut(Qt::Key_F3);
	connect(m_findNextAction, &QAction::triggered, this, &BuildTabPage::search);
	addAction(m_findNextAction);

	m_prevIssueAction = new QAction(tr("Prev Issue"), this);
	m_prevIssueAction->setShortcut(Qt::SHIFT | Qt::Key_F6);
	connect(m_prevIssueAction, &QAction::triggered, this, &BuildTabPage::prevIssue);
	addAction(m_prevIssueAction);

	m_nextIssueAction = new QAction(tr("Next Issue"), this);
	m_nextIssueAction->setShortcut(Qt::Key_F6);
	connect(m_nextIssueAction, &QAction::triggered, this, &BuildTabPage::nextIssue);
	addAction(m_nextIssueAction);

	return;
}

void BuildTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void BuildTabPage::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == m_logTimerId && m_builder->log().isEmpty() == false && m_outputWidget != nullptr)
	{
		thread_local std::vector<OutputLogItem> messages;
		messages.clear();

		m_builder->log().popMessages(&messages, 40);

		std::copy(messages.begin(), messages.end(), std::back_inserter(m_messages));

		appendMessagesToOutputLog(messages);

		return;
	}

	return;
}

void BuildTabPage::projectOpened()
{
	QString buildCurrentPath;
	QString buildLastPath;
	getProjectBuildPath(&buildCurrentPath, &buildLastPath);

	if (buildLastPath.isEmpty() == false)
	{
		m_buildLabel[0]->setText(QStringLiteral("<a href=\"%1\">%1</a>").arg(buildLastPath));
	}
	else
	{
		m_buildLabel[0]->setText(tr("No build performed yet"));
	}

	if (buildCurrentPath.isEmpty() == false)
	{
		m_buildLabel[1]->setText(QStringLiteral("<a href=\"%1\">%1</a>").arg(buildCurrentPath));
	}
	else
	{
		m_buildLabel[1]->setText(QString());
	}

	// Set Build options
	//
	{
		auto setBuildOption = [this](QComboBox* cb, QString property)
		{
			QString strValue;
			db()->getUserProperty(property, &strValue, E::valueToString(E::BuildOptionValue::Inherit), this);

			int index = static_cast<int>(E::stringToValue<E::BuildOptionValue>(strValue).first);

			cb->blockSignals(true);
			cb->setCurrentIndex(index);
			cb->blockSignals(false);
		};

		setBuildOption(m_generateAppLogicDrawings, Db::ProjectProperty::GenerateAppLogicDrawings);
		setBuildOption(m_generateAppSignalsXml, Db::ProjectProperty::GenerateAppSignalsXml);
		setBuildOption(m_generateAppSignalsExtXml, Db::ProjectProperty::GenerateAppSignalsExtXml);
		setBuildOption(m_generateExtraDebugInfo, Db::ProjectProperty::GenerateExtraDebugInfo);
		setBuildOption(m_runSimTestsOnBuild, Db::ProjectProperty::RunSimTestsOnBuild);
	}

	// --
	//
	this->setEnabled(true);

	return;
}

void BuildTabPage::projectClosed()
{
	cancelBuild();

	m_buildLabel[0]->setText(tr("Project is not opened"));
	m_buildLabel[1]->setText(QString());

	m_findTextEdit->clear();
	m_outputWidget->clear();
	m_messages.clear();

	this->setEnabled(false);
	return;
}

void BuildTabPage::build()
{
	m_outputWidget->clear();
	m_messages.clear();

	// --
	//
	Builder::BuildOptions buildOptions;

	buildOptions.generateAppLogicDrawings = static_cast<E::BuildOptionValue>(m_generateAppLogicDrawings->currentData().toInt());
	buildOptions.generateAppSignalsXml = static_cast<E::BuildOptionValue>(m_generateAppSignalsXml->currentData().toInt());
	buildOptions.generateAppSignalsExtXml = static_cast<E::BuildOptionValue>(m_generateAppSignalsExtXml->currentData().toInt());
	buildOptions.generateExtraDebugInfo = static_cast<E::BuildOptionValue>(m_generateExtraDebugInfo->currentData().toInt());
	buildOptions.runSimTestsOnBuild = static_cast<E::BuildOptionValue>(m_runSimTestsOnBuild->currentData().toInt());

	GlobalMessanger::instance().fireBuildStarted();

	m_builder->start(db()->host(),
					 db()->port(),
					 db()->serverUsername(),
					 db()->serverPassword(),
					 db()->currentProject().projectName(),
					 db()->currentUser().username(),
					 db()->currentUser().password(),
					 theAppSettings.buildOutputPath(),
					 theAppSettings.isExpertMode(),
					 buildOptions,
					 QStringList{});

	return;
}

void BuildTabPage::cancel()
{
	m_builder->stop();
}

void BuildTabPage::buildWasStarted()
{
	// This is required for showing progress indicator on task bar button
	// #ifdef Q_OS_WIN32
	//	m_taskbarButton->setWindow(windowHandle());
	// #endif

	//	QWinTaskbarButton* button = new QWinTaskbarButton(this);
	//	button->setWindow(windowHandle());
	//	QWinTaskbarProgress* progress = button->progress();

	//	progress->setRange(0, 100);
	//	progress->show();
	//	progress->setValue(50);

	GlobalMessanger::instance().clearBuildSchemaIssues();

	m_buildButton->setEnabled(false);
	m_cancelButton->setEnabled(true);
}

void BuildTabPage::buildWasFinished(int errorCount)
{
	//	QWinTaskbarButton* button = new QWinTaskbarButton(this);
	//	QWinTaskbarProgress* progress = button->progress();
	//	progress->hide();

	m_buildButton->setEnabled(true);
	m_cancelButton->setEnabled(false);

	m_itemsIssues.clear();

	GlobalMessanger::instance().fireBuildFinished(errorCount);

	QString buildCurrentPath;
	QString buildLastPath;
	getProjectBuildPath(&buildCurrentPath, &buildLastPath);

	if (buildLastPath.isEmpty() == false)
	{
		m_buildLabel[0]->setText(QStringLiteral("<a href=\"%1\">%1</a>").arg(buildLastPath));
	}
	else
	{
		m_buildLabel[0]->setText(tr("No build performed yet"));
	}

	if (buildCurrentPath.isEmpty() == false)
	{
		m_buildLabel[1]->setText(QStringLiteral("<a href=\"%1\">%1</a>").arg(buildCurrentPath));
	}
	else
	{
		m_buildLabel[1]->setText(tr("No build performed yet"));
	}

	return;
}

void BuildTabPage::warningsLevelChanged(int index)
{
	theSettings.setBuildWarningLevel(index);

	// Refill the output window
	//
	QTextCursor m_lastNavCursor;
	m_lastNavIsPrevIssue = false;
	m_lastNavIsNextIssue = false;

	m_outputWidget->clear();

	appendMessagesToOutputLog(m_messages);

	return;
}

void BuildTabPage::prevIssue()
{
	assert(m_outputWidget);

	QString regExpVal("\\b(ERR|WRN)\\b");

	//  --
	//
	if ((m_lastNavIsNextIssue == true || m_lastNavIsPrevIssue == true) && m_outputWidget->textCursor() == m_lastNavCursor)
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

		// Highlight the line
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

	//  --
	//
	if (m_lastNavIsPrevIssue == true && m_outputWidget->textCursor() == m_lastNavCursor)
	{
		m_lastNavCursor.movePosition(QTextCursor::EndOfLine);
		m_outputWidget->setTextCursor(m_lastNavCursor);
	}

	// Find Issue
	//
	thread_local const QRegularExpression rx{"\\b(ERR|WRN)\\b"};
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

		// Highlight the line
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
		// Try to find one more time from the document start
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

void BuildTabPage::getProjectBuildPath(QString* buildCurrentPath, QString* buildLastPath) const
{
	if (buildCurrentPath == nullptr || buildLastPath == nullptr)
	{
		Q_ASSERT(buildCurrentPath);
		Q_ASSERT(buildLastPath);
		return;
	}

	buildCurrentPath->clear();
	buildLastPath->clear();

	QString buildBasePath =
		QDir::fromNativeSeparators(QStringLiteral("%1/%2").arg(theAppSettings.buildOutputPath()).arg(db()->currentProject().projectName()));

	// Current build path (/build)
	//
	*buildCurrentPath = QStringLiteral("%1/build").arg(buildBasePath);
	if (QDir().exists(*buildCurrentPath) == false)
	{
		buildCurrentPath->clear();
	}

	// Last build path (/build-xxxxxx)
	//
	QStringList buildDirsList = QDir(buildBasePath).entryList({QStringLiteral("build-*")}, QDir::Dirs, QDir::Name);

	if (buildDirsList.isEmpty() == false)
	{
		*buildLastPath = QStringLiteral("%1/%2").arg(buildBasePath).arg(buildDirsList.last());
	}

	return;
}

void BuildTabPage::appendMessagesToOutputLog(const std::vector<OutputLogItem>& messages)
{
	WarningShowLevel warningShowLevel = static_cast<WarningShowLevel>(theSettings.buildWarningLevel());

	QString outputMessagesBuffer;
	outputMessagesBuffer.reserve(64000);

	auto filter = [warningShowLevel](const OutputLogItem& m)
	{
		if (warningShowLevel == WarningShowLevel::HideAll && m.isWarning() == true)
		{
			return false;
		}

		if (warningShowLevel == WarningShowLevel::Important && (m.isWarning1() == true || m.isWarning2()))
		{
			return false;
		}

		if (warningShowLevel == WarningShowLevel::Middle && m.isWarning2())
		{
			return false;
		}

		return true;
	};

	for (const OutputLogItem& m : messages | std::views::filter(filter))
	{
		outputMessagesBuffer.append(m.toHtml());

		if (&m != &messages.back()) // It is not the last message.
		{
			outputMessagesBuffer += QLatin1String("<br>");
		}
	}

	if (outputMessagesBuffer.isEmpty() == false)
	{
		m_outputWidget->append(outputMessagesBuffer);
	}
}
