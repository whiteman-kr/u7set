#include "Stable.h"
#include "BuildTabPage.h"
#include "Settings.h"
#include "../include/DbController.h"


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
	m_outputWidget->document()->setUndoRedoEnabled(false);
	m_outputWidget->document()->setMaximumBlockCount(600);

	m_buildButton = new QPushButton(tr("Build..."));
	m_cancelButton = new QPushButton(tr("Cancel"));

	QGridLayout* rightWidgetLayout = new QGridLayout();

	rightWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 3);
	rightWidgetLayout->addWidget(m_buildButton, 1, 1);
	rightWidgetLayout->addWidget(m_cancelButton, 1, 2);

	rightWidgetLayout->setColumnStretch(0, 1);

	m_rightSideWidget->setLayout(rightWidgetLayout);

	// Left Side
	//
	m_settingsWidget = new QWidget(m_vsplitter);
	QVBoxLayout* settingsWidgetLayout = new QVBoxLayout();

	m_debugCheckBox = new QCheckBox(tr("Debug build"), m_settingsWidget);
	m_debugCheckBox->setChecked(true);

	settingsWidgetLayout->addWidget(m_debugCheckBox);

	m_settingsWidget->setLayout(settingsWidgetLayout);

	// V Splitter
	//
	m_vsplitter = new QSplitter(this);

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
	connect(dbController(), &DbController::projectOpened, this, &BuildTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &BuildTabPage::projectClosed);

	connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::build);
	connect(m_cancelButton, &QAbstractButton::clicked, this, &BuildTabPage::cancel);

	connect(&m_builder, &Builder::Builder::buildStarted, this, &BuildTabPage::buildWasStarted);
	connect(&m_builder, &Builder::Builder::buildFinished, this, &BuildTabPage::buildWasFinished);

	//connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::buildStarted);	// On button clicked event!!!
	connect(&m_builder, &Builder::Builder::buildFinished, this, &BuildTabPage::buildFinished);

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
//	BuildTabPage::m_this = nullptr;

	theSettings.m_buildTabPageSplitterState = m_vsplitter->saveState();
	theSettings.writeUserScope();
}

//BuildTabPage* BuildTabPage::instance()
//{
//	assert(m_this);
//	return m_this;
//}

void BuildTabPage::CreateActions()
{
//	m_addSystemAction = new QAction(tr("System"), this);
//	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
//	m_addSystemAction->setEnabled(false);
//	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

	return;
}

void BuildTabPage::writeOutputLog(const OutputLogItem& logItem)
{
	if (m_outputWidget == nullptr)
	{
		assert(m_outputWidget != nullptr);
		return;
	}

	QString html = logItem.toHtml();
	m_outputWidget->append(html);

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
		std::list<OutputLogItem> messages;

		for (int i = 0; i < 10 && m_outputLog.isEmpty() == false; i++)
		{
			messages.push_back(m_outputLog.popMessages());
		}

		for (auto m = messages.begin(); m != messages.end(); ++m)
		{
			writeOutputLog(*m);
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
		LOG_WARNING((&m_outputLog), tr("Cannot open output log file (%1) for writing").arg(logFileName));
	}

	// --
	//
	emit buildStarted();

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
}

