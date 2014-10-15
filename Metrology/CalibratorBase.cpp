#include "CalibratorBase.h"


#include <QMessageBox>
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
    m_parentWidget = parent;

    createCalibrators();                // create objects of calibrators
    createDialog();                    // create dialog initialization

    setHeaderList();                    // init calibrator list

    connect(&m_timer, &QTimer::timeout, this, &CalibratorBase::timeoutInitialization, Qt::QueuedConnection);

    emit calibratorOpen();           // open all calibratirs
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createCalibrators()
{
    for(int index = 0; index < MAX_CALIBRATOR_COUNT; index++ )
    {
        Calibrator* calibrator = new Calibrator;
        if (calibrator == nullptr)
        {
            continue;
        }

        CalibratorManager* manager = new CalibratorManager(calibrator, m_parentWidget);
        if (manager == nullptr)
        {
            continue;
        }

        manager->setIndex(index);
        manager->loadSettings();

        m_calibratorManagerList.append(manager);


        QThread *pThread = new QThread;
        if (pThread != nullptr)
        {
            calibrator->moveToThread(pThread);

            connect( pThread, &QThread::finished, calibrator, &Calibrator::deleteLater );

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

    int count = m_calibratorManagerList.count();
    for(int index = 0; index < count; index++)
    {
        CalibratorManager* manager = m_calibratorManagerList.at(index);
        if (manager == nullptr)
        {
            continue;
        }

        Calibrator* calibrator = manager->getCalibrator();
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

    m_calibratorManagerList.clear();

}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createDialog()
{
    m_pDialog = new QDialog(m_parentWidget);
    m_pDialog->setWindowFlags(Qt::Drawer);
    m_pDialog->setFixedSize(500, 200);
    m_pDialog->setWindowTitle(tr("Calibrators initialization"));
    m_pDialog->installEventFilter(this);

        m_pMenuBar = new QMenuBar(m_pDialog);
        m_pCalibratorMenu = new QMenu(tr("&Calibrators"), m_pDialog);

        m_pInitAction = m_pCalibratorMenu->addAction(tr("&Initialization"));
        m_pInitAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/Calibrators.png")));
        m_pInitAction->setShortcut(Qt::CTRL + Qt::Key_I);
        m_pCalibratorMenu->addSeparator();
        m_pManageAction = m_pCalibratorMenu->addAction(tr("&Manage ..."));
        m_pManageAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/Manage.png")));
        m_pManageAction->setShortcut(Qt::CTRL + Qt::Key_M);
        m_pSettingsAction = m_pCalibratorMenu->addAction(tr("&Settings ..."));
        m_pSettingsAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/Settings.png")));
        m_pSettingsAction->setShortcut(Qt::CTRL + Qt::Key_S);


        m_pMenuBar->addMenu(m_pCalibratorMenu);

        connect(m_pInitAction, &QAction::triggered, this, &CalibratorBase::onInitialization, Qt::QueuedConnection);
        connect(m_pManageAction, &QAction::triggered, this, &CalibratorBase::onManage, Qt::QueuedConnection);
        connect(m_pSettingsAction, &QAction::triggered, this, static_cast<void (CalibratorBase::*)()>(&CalibratorBase::onSettings), Qt::QueuedConnection);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->setMenuBar(m_pMenuBar);

        m_pCalibratorView = new QTableWidget(m_pDialog);
        m_pCalibratorProgress = new QProgressBar(m_pDialog);

        QVBoxLayout *listLayout = new QVBoxLayout;
        listLayout->addWidget(m_pCalibratorView);
        listLayout->addWidget(m_pCalibratorProgress);

        mainLayout->addLayout(listLayout);

    m_pDialog->setLayout(mainLayout);
}



// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::setHeaderList()
{
    // init colomns
    //
    QStringList horizontalHeaderLabels;

    for(int c = 0; c < CalibratorsColumnCount; c++)
    {
        horizontalHeaderLabels.append(CalibratorsColumn[c]);
    }

    m_pCalibratorView->setColumnCount(CalibratorsColumnCount);
    m_pCalibratorView->setHorizontalHeaderLabels(horizontalHeaderLabels);
    m_pCalibratorView->setEditTriggers(QAbstractItemView::NoEditTriggers);


    // init rows
    //
    QStringList verticalHeaderLabels;
    m_pCalibratorView->setRowCount(MAX_CALIBRATOR_COUNT);

    for(int index = 0; index < MAX_CALIBRATOR_COUNT; index++ )
    {
        verticalHeaderLabels.append(QString("Calibrator %1").arg(index + 1));
        m_pCalibratorView->setRowHeight(index, 18);
    }
    m_pCalibratorView->setVerticalHeaderLabels(verticalHeaderLabels);

    m_pCalibratorView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int count = horizontalHeaderLabels.count();
    for(int column = 0; column < count; column++ )
    {
        for(int row = 0; row < MAX_CALIBRATOR_COUNT; row++ )
        {
            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setTextAlignment(Qt::AlignHCenter);
            m_pCalibratorView->setItem(row, column, item);
        }
    }

    connect(m_pCalibratorView, &QTableWidget::cellDoubleClicked, this, static_cast<void (CalibratorBase::*)(int, int)>(&CalibratorBase::onSettings), Qt::QueuedConnection);

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
    int count = m_calibratorManagerList.count();
    for(int index = 0; index < count; index++ )
    {
        CalibratorManager* manager = m_calibratorManagerList.at(index);
        if (manager == nullptr)
        {
            continue;
        }

        Calibrator* pCalibrator = manager->getCalibrator();
        if (pCalibrator == nullptr)
        {
            continue;
        }

        m_pCalibratorView->item(index, CalibratorColumnPort)->setText(pCalibrator->getPortName());
        m_pCalibratorView->item(index, CalibratorColumnType)->setText(pCalibrator->getTypeString());
        m_pCalibratorView->item(index, CalibratorColumnConnect)->setText(pCalibrator->isConnected() ? tr("Yes") : tr("No"));
        m_pCalibratorView->item(index, CalibratorColumnSN)->setText(pCalibrator->getSerialNo());

        m_pCalibratorView->item(index, CalibratorColumnConnect)->setBackgroundColor(pCalibrator->isConnected() ? Qt::green : Qt::white);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::updateConnectedCalibrators()
{
    m_connectedCalibratorsCount = 0;

    int count = m_calibratorManagerList.count();
    for(int index = 0; index < count; index++ )
    {
        CalibratorManager* manager = m_calibratorManagerList.at(index);
        if (manager == nullptr)
        {
            continue;
        }

        if (manager->calibratorIsConnected() == true)
        {
            m_connectedCalibratorsCount ++;
        }
    }

    emit calibratorConnectedChanged( m_connectedCalibratorsCount);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::show()
{
    updateList();

    m_pDialog->show();
    m_pCalibratorProgress->setValue(0);
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager* CalibratorBase::at(int index)
{
    if (index < 0 || index >= m_calibratorManagerList.count())
    {
        return nullptr;
    }

    return m_calibratorManagerList.at(index);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onInitialization()
{
    m_pDialog->setWindowFlags(Qt::Tool | Qt::WindowSystemMenuHint );
    m_pDialog->show();
    m_pDialog->activateWindow();

    m_pCalibratorMenu->setEnabled(false);
    m_pCalibratorView->setEnabled(false);

    int calibratorCount = count();
    for(int index = 0; index < calibratorCount; index++ )
    {
        CalibratorManager* manager = at(index);
        if (manager == nullptr)
        {
            continue;
        }

        Calibrator* calibrator = manager->getCalibrator();
        if (calibrator == nullptr)
        {
            continue;
        }

        calibrator->setWaitResponse(false);
    }

    emit calibratorClose();                         // close all calibratirs serial port
    QThread::msleep(MAX_CALIBRATOR_COUNT *  100 );  // wait, until all the serial ports will be closed
    emit calibratorOpen();                          // open all calibratirs

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

        m_pDialog->setWindowFlags( Qt::Drawer );
        m_pDialog->show();
        m_pDialog->activateWindow();

        updateList();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onManage()
{
    int index = m_pCalibratorView->currentRow();
    if (index < 0 || index >= count())
    {
        QMessageBox::information(m_pDialog, m_pDialog->windowTitle(), tr("Please, select calibrator for manage!"));
        return;
    }

    CalibratorManager* manager = m_calibratorManagerList.at(index);
    if (manager == nullptr)
    {
        return;
    }

    manager->show();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onSettings()
{
    int index = m_pCalibratorView->currentRow();
    if (index < 0 || index >= count())
    {
        QMessageBox::information(m_pDialog, m_pDialog->windowTitle(), tr("Please, select calibrator for edit settings!"));
        return;
    }

    onSettings(index, -1);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onSettings(int row,int)
{
    int index = row;
    if (index < 0 || index >= count())
    {
        return;
    }

    CalibratorManager* manager = m_calibratorManagerList.at(index);
    if (manager == nullptr)
    {
        return;
    }

    Calibrator* calibrator = manager->getCalibrator();
    if (calibrator == nullptr)
    {
        return;
    }

    // create dialog for select serial port and calibrator type
    //
    QDialog* dialog = new QDialog(m_pDialog);
    dialog->setWindowFlags(Qt::Drawer);
    dialog->setFixedSize(200, 120);
    dialog->setWindowTitle(tr("Settings calibrator %1").arg(manager->getIndex() + 1));

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
        portCombo->setCurrentText(calibrator->getPortName());

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
        typeCombo->setCurrentIndex(calibrator->getType());

        typeLayout->addWidget(typeLabel);
        typeLayout->addWidget(typeCombo);

        // buttons
        //
        QHBoxLayout *buttonLayout = new QHBoxLayout ;

        QPushButton* okButton = new QPushButton(tr("OK"));
        QPushButton* cancelButton = new QPushButton(tr("Cancel"));

        connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept, Qt::QueuedConnection);
        connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject, Qt::QueuedConnection);

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

    manager->saveSettings();

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onContextMenu(QPoint)
{
    QMenu *menu = new QMenu(m_pCalibratorView);
    menu->addAction(m_pManageAction);
    menu->addAction(m_pSettingsAction);
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
            m_pDialog->show();
            m_pDialog->activateWindow();

            event->ignore();
        }
    }

    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>( event );

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

