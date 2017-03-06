#include "Main.h"
#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "Settings.h"
#include "DialogSettings.h"
#include "TuningFilter.h"
#include "DialogTuningSources.h"
#include "DialogUsers.h"
#include "TuningClientFilterEditor.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent) :
	m_configController(this, theSettings.configuratorAddress1(), theSettings.configuratorAddress2()),
	QMainWindow(parent)
{
	if (theSettings.m_mainWindowPos.x() != -1 && theSettings.m_mainWindowPos.y() != -1)
	{
		move(theSettings.m_mainWindowPos);
		restoreGeometry(theSettings.m_mainWindowGeometry);
		restoreState(theSettings.m_mainWindowState);
	}
	else
	{
		resize(1024, 768);
	}

    theLogFile = new LogFile("TuningClient", QDir::toNativeSeparators(theSettings.localAppDataPath()));

    theLogFile->write("--");
    theLogFile->write("-----------------------");
    theLogFile->write("--");
    theLogFile->writeMessage(tr("Application started."));

	createActions();
	createMenu();
	createStatusBar();

    setWindowTitle(QString("TuningClient - ") + theSettings.instanceStrId());

    setCentralWidget(new QLabel(tr("Waiting for configuration...")));

	// TcpSignalClient
	//
	HostAddressPort fakeAddress(QLatin1String("0.0.0.0"), 0);
    theObjectManager = new TuningObjectManager(&m_configController, fakeAddress, fakeAddress);

    m_tcpClientThread = new SimpleThread(theObjectManager);
	m_tcpClientThread->start();

    connect(theObjectManager, &TuningObjectManager::tuningSourcesArrived, this, &MainWindow::slot_tuningSourcesArrived);
    connect(theObjectManager, &TuningObjectManager::connectionFailed, this, &MainWindow::slot_tuningConnectionFailed);

	QString errorCode;

    if (theFilters.load(theSettings.userFiltersFile(), &errorCode, false) == false)
	{
		QString msg = tr("Failed to load user filters: %1").arg(errorCode);

        theLogFile->writeError(msg);
        QMessageBox::critical(this, tr("Error"), msg);
	}

	//

    m_mainWindowTimerId = startTimer(100);

	connect(&m_configController, &ConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);
	m_configController.start();
}

MainWindow::~MainWindow()
{

	m_tcpClientThread->quitAndWait(10000);
	delete m_tcpClientThread;

	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

    theLogFile->writeMessage(tr("Application finished."));

    delete theLogFile;
}


void MainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	//m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MainWindow::exit);


	m_pPresetEditorAction = new QAction(tr("Preset Editor..."), this);
	m_pPresetEditorAction->setStatusTip(tr("Edit user presets"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pPresetEditorAction->setEnabled(true);
    connect(m_pPresetEditorAction, &QAction::triggered, this, &MainWindow::runPresetEditor);

	m_pUsersAction = new QAction(tr("Users..."), this);
	m_pUsersAction->setStatusTip(tr("Edit users"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pUsersAction->setEnabled(true);
	connect(m_pUsersAction, &QAction::triggered, this, &MainWindow::runUsersEditor);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

	m_pTuningSourcesAction = new QAction(tr("Tuning sources..."), this);
	m_pTuningSourcesAction->setStatusTip(tr("View tuning sources"));
	//m_pTuningSourcesAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pTuningSourcesAction->setEnabled(true);
	connect(m_pTuningSourcesAction, &QAction::triggered, this, &MainWindow::showTuningSources);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	//m_pLogAction->setEnabled(false);
	//connect(m_pLogAction, &QAction::triggered, this, &MonitorMainWindow::showLog);

	m_pAboutAction = new QAction(tr("About..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
    connect(m_pAboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenu()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_pPresetEditorAction);

    QMenu* pServiceMenu = menuBar()->addMenu(tr("&Service"));
    pServiceMenu->addAction(m_pTuningSourcesAction);
    pServiceMenu->addAction(m_pUsersAction);
    pServiceMenu->addAction(m_pSettingsAction);

	// Help
	//
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_pLogAction);
	pHelpMenu->addAction(m_pAboutAction);
}

void MainWindow::createStatusBar()
{
	m_statusBarInfo = new QLabel();
	m_statusBarInfo->setAlignment(Qt::AlignLeft);
	m_statusBarInfo->setIndent(3);

	m_statusBarConnectionStatistics = new QLabel();
	m_statusBarConnectionStatistics->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionStatistics->setMinimumWidth(100);

	m_statusBarConnectionState = new QLabel();
	m_statusBarConnectionState->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionState->setMinimumWidth(100);

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConnectionStatistics, 0);
	statusBar()->addPermanentWidget(m_statusBarConnectionState, 0);
}

void MainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);

	// Update status bar
	//
    if  (event->timerId() == m_mainWindowTimerId)
	{
        if (theSharedMemorySingleApp != nullptr)
        {
            bool ok = theSharedMemorySingleApp->lock();
            if (ok == true)
            {
                TuningClientSharedData* data = (TuningClientSharedData*)theSharedMemorySingleApp->data();

                if (data->showCommand == true)
                {
                    data->showCommand = false;

                    showMinimized(); // This is to bring up the window if not minimized but beneath some other window
                    setWindowState(Qt::WindowActive);
                    showNormal();
                }


                ok = theSharedMemorySingleApp->unlock();
                if (ok == false)
                {
                    qDebug()<<"Failed to unlock QSharedMemory object!";
                    assert(false);
                }
            }
            else
            {
                qDebug()<<"Failed to lock QSharedMemory object!";
                assert(false);
            }
        }


        if (theObjectManager != nullptr)
        {
            assert(m_statusBarConnectionState);
            assert(m_statusBarConnectionStatistics);

            Tcp::ConnectionState confiConnState =  m_configController.getConnectionState();
            Tcp::ConnectionState tuningClientState =  theObjectManager->getConnectionState();

            // State
            //
            QString text = tr(" ConfigSrv: %1   TuningSrv: %2 ")
					.arg(confiConnState.isConnected ? confiConnState.peerAddr.addressStr() :tr("NoConnection"))
					.arg(tuningClientState.isConnected ? tuningClientState.peerAddr.addressStr() : tr("NoConnection"));

            m_statusBarConnectionState->setText(text);

            // Statistics
            //
            text = tr(" ConfigSrv: %1   TuningSrv: %2 ")
                    .arg(QString::number(confiConnState.replyCount))
                    .arg(QString::number(tuningClientState.replyCount));

            m_statusBarConnectionStatistics->setText(text);
        }

		return;
	}

	return;
}

void MainWindow::createWorkspace(const TuningObjectStorage *objects)
{
    if (m_tuningWorkspace != nullptr)
    {
        delete m_tuningWorkspace;
        m_tuningWorkspace = nullptr;
    }

    m_tuningWorkspace = new TuningWorkspace(objects, this);

    setCentralWidget(m_tuningWorkspace);

}

void MainWindow::slot_configurationArrived(ConfigSettings settings)
{
	if (settings.updateFilters == false && settings.updateSignals == false && settings.updateSchemas == false)
	{
		return;
	}

	if (settings.updateFilters == true)
	{
        theFilters.removeAutomaticFilters();

        if (m_configController.getObjectFilters() == false)
        {

        }
    }

	if (settings.updateSchemas == true)
	{
		if (m_configController.getSchemasDetails() == false)
		{

		}
    }

    if (settings.updateSignals == true)
    {
        if (m_configController.getTuningSignals() == false)
        {

        }
        emit signalsUpdated();
    }

    TuningObjectStorage objects = theObjectManager->objectStorage();

    theFilters.createAutomaticFilters(&objects, theSettings.filterBySchema(), theSettings.filterByEquipment(), theObjectManager->tuningSourcesEquipmentIds());

    // Find and possibly remove non-existing signals from the list

    if (settings.updateFilters == true || settings.updateSignals == true)
    {

        bool removedNotFound = false;

        theFilters.checkSignals(&objects, removedNotFound, this);

        if (removedNotFound == true)
        {
            QString errorMsg;

            if (theFilters.save(theSettings.userFiltersFile(), &errorMsg) == false)
            {
                theLogFile->writeError(errorMsg);
                QMessageBox::critical(this, tr("Error"), errorMsg);
            }
        }
    }

    if (m_tuningWorkspace != nullptr)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Program configuration has been changed and will be updated."));
    }

    createWorkspace(&objects);

    return;
}

void MainWindow::slot_tuningSourcesArrived()
{

}

void MainWindow::slot_tuningConnectionFailed()
{

}

void MainWindow::slot_presetsEditorClosing(std::vector <int>& signalsTableColumnWidth, std::vector <int>& presetsTreeColumnWidth, QPoint pos, QByteArray geometry)
{
    theSettings.m_presetEditorSignalsTableColumnWidth = signalsTableColumnWidth;
    theSettings.m_presetEditorPresetsTreeColumnWidth = presetsTreeColumnWidth;
    theSettings.m_presetEditorPos = pos;
    theSettings.m_presetEditorGeometry = geometry;
}

void MainWindow::exit()
{
	close();
}

void MainWindow::runPresetEditor()
{
    if (theUserManager.requestPassword(this, false) == false)
    {
        return;
    }

    TuningFilterStorage editStorage = theFilters;

    bool editAutomatic = false;

    TuningObjectStorage objects = theObjectManager->objectStorage();

    TuningClientFilterEditor d(&editStorage, &objects, editAutomatic,
                         theSettings.m_presetEditorSignalsTableColumnWidth,
                         theSettings.m_presetEditorPresetsTreeColumnWidth,
                         theSettings.m_presetEditorPos,
                         theSettings.m_presetEditorGeometry,
                         this);

    connect(&d, &TuningFilterEditor::editorClosing, this, &MainWindow::slot_presetsEditorClosing);
    connect(this, &MainWindow::signalsUpdated, &d, &TuningFilterEditor::slot_signalsUpdated);

    if (d.exec() == QDialog::Accepted)
    {
        theFilters = editStorage;

        QString errorMsg;

        if (theFilters.save(theSettings.userFiltersFile(), &errorMsg) == false)
        {
            theLogFile->writeError(errorMsg);
            QMessageBox::critical(this, tr("Error"), errorMsg);
        }

        createWorkspace(&objects);
    }
}

void MainWindow::runUsersEditor()
{
    if (theUserManager.requestPassword(this, true) == false)
    {
        return;
    }

	DialogUsers d(theUserManager, this);
	if (d.exec() == QDialog::Accepted && theSettings.admin() == true)
	{
		theUserManager = d.m_userManager;
		theUserManager.Store();
	}
}

void MainWindow::showSettings()
{
    if (theUserManager.requestPassword(this, true) == false)
    {
        return;
    }

    DialogSettings* d = new DialogSettings(this);

    d->exec();

    if (d->filterSettingsChanged() == true)
    {
        TuningObjectStorage objects = theObjectManager->objectStorage();

        theFilters.removeAutomaticFilters();

        theFilters.createAutomaticFilters(&objects, theSettings.filterBySchema(), theSettings.filterByEquipment(), theObjectManager->tuningSourcesEquipmentIds());

        createWorkspace(&objects);
    }

    delete d;
}


void MainWindow::showTuningSources()
{
	if (theDialogTuningSources == nullptr)
	{
		theDialogTuningSources = new DialogTuningSources(this);
		theDialogTuningSources->show();
	}
	else
	{
		theDialogTuningSources->activateWindow();
	}
}

void MainWindow::showAbout()
{
    QDialog aboutDialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    QHBoxLayout* hl = new QHBoxLayout;

    /*QLabel* logo = new QLabel(&aboutDialog);
    logo->setPixmap(QPixmap(":/Images/Images/logo.png"));

    hl->addWidget(logo);*/

    QVBoxLayout* vl = new QVBoxLayout;
    hl->addLayout(vl);

    QString text = "<h3>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h3>";
#ifndef Q_DEBUG
    text += "Build: Release";
#else
    text += "Build: Debug";
#endif
    text += "<br>Commit SHA1: " USED_SERVER_COMMIT_SHA;

    QLabel* label = new QLabel(text, &aboutDialog);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    vl->addWidget(label);

    label = new QLabel(&aboutDialog);
    label->setText(qApp->applicationName() + " allows user to modify tuning values.");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    label->setWordWrap(true);
    vl->addWidget(label);

    QPushButton* copyCommitSHA1Button = new QPushButton("Copy commit SHA1");
    connect(copyCommitSHA1Button, &QPushButton::clicked, [](){
        qApp->clipboard()->setText(USED_SERVER_COMMIT_SHA);
    });

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(copyCommitSHA1Button, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hl);
    mainLayout->addWidget(buttonBox);
    aboutDialog.setLayout(mainLayout);

    connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog, &QDialog::accept);

    aboutDialog.exec();
}

MainWindow* theMainWindow = nullptr;
LogFile* theLogFile = nullptr;

TuningObjectManager* theObjectManager = nullptr;

TuningFilterStorage theFilters;

UserManager theUserManager;
