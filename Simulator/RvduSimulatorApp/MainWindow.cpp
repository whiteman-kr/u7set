#include "MainWindow.h"
#include <RvduSimulatorLib.h>
#include "RvduSimThread.h"
#include "SimulatorCommThread.h"
#include "RvduWidget.h"
#include "SignalsView.h"

#include <QTimer>
#include <QSettings>
#include <QStandardPaths>

#include <UiLib/DialogAbout.h>
#include <UiLib/LogDialog.h>

static const int m_logFileCount = 9999;
static const int m_logFileMaxSize = 1048576;
static int m_logFileIndex = 0;
#define STRING_MAX_LEN 256

static Log::LogFile* theLogFile = nullptr;

MainWindow::MainWindow(QString buildFolder, int subsystemKey, QString sharedMemoryKey, QWidget* parent) :
	QMainWindow(parent),
	m_buildFolder(buildFolder), 
	m_subsystemKey(subsystemKey),
	m_sharedMemoryKey(sharedMemoryKey),
	m_logFile{qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + QString::number(subsystemKey)}
{

	theLogFile = &m_logFile;

	// Allocate video memory
	//

	m_pHdmiBuffer = new unsigned int[m_width * m_height]; // 4 bytes per pixel
	memset(m_pHdmiBuffer, 0, m_width * m_height * sizeof(unsigned int));

	setupUi();
	
	restoreGeometry(QSettings().value("MainWindow/geometry").toByteArray());

	rnLibSetLogRecordHandler(MainWindow::writeLogFileMessage);

	// Set window caption
	//
	setWindowTitle(tr("RVDU - Build path: %1, Key: %2").arg(m_buildFolder).arg(m_subsystemKey));

	setupRvduController();

	// Create timer
	//
	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimer);
	m_timer->start(50);

	m_logFile.writeMessage("Simulator started.");
}

MainWindow::~MainWindow()
{
	// Save window size
	QSettings settings;
	settings.setValue("MainWindow/geometry", saveGeometry());

	theLogFile = nullptr;
	
	if (m_sharedMemController != NULL)
	{
		delete m_sharedMemController;
		m_sharedMemController = NULL;
	}

	if (m_rvduController != NULL)
	{
		delete m_rvduController;
		m_rvduController = NULL;
	}

	qDebug() << "delete hdmi buffer";
	delete[] m_pHdmiBuffer;

	m_logFile.writeMessage("Simulator finished.");
}

void MainWindow::onTimer()
{
	if (m_rvduController->isError() == true) 
	{
		m_rvduWidget->setErrorMode(true, tr("RVDU build loading error, please check the log file."));
		m_rvduWidget->update();
	}
	else
	{
		if (m_rvduController->isRunning() == true)
		{
			m_rvduWidget->setRunningMode(true);
			m_rvduWidget->update();
		}
	}

	updateStatusBar();
}

void MainWindow::onSignals()
{
	SignalsView* dialogView = SignalsView::create(this);
	Q_UNUSED(dialogView);
}

void MainWindow::onShowAppLog()
{ 
	Log::LogFileDialog::view(m_logFile, this); 
}

void MainWindow::onShowAbout()
{
	QString text = qApp->applicationName() + tr(" provides RVDU simuulation.");
	UiLib::DialogAbout::show(this, text, {});
	return;
}

void MainWindow::setupUi()
{
	Q_ASSERT(m_pHdmiBuffer != NULL);

	createMenu();
	createStatusBar();

	m_rvduWidget = new RvduWidget(m_pHdmiBuffer, m_width, m_height, this);
	m_rvduWidget->setFixedSize(m_width, m_height);
	m_rvduWidget->setFocusPolicy(Qt::StrongFocus);

	QWidget* w = new QWidget();
	
	QHBoxLayout* l = new QHBoxLayout(w);
	l->addWidget(m_rvduWidget);

	setCentralWidget(w);
}

void MainWindow::setupRvduController() 
{
	// Create controller
	//
	m_rvduController = new RvduSimThreadController(this, m_pHdmiBuffer, m_buildFolder, m_subsystemKey, m_width, m_height);

	connect(m_rvduController,
			&RvduSimThreadController::logError,
			this,
			[this](const QString& msg)
			{
				m_logFile.writeError(msg);
			});
	connect(m_rvduController,
			&RvduSimThreadController::logWarning,
			this,
			[this](const QString& msg)
			{
				m_logFile.writeWarning(msg);
			});
	connect(m_rvduController,
			&RvduSimThreadController::logMessage,
			this,
			[this](const QString& msg)
			{
				m_logFile.writeMessage(msg);
			});

	connect(m_rvduController,
			&RvduSimThreadController::rnInitComplete,
			this,
			[this]()
			{
				// When the simulator is started, create a shared memory communication thread
				//
				if (m_sharedMemoryKey.isEmpty() == false)
				{
					setupSharedMemController();
				}
				else
				{
					m_statusBarSharedDataInfo->setText("Autonomous Mode");
				}
			});

	connect(m_rvduController,
			&RvduSimThreadController::updateProjectInfo,
			this,
			[this](const QString& projectInfo)
			{
				m_statusBarProjectInfo->setText(projectInfo);
			});
}


void MainWindow::setupSharedMemController()
{
	// Create controller
	//
	m_sharedMemController =
		new SimCommThreadController(this, m_sharedMemoryKey);

	connect(m_sharedMemController,
			&SimCommThreadController::logError,
			this,
			[this](const QString& msg)
			{
				m_logFile.writeError(msg);
			});
	connect(m_sharedMemController,
			&SimCommThreadController::logWarning,
			this,
			[this](const QString& msg)
			{
				m_logFile.writeWarning(msg);
			});
	connect(m_sharedMemController,
			&SimCommThreadController::logMessage,
			this,
			[this](const QString& msg)
			{
				m_logFile.writeMessage(msg);
			});

	connect(m_sharedMemController,
			&SimCommThreadController::updateSharedDataInfo,
			this,
			[this](const QString& info, bool timeout)
			{
				m_statusBarSharedDataInfo->setText(info);
				if (timeout == false)
				{
					m_statusBarSharedDataInfo->setStyleSheet(m_statusBarProjectInfo->styleSheet());
				}
				else
				{
					m_statusBarSharedDataInfo->setStyleSheet(QString("QLabel {color : white; background-color: #C00000}"));
				}
			});
}

void MainWindow::createMenu() 
{
	auto fileMenu = menuBar()->addMenu(tr("&File"));

	fileMenu->addSeparator();

	QAction* a = new QAction(tr("Exit"), this);
	connect(a, &QAction::triggered, this, &QMainWindow::close);
	fileMenu->addAction(a);

	auto signalsMenu = menuBar()->addMenu(tr("&Signals"));

	QAction* signalsView = new QAction(tr("Signals View"), this);
	connect(signalsView, &QAction::triggered, this, &MainWindow::onSignals);
	signalsMenu->addAction(signalsView);

	auto helpMenu = menuBar()->addMenu(tr("&?"));

	a = new QAction(tr("About..."), this);
	connect(a, &QAction::triggered, this, &MainWindow::onShowAbout);
	helpMenu->addAction(a);

	a = new QAction(tr("Application Log..."), this);
	a->setStatusTip(tr("Show application log"));
	connect(a, &QAction::triggered, this, &MainWindow::onShowAppLog);
	helpMenu->addAction(a);
}

void MainWindow::createStatusBar()
{
	m_statusBarProjectInfo = new QLabel();
	m_statusBarProjectInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_statusBarSharedDataInfo = new QLabel();
	m_statusBarSharedDataInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_statusBarLogAlerts = new QLabel();
	m_statusBarLogAlerts->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->installEventFilter(this);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));

	// --
	//
	statusBar()->addPermanentWidget(m_statusBarProjectInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarSharedDataInfo, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);
}

void MainWindow::updateStatusBar()
{
	// Status bar
	//
	Q_ASSERT(m_statusBarLogAlerts);


	// Log status
	//
	int errorsCounter = m_logFile.errorAckCounter();
	int warningsCounter = m_logFile.warningAckCounter();

	static int m_logErrorsCounter = -1;
	static int m_logWarningsCounter = -1;

	if (m_logErrorsCounter != errorsCounter || m_logWarningsCounter != warningsCounter)
	{
		m_logErrorsCounter = errorsCounter;
		m_logWarningsCounter = warningsCounter;

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(QString(tr(" Log E: %1 W: %2 ")).arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
			m_statusBarLogAlerts->setStyleSheet(m_statusBarProjectInfo->styleSheet());
		}
		else
		{
			if (m_logErrorsCounter == 0)
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #F87217}");
			}
			else
			{
				m_statusBarLogAlerts->setStyleSheet(QString("QLabel {color : white; background-color: #C00000}"));
			}
		}
	}
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_statusBarLogAlerts && m_statusBarLogAlerts->text().isEmpty() == false && event->type() == QEvent::MouseButtonPress)
	{
		onShowAppLog();
	}

	return QWidget::eventFilter(object, event);
}

void MainWindow::writeLogFileMessage(int code, const char* message)
{
	if (theLogFile == nullptr)
	{
		return;
	}

	switch (code)
	{
	case RnLibError:
		theLogFile->writeError(QString::fromUtf8(message));
		break;
	case RnLibWarning:
		theLogFile->writeWarning(QString::fromUtf8(message));
		break;
	case RnLibMessage:
		theLogFile->writeMessage(QString::fromUtf8(message));
		break;
	}
}
