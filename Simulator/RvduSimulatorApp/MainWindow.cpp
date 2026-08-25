#include "MainWindow.h"
#include <RvduSimulatorLib.h>
#include "RvduSimThread.h"
#include "RvduWidget.h"
#include "SignalsView.h"

#include <QTimer>
#include <QSettings>

#include <UiLib/DialogAbout.h>

static const int m_logFileCount = 9999;
static const int m_logFileMaxSize = 1048576;
static int m_logFileIndex = 0;
#define STRING_MAX_LEN 256

static void writeLogFileMessage(int code, const char* message)
{
	Q_UNUSED(code);

	FILE* logFile = NULL;

	do
	{
		char logFileName[STRING_MAX_LEN];
		sprintf(logFileName, "RvduSimulatorApp%04d.log", m_logFileIndex);

		logFile = fopen(logFileName, "a");
		if (logFile == NULL)
		{
			return;
		}

		long size = ftell(logFile); // Get current position (size in bytes)
		if (size < m_logFileMaxSize)
		{
			break;
		}

		// Open next file
		//
		fclose(logFile);
		logFile = NULL;

		m_logFileIndex++;
	} while (m_logFileIndex < m_logFileCount);


	if (logFile != NULL)
	{
		// Get current time with nanosecond precision
		struct timespec ts;
		(void)timespec_get(&ts, TIME_UTC);

		struct tm* t = localtime(&ts.tv_sec);

		char text[STRING_MAX_LEN];
		snprintf(text,
				 STRING_MAX_LEN,
				 "%02d.%02d.%04d %02d.%02d.%02d.%03d\t%s\n",
				 t->tm_mday,
				 t->tm_mon + 1u,
				 t->tm_year + 1900u,
				 t->tm_hour,
				 t->tm_min,
				 t->tm_sec,
				 ts.tv_nsec / 1000000u,
				 message);

		fputs(text, logFile);

		fclose(logFile);
	}
}

MainWindow::MainWindow(QString buildFolder, int subsystemKey, QWidget* parent) :
	QMainWindow(parent),
	m_buildFolder(buildFolder), 
	m_subsystemKey(subsystemKey)
{
	// Allocate video memory
	//

	m_pHdmiBuffer = new unsigned int[m_width * m_height]; // 4 bytes per pixel
	memset(m_pHdmiBuffer, 0, m_width * m_height * sizeof(unsigned int));

	setupUi();
	
	restoreGeometry(QSettings().value("MainWindow/geometry").toByteArray());

	rnLibSetLogRecordHandler(writeLogFileMessage);

	// Set window caption
	//
	setWindowTitle(tr("RVDU - Build path: %1, Key: %2").arg(m_buildFolder).arg(m_subsystemKey));

	// Create controller
	//
	m_controller = new RvduSimThreadController(this, m_pHdmiBuffer, m_buildFolder, m_subsystemKey, m_width, m_height);

	// Create timer
	//
	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimer);
	m_timer->start(50);

	writeLogFileMessage(RnLibMessage, "Simulator started.");
}

MainWindow::~MainWindow()
{
	// Save window size
	QSettings settings;
	settings.setValue("MainWindow/geometry", saveGeometry());

	delete m_controller;

	qDebug() << "delete hdmi buffer";
	delete[] m_pHdmiBuffer;

	writeLogFileMessage(RnLibMessage, "Simulator finished.");
}

void MainWindow::onTimer()
{
	if (m_controller->isError() == true) 
	{
		m_rvduWidget->setErrorMode(true, tr("RVDU build loading error, please check the log file."));
		m_rvduWidget->update();
	}
	else
	{
		if (m_controller->isRunning() == true)
		{
			m_rvduWidget->setRunningMode(true);
			m_rvduWidget->update();
		}
	}
}

void MainWindow::onSignals()
{
	SignalsView* dialogView = SignalsView::create(this);
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

	m_rvduWidget = new RvduWidget(m_pHdmiBuffer, m_width, m_height, this);
	m_rvduWidget->setFixedSize(m_width, m_height);
	m_rvduWidget->setFocusPolicy(Qt::StrongFocus);

	QWidget* w = new QWidget();
	
	QHBoxLayout* l = new QHBoxLayout(w);
	l->addWidget(m_rvduWidget);

	setCentralWidget(w);
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
}