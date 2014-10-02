#include "CalibratorBase.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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
    createMainWnd();                    // create dialog initialization

    setHeaderList();                    // init calibrator list

    connect(&m_timer, &QTimer::timeout, this, &CalibratorBase::timeoutInitialization, Qt::QueuedConnection);

    emit openAllCalibrator();           // open all calibratirs
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::clear()
{
    removeCalibrators();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createCalibrators()
{
    for(int index = 0; index < MAX_CALIBRATOR_COUNT; index++ )
    {
        Calibrator* pCalibrator = new Calibrator;
        if (pCalibrator == nullptr)
        {
            continue;
        }

        m_calibratorList.append(pCalibrator);

        pCalibrator->setIndex(index);
        pCalibrator->loadSettings();

        QThread *pThread = new QThread;
        if (pThread != nullptr)
        {
            pCalibrator->moveToThread(pThread);

            connect( pThread, SIGNAL(finished()), pCalibrator, SLOT(deleteLater()) );

            pThread->start();
        }

        connect(this, &CalibratorBase::openAllCalibrator, pCalibrator, &Calibrator::open, Qt::QueuedConnection);
        connect(this, &CalibratorBase::closeAllCalibrator, pCalibrator, &Calibrator::close, Qt::QueuedConnection);
        connect(pCalibrator, &Calibrator::connected, this, &CalibratorBase::onConnected, Qt::QueuedConnection);

        CalibratorManagerDialog* pCalibratorManager = new CalibratorManagerDialog(pCalibrator, m_parentWidget);
        m_calibratorManagerList.append(pCalibratorManager);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::removeCalibrators()
{
    int dialogCount = m_calibratorManagerList.count();
    for(int index = 0; index < dialogCount; index++ )
    {
        CalibratorManagerDialog* dialog = m_calibratorManagerList.at(index);
        if (dialog == nullptr)
        {
            continue;
        }

        delete dialog;
    }

    emit closeAllCalibrator();

    int calibratorCount = m_calibratorList.count();
    for(int index = 0; index < calibratorCount; index++ )
    {
        Calibrator* pCalibrator = m_calibratorList.at(index);
        if (pCalibrator == nullptr)
        {
            continue;
        }

        pCalibrator->enableWaitResponse(false);

        QThread *pThread = pCalibrator->thread();

        pThread->quit();
        pThread->wait();
        pThread->deleteLater();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::createMainWnd()
{
    m_pMainWnd = new QDialog(m_parentWidget);
    m_pMainWnd->setWindowFlags(Qt::Drawer);
    m_pMainWnd->setFixedSize(500, 200);
    m_pMainWnd->setWindowTitle(tr("Calibrators initialization"));

        m_pMenuBar = new QMenuBar(m_pMainWnd);
        m_pCalibratorMenu = new QMenu(tr("&Calibrators"), m_pMainWnd);

        m_pInitAction = m_pCalibratorMenu->addAction(tr("&Initialization"));
        m_pInitAction->setShortcut(Qt::CTRL + Qt::Key_I);
        m_pCalibratorMenu->addSeparator();
        m_pManageAction = m_pCalibratorMenu->addAction(tr("&Manage ..."));
        m_pManageAction->setShortcut(Qt::CTRL + Qt::Key_M);
        m_pEditAction = m_pCalibratorMenu->addAction(tr("&Edit settings ..."));
        m_pEditAction->setShortcut(Qt::CTRL + Qt::Key_E);


        m_pMenuBar->addMenu(m_pCalibratorMenu);

        connect(m_pInitAction, &QAction::triggered, this, &CalibratorBase::onInitialization, Qt::QueuedConnection);
        connect(m_pManageAction, &QAction::triggered, this, &CalibratorBase::onManage, Qt::QueuedConnection);
        connect(m_pEditAction, &QAction::triggered, this, &CalibratorBase::onEdit, Qt::QueuedConnection);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->setMenuBar(m_pMenuBar);

        m_pCalibratorView = new QTableWidget(m_pMainWnd);
        m_pCalibratorProgress = new QProgressBar(m_pMainWnd);

        QVBoxLayout *listLayout = new QVBoxLayout;
        listLayout->addWidget(m_pCalibratorView);
        listLayout->addWidget(m_pCalibratorProgress);

        mainLayout->addLayout(listLayout);

    m_pMainWnd->setLayout(mainLayout);

    connect(m_pMainWnd, &QDialog::finished , this, &CalibratorBase::onFinishedInitializationWnd, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onFinishedInitializationWnd()
{
    if (m_timeout != 0)
    {
        m_pMainWnd->show();
    }
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

    connect(m_pCalibratorView, &QTableWidget::cellDoubleClicked, this, &CalibratorBase::editSettings, Qt::QueuedConnection);

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
    int count = m_calibratorList.count();
    for(int index = 0; index < count; index++ )
    {
        Calibrator* pCalibrator = m_calibratorList.at(index);

        m_pCalibratorView->item(index, CalibratorColumnPort)->setText(pCalibrator->getPortName());
        m_pCalibratorView->item(index, CalibratorColumnType)->setText(pCalibrator->getTypeStr());
        m_pCalibratorView->item(index, CalibratorColumnConnect)->setText(pCalibrator->isConnected() ? tr("Yes") : tr("No"));
        m_pCalibratorView->item(index, CalibratorColumnSN)->setText(pCalibrator->getSerialNo());

        m_pCalibratorView->item(index, CalibratorColumnConnect)->setBackgroundColor(pCalibrator->isConnected() ? Qt::green : Qt::white);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::showWnd()
{
    updateList();

    m_pMainWnd->show();
    m_pCalibratorProgress->setValue(0);
}

// -------------------------------------------------------------------------------------------------------------------

Calibrator* CalibratorBase::getMainCalibrator()
{
    Calibrator* pMainCalibrator = nullptr;

    int count = m_calibratorList.count();
    for(int index = 0; index < count; index++ )
    {
        Calibrator* pCalibrator = m_calibratorList.at(index);

        if (pCalibrator->isConnected() == true )
        {
            pMainCalibrator = pCalibrator;
            break;
        }
    }

    return pMainCalibrator;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onInitialization()
{
    m_pMainWnd->setWindowFlags(Qt::Tool | Qt::WindowSystemMenuHint );
    m_pMainWnd->show();

    m_pCalibratorMenu->setEnabled(false);
    m_pCalibratorView->setEnabled(false);

    int calibratorCount = m_calibratorList.count();
    for(int index = 0; index < calibratorCount; index++ )
    {
        Calibrator* pCalibrator = m_calibratorList.at(index);
        if (pCalibrator == nullptr)
        {
            continue;
        }

        pCalibrator->enableWaitResponse(false);
    }

    emit closeAllCalibrator();      // close all calibratirs serial port
    emit openAllCalibrator();       // open all calibratirs

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

        m_pMainWnd->setWindowFlags( Qt::Drawer );
        m_pMainWnd->show();

        updateList();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onManage()
{
    int index = m_pCalibratorView->currentRow();
    if (index < 0 || index >= m_calibratorList.count())
    {
        QMessageBox msg;
        msg.setText(tr("Please, select calibrator for manage!"));
        msg.exec();

        return;
    }

    CalibratorManagerDialog* dialog = m_calibratorManagerList.at(index);
    if (dialog == nullptr)
    {
        return;
    }

    dialog->show();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onEdit()
{
    int index = m_pCalibratorView->currentRow();
    if (index < 0 || index >= m_calibratorList.count())
    {
        QMessageBox msg;
        msg.setText(tr("Please, select calibrator for manage!"));
        msg.exec();

        return;
    }

    editSettings(index, -1);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onContextMenu(QPoint)
{
    QMenu *menu = new QMenu(m_pCalibratorView);
    menu->addAction(m_pManageAction);
    menu->addAction(m_pEditAction);
    menu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::editSettings(int row,int)
{
    int index = row;
    if (index < 0 || index >= m_calibratorList.count())
    {
        return;
    }

    Calibrator* pCalibrator = m_calibratorList.at(index);
    if (pCalibrator == nullptr)
    {
        return;
    }

    // create dialog for select serial port and calibrator type
    //
    QDialog* dialog = new QDialog(m_pMainWnd);
    dialog->setWindowFlags(Qt::Drawer);
    dialog->setFixedSize(200, 120);
    dialog->setWindowTitle(tr("Edit settings calibrator %1").arg(pCalibrator->getIndex() + 1));

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
        portCombo->setCurrentText(pCalibrator->getPortName());

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
        typeCombo->setCurrentIndex(pCalibrator->getType());

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

    pCalibrator->setPortName(portCombo->currentText());
    pCalibrator->setType(typeCombo->currentIndex());
    pCalibrator->saveSettings();

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorBase::onConnected()
{
    updateList();
}

// -------------------------------------------------------------------------------------------------------------------
