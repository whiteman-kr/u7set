#include "CalibratorBase.h"

#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QKeyEvent>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

CalibratorBase theCalibratorBase;

// -------------------------------------------------------------------------------------------------------------------

CalibratorBase::CalibratorBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorBase::~CalibratorBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::init(QWidget* parent)
{
	createCalibrators(parent);			// create objects of calibrators
	createInitDialog(parent);			// create dialog initialization

	setHeaderList();					// init calibrator list

	connect(&m_timer, &QTimer::timeout, this, &CalibratorBase::timeoutInitialization, Qt::QueuedConnection);

	emit calibratorOpen();				// open all calibratirs
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createCalibrators(QWidget* parent)
{
	for(int channel = 0; channel < Metrology::ChannelCount; channel++)
	{
		Calibrator* calibrator = new Calibrator(channel);
		if (calibrator == nullptr)
		{
			continue;
		}

		CalibratorManager* manager = new CalibratorManager(calibrator, parent);
		if (manager == nullptr)
		{
			continue;
		}

		m_mutex.lock();
			m_calibratorManagerList.append(manager);
		m_mutex.unlock();

		QThread *pThread = new QThread;
		if (pThread != nullptr)
		{
			calibrator->moveToThread(pThread);

			connect(pThread, &QThread::finished, calibrator, &Calibrator::deleteLater);

			pThread->start();
		}

		connect(this, &CalibratorBase::calibratorOpen, calibrator, &Calibrator::open, Qt::QueuedConnection);
		connect(this, &CalibratorBase::calibratorClose, calibrator, &Calibrator::close, Qt::QueuedConnection);
		connect(calibrator, &Calibrator::connected, this, &CalibratorBase::onCalibratorConnected, Qt::QueuedConnection);
		connect(calibrator, &Calibrator::disconnected, this, &CalibratorBase::onCalibratorDisconnected, Qt::QueuedConnection);

		emit calibrator->disconnected();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::removeCalibrators()
{
	emit calibratorClose();

	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		CalibratorManager* manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		Calibrator* calibrator = manager->calibrator();
		if (calibrator != nullptr)
		{
			calibrator->setWaitResponse(false);

			QThread *pThread = calibrator->thread();
			if (pThread != nullptr)
			{
				pThread->quit();
				pThread->wait();
				pThread->deleteLater();
			}
		}

		delete manager;
	}

	m_mutex.lock();
		m_calibratorManagerList.clear();
	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createInitDialog(QWidget* parent)
{
	m_pInitDialog = new QDialog(parent);
	m_pInitDialog->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	m_pInitDialog->setFixedSize(500, 200);
	m_pInitDialog->setWindowIcon(QIcon(":/icons/Calibrators.png"));
	m_pInitDialog->setWindowTitle(tr("Calibrators initialization"));
	m_pInitDialog->installEventFilter(this);

		m_pMenuBar = new QMenuBar(m_pInitDialog);
		m_pCalibratorMenu = new QMenu(tr("&Calibrators"), m_pInitDialog);

		m_pInitAction = m_pCalibratorMenu->addAction(tr("&Initialization"));
		m_pInitAction->setIcon(QIcon(":/icons/Calibrators.png"));
		m_pInitAction->setShortcut(Qt::CTRL + Qt::Key_I);
		m_pCalibratorMenu->addSeparator();
		m_pManageAction = m_pCalibratorMenu->addAction(tr("&Manage ..."));
		m_pManageAction->setIcon(QIcon(":/icons/Manage.png"));
		m_pManageAction->setShortcut(Qt::CTRL + Qt::Key_M);
		m_pSettingsAction = m_pCalibratorMenu->addAction(tr("&Settings ..."));
		m_pSettingsAction->setIcon(QIcon(":/icons/Settings.png"));
		m_pSettingsAction->setShortcut(Qt::CTRL + Qt::Key_S);
		m_pCopyAction = new QAction(tr("&Copy"), m_pInitDialog);
		m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
		m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

		m_pMenuBar->addMenu(m_pCalibratorMenu);

		connect(m_pInitAction, &QAction::triggered, this, &CalibratorBase::onInitialization);
		connect(m_pManageAction, &QAction::triggered, this, &CalibratorBase::onManage);
		connect(m_pSettingsAction, &QAction::triggered, this, static_cast<void (CalibratorBase::*)()>(&CalibratorBase::onSettings));
		connect(m_pCopyAction, &QAction::triggered, this, &CalibratorBase::onCopy);

		QVBoxLayout *mainLayout = new QVBoxLayout;
		mainLayout->setMenuBar(m_pMenuBar);

		m_pCalibratorView = new QTableWidget(m_pInitDialog);
		m_pCalibratorProgress = new QProgressBar(m_pInitDialog);

		QVBoxLayout *listLayout = new QVBoxLayout;
		listLayout->addWidget(m_pCalibratorView);
		listLayout->addWidget(m_pCalibratorProgress);

		mainLayout->addLayout(listLayout);

	m_pInitDialog->setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::setHeaderList()
{
	// init colomns
	//
	QStringList horizontalHeaderLabels;

	for(int c = 0; c < CALIBRATOR_COLUMN_COUNT; c++)
	{
		horizontalHeaderLabels.append(CalibratorColumn[c]);
	}

	m_pCalibratorView->setColumnCount(CALIBRATOR_COLUMN_COUNT);
	m_pCalibratorView->setHorizontalHeaderLabels(horizontalHeaderLabels);
	m_pCalibratorView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// init rows
	//
	QStringList verticalHeaderLabels;
	m_pCalibratorView->setRowCount(Metrology::ChannelCount);

	for(int channel = 0; channel < Metrology::ChannelCount; channel++)
	{
		verticalHeaderLabels.append(QString("Calibrator %1").arg(channel + 1));
		m_pCalibratorView->setRowHeight(channel, 18);
	}
	m_pCalibratorView->setVerticalHeaderLabels(verticalHeaderLabels);

	m_pCalibratorView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	int count = horizontalHeaderLabels.count();
	for(int column = 0; column < count; column++)
	{
		for(int row = 0; row < Metrology::ChannelCount; row++)
		{
			QTableWidgetItem* item = new QTableWidgetItem(QString());
			item->setTextAlignment(Qt::AlignHCenter);
			m_pCalibratorView->setItem(row, column, item);
		}
	}

	connect(m_pCalibratorView, &QTableWidget::cellDoubleClicked, this, &CalibratorBase::onManage);

	// init context menu
	//
	m_pCalibratorView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pCalibratorView, &QTableWidget::customContextMenuRequested, this, &CalibratorBase::onContextMenu);

	// init progress
	//
	m_pCalibratorProgress->setRange(0, CALIBRATOR_TIMEOUT);
	m_pCalibratorProgress->setValue(0);
	m_pCalibratorProgress->setFixedHeight(10);
	m_pCalibratorProgress->setTextVisible(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::updateList()
{
	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		CalibratorManager* manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		Calibrator* pCalibrator = manager->calibrator();
		if (pCalibrator == nullptr)
		{
			continue;
		}

		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_PORT)->setText(pCalibrator->portName());
		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_TYPE)->setText(pCalibrator->typeStr());
		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_CONNECT)->setText(pCalibrator->isConnected() ? tr("Yes") : tr("No"));
		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_SN)->setText(pCalibrator->serialNo());

		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_CONNECT)->setBackgroundColor(pCalibrator->isConnected() == true ? COLOR_CALIBRATOR_CONNECTED : COLOR_CALIBRATOR_NOT_CONNECTED);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::updateConnectedCalibrators()
{
	m_connectedCalibratorsCount = 0;

	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		CalibratorManager* manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		if (manager->calibratorIsConnected() == true)
		{
			m_connectedCalibratorsCount ++;
		}
	}

	emit calibratorConnectedChanged(m_connectedCalibratorsCount);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::showInitDialog()
{
	updateList();

	m_pInitDialog->show();
	m_pCalibratorProgress->setValue(0);
}

// -------------------------------------------------------------------------------------------------------------------

int CalibratorBase::calibratorCount() const
{
	int count = 0;

	m_mutex.lock();

		count = m_calibratorManagerList.count();

	m_mutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager* CalibratorBase::calibratorManager(int index) const
{
	CalibratorManager* pCalibratorManager = nullptr;

	m_mutex.lock();

		if (index >= 0 || index < m_calibratorManagerList.count())
		{
			pCalibratorManager = m_calibratorManagerList[index];
		}

	m_mutex.unlock();

	return pCalibratorManager;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager* CalibratorBase::firstConnectedCalibrator() const
{
	CalibratorManager* pFirstConnected = nullptr;

	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		CalibratorManager* pManager = calibratorManager(index);
		if (pManager == nullptr)
		{
			continue;
		}

		if (pManager->calibratorIsConnected() == true)
		{
			pFirstConnected = pManager;

			break;
		}
	}

	return pFirstConnected;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager* CalibratorBase::calibratorForMeasure(int index) const
{
	CalibratorManager* pManager = nullptr;

	switch(theOptions.toolBar().measureKind())
	{
		case MEASURE_KIND_ONE:		pManager = firstConnectedCalibrator();	break;	// we need only one - connected;
		case MEASURE_KIND_MULTI:	pManager = calibratorManager(index);	break;
		default:					assert(0);
	}

	return pManager;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onInitialization()
{
	m_pInitDialog->setWindowFlags(Qt::Tool | Qt::WindowSystemMenuHint);
	m_pInitDialog->show();
	m_pInitDialog->activateWindow();

	m_pCalibratorMenu->setEnabled(false);
	m_pCalibratorView->setEnabled(false);

	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		CalibratorManager* manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		Calibrator* calibrator = manager->calibrator();
		if (calibrator == nullptr)
		{
			continue;
		}

		calibrator->setWaitResponse(false);
	}

	emit calibratorClose();									// close all calibratirs serial port
	QThread::msleep(Metrology::ChannelCount * 100);			// wait, until all the serial ports will be closed
	emit calibratorOpen();									// open all calibratirs

	m_timeout = 0;
	m_timer.start(CALIBRATOR_TIMEOUT_STEP);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::timeoutInitialization()
{
	m_timeout += CALIBRATOR_TIMEOUT_STEP;
	m_pCalibratorProgress->setValue(m_timeout);

	if (m_timeout >= CALIBRATOR_TIMEOUT)
	{
		m_timeout = 0;
		m_timer.stop();

		m_pCalibratorMenu->setEnabled(true);
		m_pCalibratorView->setEnabled(true);

		m_pInitDialog->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
		m_pInitDialog->show();
		m_pInitDialog->activateWindow();

		updateList();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onManage()
{
	int index = m_pCalibratorView->currentRow();
	if (index < 0 || index >= calibratorCount())
	{
		QMessageBox::information(m_pInitDialog, m_pInitDialog->windowTitle(), tr("Please, select calibrator for manage!"));
		return;
	}

	CalibratorManager* manager = calibratorManager(index);
	if (manager == nullptr)
	{
		return;
	}

	manager->show();
	manager->activateWindow();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onSettings()
{
	int index = m_pCalibratorView->currentRow();
	if (index < 0 || index >= calibratorCount())
	{
		QMessageBox::information(m_pInitDialog, m_pInitDialog->windowTitle(), tr("Please, select calibrator for edit settings!"));
		return;
	}

	onSettings(index, -1);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onSettings(int row, int)
{
	int index = row;
	if (index < 0 || index >= calibratorCount())
	{
		return;
	}

	CalibratorManager* manager = calibratorManager(index);
	if (manager == nullptr)
	{
		return;
	}

	Calibrator* calibrator = manager->calibrator();
	if (calibrator == nullptr)
	{
		return;
	}

	// create dialog for select serial port and calibrator type
	//
	QDialog* dialog = new QDialog(m_pInitDialog);
	dialog->setWindowFlags(Qt::Drawer);
	dialog->setFixedSize(200, 120);
	m_pInitDialog->setWindowIcon(QIcon(":/icons/Settings.png"));
	dialog->setWindowTitle(tr("Settings calibrator %1").arg(manager->calibratorChannel() + 1));
	dialog->move(m_pInitDialog->geometry().center() - dialog->rect().center());

		// serial port
		//
		QHBoxLayout *portLayout = new QHBoxLayout ;

		QLabel* portLabel = new QLabel;
		portLabel->setText(tr("Serial port:"));

		QComboBox* portCombo = new QComboBox;

		foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
		{
			portCombo->addItem(info.portName());
		}
		portCombo->setEditable(true);
		portCombo->setCurrentText(calibrator->portName());

		portLayout->addWidget(portLabel);
		portLayout->addWidget(portCombo);

		// calibrator type
		//
		QHBoxLayout *typeLayout = new QHBoxLayout ;

		QLabel* typeLabel = new QLabel;
		typeLabel->setText(tr("Calibrator type:"));

		QComboBox* typeCombo = new QComboBox;

		for (int t = 0; t < CALIBRATOR_TYPE_COUNT; t ++)
		{
			typeCombo->addItem(CalibratorType[t]);
		}
		typeCombo->setCurrentIndex(calibrator->type());

		typeLayout->addWidget(typeLabel);
		typeLayout->addWidget(typeCombo);

		// buttons
		//
		QHBoxLayout *buttonLayout = new QHBoxLayout ;

		QPushButton* okButton = new QPushButton(tr("OK"));
		QPushButton* cancelButton = new QPushButton(tr("Cancel"));

		connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
		connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);

		// add layoyt
		//
		QVBoxLayout *mainLayout = new QVBoxLayout ;

		mainLayout->addLayout(portLayout);
		mainLayout->addLayout(typeLayout);
		mainLayout->addStretch();
		mainLayout->addLayout(buttonLayout);

	dialog->setLayout(mainLayout);

	if (dialog->exec() != QDialog::Accepted)
	{
		return;
	}

	calibrator->setPortName(portCombo->currentText());
	calibrator->setType(typeCombo->currentIndex());

	manager->saveSettings(calibrator);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onCopy()
{
	int index = m_pCalibratorView->currentRow();
	if (index < 0 || index >= calibratorCount())
	{
		QMessageBox::information(m_pInitDialog, m_pInitDialog->windowTitle(), tr("Please, select calibrator!"));
		return;
	}

	CalibratorManager* manager = calibratorManager(index);
	if (manager == nullptr)
	{
		return;
	}

	Calibrator* calibrator = manager->calibrator();
	if (calibrator == nullptr)
	{
		return;
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(calibrator->typeStr() + " " + calibrator->serialNo());
}


// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onContextMenu(QPoint)
{
	QMenu *menu = new QMenu(m_pCalibratorView);

	menu->addAction(m_pManageAction);
	menu->addAction(m_pSettingsAction);

	int index = m_pCalibratorView->currentRow();
	if (index >= 0 && index < calibratorCount())
	{
		CalibratorManager* manager = calibratorManager(index);
		if (manager != nullptr)
		{
			Calibrator* calibrator = manager->calibrator();
			if (calibrator != nullptr)
			{
				if (calibrator->isConnected() == true)
				{
					menu->addSeparator();
					menu->addAction(m_pCopyAction);
				}
			}
		}
	}

	menu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onCalibratorConnected()
{
	updateList();
	updateConnectedCalibrators();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onCalibratorDisconnected()
{
	updateList();
	updateConnectedCalibrators();
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorBase::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::HideToParent)
	{
		if (m_timeout != 0)
		{
			m_pInitDialog->show();
			m_pInitDialog->activateWindow();

			event->ignore();
		}
	}

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return)
		{
			if (m_timeout == 0)
			{
				onSettings();
			}
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::clear()
{
	removeCalibrators();
}

// -------------------------------------------------------------------------------------------------------------------

