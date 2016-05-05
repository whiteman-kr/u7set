#include "Stable.h"
#include "UploadTabPage.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../include/DbController.h"


//
//
// UploadTabPage
//
//
//UploadTabPage* UploadTabPage::m_this = nullptr;
//const char* UploadTabPage::m_buildLogFileName = "buildlog.html";


UploadTabPage::UploadTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)//,
	//m_builder(&m_outputLog)
{
	assert(dbcontroller != nullptr);

	//	UploadTabPage::m_this = this;

	//
	// Controls
	//

	// Create Actions
	//
	CreateActions();

	// US Widgets
	//
	QLayout* layout = new QHBoxLayout();

	QListWidget* stackedListWidget = new QListWidget();
	stackedListWidget->addItem("Choose Build");
	stackedListWidget->addItem("Upload");

	m_stackedWidget = new QStackedWidget();
	m_stackedWidget->addWidget(new QPushButton("Choose Build"));
	m_stackedWidget->addWidget(new QPushButton("Upload"));

	connect(stackedListWidget, &QListWidget::currentRowChanged, m_stackedWidget, &QStackedWidget::setCurrentIndex);

	// --
	//
	layout->addWidget(stackedListWidget);
	layout->addWidget(m_stackedWidget);

	setLayout(layout);



	// Right Side Widget
	//
//	m_rightSideWidget = new QWidget();

//	// Output windows
//	//
//	m_outputWidget = new QTextEdit();
//	m_outputWidget->setReadOnly(true);
//	m_outputWidget->document()->setUndoRedoEnabled(false);
//	m_outputWidget->document()->setMaximumBlockCount(600);

//	m_buildButton = new QPushButton(tr("Build..."));
//	m_cancelButton = new QPushButton(tr("Cancel"));

//	QGridLayout* rightWidgetLayout = new QGridLayout();

//	rightWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 3);
//	rightWidgetLayout->addWidget(m_buildButton, 1, 1);
//	rightWidgetLayout->addWidget(m_cancelButton, 1, 2);

//	rightWidgetLayout->setColumnStretch(0, 1);

//	m_rightSideWidget->setLayout(rightWidgetLayout);

//	// Left Side
//	//
//	m_vsplitter = new QSplitter(this);

//	m_settingsWidget = new QWidget(m_vsplitter);
//	QVBoxLayout* settingsWidgetLayout = new QVBoxLayout();

//	m_debugCheckBox = new QCheckBox(tr("Debug build"), m_settingsWidget);
//	m_debugCheckBox->setChecked(true);

//	settingsWidgetLayout->addWidget(m_debugCheckBox);

//	m_settingsWidget->setLayout(settingsWidgetLayout);

//	// V Splitter
//	//
//	m_vsplitter->addWidget(m_settingsWidget);
//	m_vsplitter->addWidget(m_rightSideWidget);

//	m_vsplitter->setStretchFactor(0, 2);
//	m_vsplitter->setStretchFactor(1, 1);

//	m_vsplitter->restoreState(theSettings.m_UploadTabPageSplitterState);

//	//
//	// Layouts
//	//
//	QHBoxLayout* pMainLayout = new QHBoxLayout();

//	pMainLayout->addWidget(m_vsplitter);

//	setLayout(pMainLayout);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &UploadTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &UploadTabPage::projectClosed);

//	connect(m_buildButton, &QAbstractButton::clicked, this, &UploadTabPage::upload);
//	connect(m_cancelButton, &QAbstractButton::clicked, this, &UploadTabPage::cancel);

//	connect(GlobalMessanger::instance(), &GlobalMessanger::buildStarted, this, &UploadTabPage::uploadWasStarted);
//	connect(GlobalMessanger::instance(), &GlobalMessanger::buildFinished, this, &UploadTabPage::uploadWasFinished);

	////connect(m_buildButton, &QAbstractButton::clicked, this, &UploadTabPage::buildStarted);	// On button clicked event!!!
	//connect(&m_builder, &Builder::Builder::buildFinished, this, &UploadTabPage::buildFinished);

	// Output Log
	//
	//m_logTimerId = startTimer(10);

	//m_outputLog.setHtmlFont("Verdana");

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

UploadTabPage::~UploadTabPage()
{
//	UploadTabPage::m_this = nullptr;

	//theSettings.m_UploadTabPageSplitterState = m_vsplitter->saveState();
	//theSettings.writeUserScope();
}

//bool UploadTabPage::isUploadingNow() const
//{
//	return m_builder.isRunning();
//}

//const std::map<QUuid, OutputMessageLevel>* UploadTabPage::itemsIssues() const
//{
//	return &m_itemsIssues;
//}

//void UploadTabPage::cancelUpload()
//{
//	if (m_builder.isRunning() == true)
//	{
//		m_builder.stop();

//		// wait for 20 seconds while bild stops
//		//
//		for (int i = 0; i < 2000 && m_builder.isRunning() == true; i++)
//		{
//			QThread::msleep(10);
//		}

//		if (m_builder.isRunning() == true)
//		{
//			qDebug() << "WARNING: Exit while the build thread is still running!";
//		}
//	}
//}

//UploadTabPage* UploadTabPage::instance()
//{
//	assert(m_this);
//	return m_this;
//}

void UploadTabPage::CreateActions()
{
//	m_addSystemAction = new QAction(tr("System"), this);
//	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
//	m_addSystemAction->setEnabled(false);
//	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

	return;
}

//void UploadTabPage::writeOutputLog(const OutputLogItem& logItem)
//{
//	if (m_outputWidget == nullptr)
//	{
//		assert(m_outputWidget != nullptr);
//		return;
//	}

//	QString html = logItem.toHtml();
//	m_outputWidget->append(html);

//	return;
//}

void UploadTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

//void UploadTabPage::timerEvent(QTimerEvent* event)
//{

//	if (event->timerId() == m_logTimerId &&
//		m_outputLog.isEmpty() == false &&
//		m_outputWidget != nullptr)
//	{
//		std::list<OutputLogItem> messages;

//		for (int i = 0; i < 10 && m_outputLog.isEmpty() == false; i++)
//		{
//			messages.push_back(m_outputLog.popMessages());
//		}

//		for (auto m = messages.begin(); m != messages.end(); ++m)
//		{
//			writeOutputLog(*m);
//		}

//		return;
//	}

//	return;
//}

void UploadTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void UploadTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

//void UploadTabPage::upload()
//{
//	m_outputLog.clear();
//	m_outputWidget->clear();

//	// init build log file
//	//
//    QString logFileName = QDir::fromNativeSeparators(theSettings.buildOutputPath());

//	if (logFileName.endsWith('/') == false)
//	{
//		logFileName += '/';
//	}
//	logFileName += m_buildLogFileName;

//	if (m_logFile.isOpen() == true && m_logFile.fileName() == logFileName)
//	{
//		// it is ok, file is open and has right name
//		//
//	}
//	else
//	{
//		if (m_logFile.isOpen())
//		{
//			m_logFile.close();
//		}

//		m_logFile.setFileName(logFileName);
//		m_logFile.open(QIODevice::Append | QIODevice::Text);
//	}

//	if (m_logFile.isOpen() == true)
//	{
//		LOG_MESSAGE((&m_outputLog), tr("Build log file: %1").arg(logFileName));

//	}
//	else
//	{
//		LOG_WARNING_OBSOLETE((&m_outputLog), Builder::IssueType::NotDefined,  tr("Cannot open output log file (%1) for writing").arg(logFileName));
//	}

//	// --
//	//
//	GlobalMessanger::instance()->fireBuildStarted();

//	bool debug = m_debugCheckBox->isChecked();

//	m_builder.start(
//		db()->currentProject().projectName(),
//		db()->host(),
//		db()->port(),
//		db()->serverUsername(),
//		db()->serverPassword(),
//		db()->currentUser().username(),
//		db()->currentUser().password(),
//		debug);

//	return;
//}

//void UploadTabPage::cancel()
//{
//	m_builder.stop();
//}

//void UploadTabPage::uploadWasStarted()
//{
//	m_buildButton->setEnabled(false);
//	m_cancelButton->setEnabled(true);
//}

//void UploadTabPage::uploadWasFinished()
//{
//	m_buildButton->setEnabled(true);
//	m_cancelButton->setEnabled(false);

//	m_itemsIssues.clear();

//	return;
//}

