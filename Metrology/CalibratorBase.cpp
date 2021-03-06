#include "CalibratorBase.h"

#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QKeyEvent>

#include "ProcessData.h"

// -------------------------------------------------------------------------------------------------------------------

CalibratorBase::CalibratorBase(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorBase::~CalibratorBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::clear()
{
	emit calibratorClose();		// close all calibratоrs

	removeCalibrators();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::init(const CalibratorsOption& calibratorsOption, QWidget* parent)
{
	m_calibratorsOption = calibratorsOption;

	createCalibrators(parent);			// create objects of calibrators
	createInitDialog(parent);			// create dialog initialization

	setHeaderList();					// init calibrator list

	connect(&m_timer, &QTimer::timeout, this, &CalibratorBase::timeoutInitialization, Qt::QueuedConnection);

	emit calibratorOpen();				// open all calibratirs
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createCalibrators(QWidget* parent)
{
	m_mutex.lock();
		m_calibratorManagerList.reserve(Metrology::ChannelCount);
	m_mutex.unlock();

	for(int channel = 0; channel < Metrology::ChannelCount; channel++)
	{
		std::shared_ptr<Calibrator> pCalibrator(new Calibrator(channel));
		if (pCalibrator == nullptr)
		{
			continue;
		}

		CalibratorOption calibratorOption = m_calibratorsOption.calibrator(channel);
		if (calibratorOption.isValid() == false)
		{
			continue;
		}

		pCalibrator->setPortName(calibratorOption.port());
		pCalibrator->setType(calibratorOption.type());

		std::shared_ptr<CalibratorManager> pManager(new CalibratorManager(pCalibrator, parent));
		if (pManager == nullptr)
		{
			continue;
		}

		m_mutex.lock();
			m_calibratorManagerList.push_back(pManager);
		m_mutex.unlock();

		QThread* pThread = new QThread;
		if (pThread != nullptr)
		{
			pCalibrator->moveToThread(pThread);

			pThread->start();
		}

		connect(this, &CalibratorBase::calibratorOpen, pCalibrator.get(), &Calibrator::open, Qt::QueuedConnection);
		connect(this, &CalibratorBase::calibratorClose, pCalibrator.get(), &Calibrator::close, Qt::QueuedConnection);
		connect(pCalibrator.get(), &Calibrator::connected, this, &CalibratorBase::onCalibratorConnected, Qt::QueuedConnection);
		connect(pCalibrator.get(), &Calibrator::disconnected, this, &CalibratorBase::onCalibratorDisconnected, Qt::QueuedConnection);

		emit pCalibrator->disconnected();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::removeCalibrators()
{
	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		std::shared_ptr<CalibratorManager> manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		std::shared_ptr<Calibrator> calibrator = manager->calibrator();
		if (calibrator == nullptr)
		{
			continue;
		}

		calibrator->setWaitResponse(false);

		if (calibrator->portIsOpen() == true)
		{
			int timeout = 0;

			while (calibrator->portIsOpen() == true)
			{
				QThread::msleep(CALIBRATOR_TIMEOUT_STEP);

				timeout += CALIBRATOR_TIMEOUT_STEP;
				if (timeout >= CALIBRATOR_TIMEOUT)
				{
					break;
				}
			}
		}

		QThread* pThread = calibrator->thread();
		if (pThread != nullptr)
		{
			pThread->quit();
			pThread->wait();
			delete pThread;
		}

		calibrator.reset();
		manager.reset();
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
	m_pInitDialog->setFixedSize(520, 180);
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

		m_pCalibratorMenu->addSeparator();

		m_pCopyAction = m_pCalibratorMenu->addAction(tr("&Copy"));
		m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
		m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

		m_pMenuBar->addMenu(m_pCalibratorMenu);

		connect(m_pInitAction, &QAction::triggered, this, &CalibratorBase::onInitialization);
		connect(m_pManageAction, &QAction::triggered, this, &CalibratorBase::onManage);
		connect(m_pSettingsAction, &QAction::triggered, this, static_cast<void (CalibratorBase::*)()>(&CalibratorBase::onSettings));
		connect(m_pCopyAction, &QAction::triggered, this, &CalibratorBase::onCopy);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setMenuBar(m_pMenuBar);

		m_pCalibratorView = new QTableWidget(m_pInitDialog);
		m_pCalibratorView->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pCalibratorProgress = new QProgressBar(m_pInitDialog);

		QVBoxLayout* listLayout = new QVBoxLayout;
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
		horizontalHeaderLabels.append(qApp->translate("CalibratorBase.h", CalibratorColumn[c]));
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
		verticalHeaderLabels.append(tr("Calibrator %1").arg(channel + 1));
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
		std::shared_ptr<CalibratorManager> manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		std::shared_ptr<Calibrator> pCalibrator = manager->calibrator();
		if (pCalibrator == nullptr)
		{
			continue;
		}

		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_PORT)->setText(pCalibrator->portName());
		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_TYPE)->setText(pCalibrator->typeStr());
		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_CONNECT)->setText(pCalibrator->isConnected() ? tr("Yes") : tr("No"));
		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_SN)->setText(pCalibrator->serialNo());

		m_pCalibratorView->item(index, CALIBRATOR_COLUMN_CONNECT)->setBackground(pCalibrator->isConnected() == true ?
																					 COLOR_CALIBRATOR_CONNECTED :
																					 COLOR_CALIBRATOR_NOT_CONNECTED);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::updateConnectedCalibrators()
{
	m_connectedCalibratorsCount = 0;

	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		std::shared_ptr<CalibratorManager> manager = calibratorManager(index);
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
	QMutexLocker l(&m_mutex);

	return TO_INT(m_calibratorManagerList.size());
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<CalibratorManager> CalibratorBase::calibratorManager(int index) const
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index >= TO_INT(m_calibratorManagerList.size()))
	{
		return nullptr;
	}

	return m_calibratorManagerList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<CalibratorManager> CalibratorBase::firstConnectedCalibrator() const
{
	std::shared_ptr<CalibratorManager> pFirstConnected;

	int count = calibratorCount();
	for(int index = 0; index < count; index++)
	{
		std::shared_ptr<CalibratorManager> pManager = calibratorManager(index);
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

std::shared_ptr<CalibratorManager> CalibratorBase::calibratorForMeasure(int index) const
{
	if (ERR_MEASURE_KIND(m_measureKind) == true)
	{
		return nullptr;
	}

	std::shared_ptr<CalibratorManager> pManager;

	switch(m_measureKind)
	{
		case Measure::Kind::OneRack:
		case Measure::Kind::OneModule:	pManager = firstConnectedCalibrator();	break;	// we need only one - connected;
		case Measure::Kind::MultiRack:	pManager = calibratorManager(index);	break;
		default:						assert(0);
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
		std::shared_ptr<CalibratorManager> manager = calibratorManager(index);
		if (manager == nullptr)
		{
			continue;
		}

		std::shared_ptr<Calibrator> calibrator = manager->calibrator();
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

void CalibratorBase::measureKindChanged(Measure::Kind measureKind)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	m_measureKind = measureKind;
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

	std::shared_ptr<CalibratorManager> manager = calibratorManager(index);
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

	std::shared_ptr<CalibratorManager> manager = calibratorManager(index);
	if (manager == nullptr)
	{
		return;
	}

	std::shared_ptr<Calibrator> calibrator = manager->calibrator();
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

		// Serial ports for calibrators
		//
		QHBoxLayout* portLayout = new QHBoxLayout ;

		QLabel* portLabel = new QLabel;
		portLabel->setText(tr("Serial port:"));

		QComboBox* portCombo = new QComboBox;

		// ports for all calibrators
		//
		foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
		{
			portCombo->addItem(info.portName());
		}

		#ifdef Q_OS_WIN

			// USB ports for Rigol
			//
			ViSession rscMng;
			ViStatus result = viOpenDefaultRM(&rscMng);		// open resource manager
			if (result == VI_SUCCESS || result == VI_WARN_CONFIG_NLOADED)
			{
				// search for the calibrator
				//
				ViChar usbPortCaption[VI_FIND_BUFLEN];
				ViUInt32 usbPortCount;
				ViFindList listOfFound;

				result = viFindRsrc(rscMng, const_cast<ViString>("USB?*"), &listOfFound, &usbPortCount, usbPortCaption);
				if (result == VI_SUCCESS)
				{
					portCombo->addItem(usbPortCaption);
				}

				viClose(rscMng);
			}

		#endif

		//
		//
		portCombo->setEditable(true);
		portCombo->setCurrentText(calibrator->portName());

		portLayout->addWidget(portLabel);
		portLayout->addWidget(portCombo);

		// calibrator type
		//
		QHBoxLayout* typeLayout = new QHBoxLayout ;

		QLabel* typeLabel = new QLabel;
		typeLabel->setText(tr("Calibrator type:"));

		QComboBox* typeCombo = new QComboBox;

		for (int t = 0; t < CalibratorTypeCount; t ++)
		{
			typeCombo->addItem(CalibratorTypeCaption(t));
		}
		typeCombo->setCurrentIndex(calibrator->type());

		typeLayout->addWidget(typeLabel);
		typeLayout->addWidget(typeCombo);

		// buttons
		//
		QHBoxLayout* buttonLayout = new QHBoxLayout ;

		QPushButton* okButton = new QPushButton(tr("Ok"));
		QPushButton* cancelButton = new QPushButton(tr("Cancel"));

		connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
		connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

		buttonLayout->addWidget(okButton);
		buttonLayout->addWidget(cancelButton);


		// add layoyt
		//
		QVBoxLayout* mainLayout = new QVBoxLayout ;

		mainLayout->addLayout(portLayout);
		mainLayout->addLayout(typeLayout);
		mainLayout->addStretch();
		mainLayout->addLayout(buttonLayout);

	dialog->setLayout(mainLayout);

	//
	//
	if (dialog->exec() != QDialog::Accepted)
	{
		return;
	}

	QString port = portCombo->currentText();
	if (port.isEmpty() == true)
	{

		return;
	}

	int type = typeCombo->currentIndex();
	if (ERR_CALIBRATOR_TYPE(type) == true)
	{
		return;
	}

	//
	//
	calibrator->setPortName(port);
	calibrator->setType(typeCombo->currentIndex());

	//
	//
	m_calibratorsOption.setCalibrator(index, CalibratorOption(port, type));
	m_calibratorsOption.save();

	//
	//
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onCopy()
{
	CopyData copyData(m_pCalibratorView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onContextMenu(QPoint)
{
	QMenu* menu = new QMenu(m_pCalibratorView);

	menu->addAction(m_pManageAction);
	menu->addAction(m_pSettingsAction);
	menu->addSeparator();
	menu->addAction(m_pCopyAction);

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

bool CalibratorBase::eventFilter(QObject* object, QEvent* event)
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
		QKeyEvent* keyEvent = static_cast<QKeyEvent* >(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
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



