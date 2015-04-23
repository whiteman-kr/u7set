#include "CalibratorManager.h"

#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDesktopWidget>

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager::CalibratorManager(Calibrator* pCalibrator, QWidget* parent) :
    QObject(parent),
    m_pCalibrator( pCalibrator ),
    m_parentWidget(parent)
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    createDialog();
    initDialog();
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager::~CalibratorManager()
{
    if (m_pErrorDialog != nullptr)
    {
        delete m_pErrorDialog;
    }

    if (m_pDialog != nullptr)
    {
        delete m_pDialog;
    }

    m_pCalibrator = nullptr;
    m_index = -1;

    m_ready = false;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::createDialog()
{
    // create elements of interface
    //
    m_pDialog = new QDialog(m_parentWidget);

    QGroupBox* valueGroup = new QGroupBox(tr("Display"));
    QVBoxLayout *valueLayout = new QVBoxLayout;

    m_pMeasureLabel = new QLabel(tr("Measure"), m_pDialog);
    m_pMeasureEdit = new QLineEdit(m_pDialog);

    m_pSourceLabel = new QLabel(tr("Source"), m_pDialog);
    m_pSourceEdit = new QLineEdit(m_pDialog);

    valueLayout->addWidget(m_pMeasureLabel);
    valueLayout->addWidget(m_pMeasureEdit);
    valueLayout->addWidget(m_pSourceLabel);
    valueLayout->addWidget(m_pSourceEdit);

    valueGroup->setLayout(valueLayout);

    m_pValueEdit = new QLineEdit(m_pDialog);

    QGroupBox* buttonGroup = new QGroupBox(tr("Control"));
    QVBoxLayout *buttonLayout = new QVBoxLayout;

    m_pSetValueButton = new QPushButton(tr("set value"), m_pDialog);

    QHBoxLayout *stepButtonLayout = new QHBoxLayout;

    m_pStepDownButton = new QPushButton(tr("Step down"), m_pDialog);
    m_pStepUpButton = new QPushButton(tr("Step up"), m_pDialog);

    stepButtonLayout->addWidget(m_pStepDownButton);
    stepButtonLayout->addWidget(m_pStepUpButton);

    buttonLayout->addWidget(m_pSetValueButton);
    buttonLayout->addLayout(stepButtonLayout);

    buttonGroup->setLayout(buttonLayout);

    QHBoxLayout *unitLayout = new QHBoxLayout;

    m_pModeList = new QComboBox(m_pDialog);
    m_pUnitList = new QComboBox(m_pDialog);

    unitLayout->addWidget(m_pModeList);
    unitLayout->addWidget(m_pUnitList);

    m_pRemoteControlCheck = new QCheckBox(tr("Remote control"), m_pDialog);

    m_pErrorsButton = new QPushButton(tr("List errors"), m_pDialog);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(valueGroup);
    mainLayout->addWidget(m_pValueEdit);
    mainLayout->addWidget(buttonGroup);
    mainLayout->addLayout(unitLayout);
    mainLayout->addWidget(m_pErrorsButton);
    mainLayout->addWidget(m_pRemoteControlCheck);
    mainLayout->addStretch();

    m_pErrorDialog = new QDialog(m_pDialog);

    QVBoxLayout *errorLayout = new QVBoxLayout;

    m_pErrorList = new QTextEdit(m_pDialog);
    errorLayout->addWidget(m_pErrorList);

    m_pErrorDialog->setLayout(errorLayout);

    m_pDialog->setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::initDialog()
{
    // init elements of interface
    //
    m_pDialog->setWindowFlags(Qt::Drawer);
    m_pDialog->setFixedWidth(190);
    m_pDialog->setWindowIcon(QIcon(":/icons/Manage.png"));

    QFont* font = new QFont("Arial", 16, 2);

    m_pMeasureLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pMeasureEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pMeasureEdit->setReadOnly(true);
    m_pMeasureEdit->setFont(*font);


    m_pSourceLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pSourceEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pSourceEdit->setReadOnly(true);
    m_pSourceEdit->setFont(*font);


    QRegExp rx( "^[-]{0,1}[0-9]*[.]{1}[0-9]*$" );
    QValidator *validator = new QRegExpValidator(rx, m_pDialog);

    m_pValueEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_pValueEdit->setFont(*font);
    m_pValueEdit->setValidator(validator);

    m_pSetValueButton->setDefault(true);
    m_pSetValueButton->setFont(*font);

    delete font;

    for (int m = 0; m < CALIBRATOR_MODE_COUNT; m++)
    {
        m_pModeList->addItem( CalibratorMode[m] );
    }
    m_pModeList->setCurrentIndex(m_pCalibrator->mode());


    for (int u = 0; u < CALIBRATOR_UNIT_COUNT; u++)
    {
        m_pUnitList->addItem(CalibratorUnit[u]);
    }

    switch(m_pCalibrator->mode())
    {
        case CALIBRATOR_MODE_MEASURE:   m_pUnitList->setCurrentIndex(m_pCalibrator->measureUnit()); break;
        case CALIBRATOR_MODE_SOURCE:    m_pUnitList->setCurrentIndex(m_pCalibrator->sourceUnit());  break;
        default:                        m_pUnitList->setCurrentIndex(CALIBRATOR_UNIT_UNKNOWN);         break;
    }

    m_pRemoteControlCheck->setLayoutDirection(Qt::RightToLeft);
    m_pRemoteControlCheck->setChecked(true);

    m_pErrorDialog->setWindowFlags(Qt::Drawer);
    m_pErrorDialog->setMinimumSize(700, 50);
    m_pErrorList->setReadOnly(true);

    connect(m_pCalibrator, &Calibrator::connected, this, &CalibratorManager::onCalibratorConnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::disconnected, this, &CalibratorManager::onCalibratorDisconnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::unitIsChanged, this, &CalibratorManager::onUnitChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsRequested , this, &CalibratorManager::onValueChanging, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsReceived, this, &CalibratorManager::onValueChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::error, this, &CalibratorManager::onCalibratorError, Qt::QueuedConnection);

    connect(this, &CalibratorManager::calibratorGetValue, m_pCalibrator, &Calibrator::getValue, Qt::QueuedConnection);

    connect(m_pSetValueButton, &QPushButton::clicked, this, &CalibratorManager::onSetValue);
    connect(this, &CalibratorManager::calibratorSetValue, m_pCalibrator, &Calibrator::setValue, Qt::QueuedConnection);
    connect(m_pStepDownButton, &QPushButton::clicked, this, &CalibratorManager::onStepDown);
    connect(this, &CalibratorManager::calibratorStepDown, m_pCalibrator, &Calibrator::stepDown, Qt::QueuedConnection);
    connect(m_pStepUpButton, &QPushButton::clicked, this, &CalibratorManager::onStepUp);
    connect(this, &CalibratorManager::calibratorStepUp, m_pCalibrator, &Calibrator::stepUp, Qt::QueuedConnection);

    connect(m_pModeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CalibratorManager::onModeUnitList);
    connect(m_pUnitList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CalibratorManager::onModeUnitList);
    connect(this, &CalibratorManager::calibratorSetUnit, m_pCalibrator, &Calibrator::setUnit, Qt::QueuedConnection);

    connect(m_pErrorsButton, &QPushButton::clicked, this, &CalibratorManager::onErrorList);

    connect(m_pRemoteControlCheck, &QCheckBox::clicked, this, &CalibratorManager::onRemoveControl);
    connect(this, &CalibratorManager::calibratorRemoveControl, m_pCalibrator, &Calibrator::setRemoteControl, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::show()
{
    if (m_pDialog == nullptr)
    {
        return;
    }

    m_pDialog->show();
    m_pDialog->move(m_parentWidget->geometry().center() - m_pDialog->rect().center());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::enableInterface(bool enable)
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    if (enable == true && m_pCalibrator->isConnected() == false)
    {
        enable = false;
    }

    m_pMeasureEdit->setEnabled(enable);
    m_pSourceEdit->setEnabled(enable);
    m_pValueEdit->setEnabled(enable);
    m_pSetValueButton->setEnabled(enable);
    m_pStepDownButton->setEnabled(enable);
    m_pStepUpButton->setEnabled(enable);
    m_pModeList->setEnabled(enable);
    m_pUnitList->setEnabled(enable);
    m_pRemoteControlCheck->setEnabled(enable);

    if (m_pCalibrator->mode() != CALIBRATOR_MODE_SOURCE || m_pCalibrator->sourceUnit() == CALIBRATOR_UNIT_UNKNOWN)
    {
        m_pSetValueButton->setEnabled(false);
        m_pStepDownButton->setEnabled(false);
        m_pStepUpButton->setEnabled(false);
    }

    if (m_pCalibrator->type() == CALIBRATOR_TYPE_CALYS75)
    {
        m_pRemoteControlCheck->setVisible(true);
    }
    else
    {
        m_pRemoteControlCheck->setVisible(false);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void  CalibratorManager::onCalibratorError(QString text)
{
    QTime time;
    time.currentTime();

    QString error = QTime::currentTime().toString("hh:mm:ss.zzz - ") + text;

    m_pErrorList->append(error);
    m_pErrorsButton->setText( QString("List errors (%1)").arg(m_pErrorList->document()->lineCount() ) );

    if(m_pDialog->isVisible() == false)
    {
        return;
    }

    QMessageBox::critical(m_pDialog, m_pDialog->windowTitle(), text);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onCalibratorConnect()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    QString title = QString("c:%1 ").arg(m_index + 1) + m_pCalibrator->name() + QString(" %1").arg(m_pCalibrator->serialNo()) ;
    m_pDialog->setWindowTitle( title );
    m_pErrorDialog->setWindowTitle(title + tr(" : List errors"));

    enableInterface(true);

    emit calibratorGetValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onCalibratorDisconnect()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    QString title = QString("c:%1 (%2) - Disconnected").arg(m_index + 1).arg(m_pCalibrator->portName()) ;
    m_pDialog->setWindowTitle(title);
    m_pErrorDialog->setWindowTitle(title + tr(" : List errors"));

    enableInterface(false);

    m_ready = true;
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorManager::calibratorIsConnected()
{
    if (m_pCalibrator == nullptr)
    {
        return false;
    }

    return m_pCalibrator->isConnected();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onUnitChanged()
{
    if (calibratorIsConnected() == false)
    {
        return;
    }

    updateValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onValueChanging()
{
    enableInterface(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onValueChanged()
{
    if (calibratorIsConnected() == false)
    {
        return;
    }

    updateValue();

    enableInterface(true);

    m_ready = true;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::updateValue()
{
    int measureUnit = m_pCalibrator->measureUnit();
    int sourceUnit = m_pCalibrator->sourceUnit();

    QString measureValue, sourceValue;

    if (measureUnit < 0 || measureUnit >= CALIBRATOR_UNIT_COUNT)
    {
        measureValue = QString::number(m_pCalibrator->measureValue(), 10, 3);
    }
    else
    {
        measureValue = QString::number(m_pCalibrator->measureValue(), 10, 3) + " " + CalibratorUnit[ measureUnit ];
    }

    if (sourceUnit < 0 || sourceUnit >= CALIBRATOR_UNIT_COUNT)
    {
        sourceValue = QString::number(m_pCalibrator->sourceValue(), 10, 3);
    }
    else
    {
        sourceValue = QString::number(m_pCalibrator->sourceValue(), 10, 3) + " " + CalibratorUnit[ sourceUnit ];
    }

    m_pMeasureEdit->setText(measureValue);
    m_pMeasureEdit->setCursorPosition(0);
    m_pSourceEdit->setText(sourceValue);
    m_pSourceEdit->setCursorPosition(0);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::getValue()
{
    if (calibratorIsConnected() == false)
    {
        return;
    }

    emit calibratorGetValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::setValue(double value)
{
    if (calibratorIsConnected() == false)
    {
        return;
    }

    m_ready = false;

    emit calibratorSetValue(value);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onSetValue()
{
    QString value = m_pValueEdit->text();

    if (value.isEmpty() == true)
    {
        return;
    }

    setValue(value.toDouble());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::stepDown()
{
    if (calibratorIsConnected() == false)
    {
        return;
    }

    m_ready = false;

    emit calibratorStepDown();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onStepDown()
{
    stepDown();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::stepUp()
{
    if (calibratorIsConnected() == false)
    {
        return;
    }

    m_ready = false;

    emit calibratorStepUp();
}

// -------------------------------------------------------------------------------------------------------------------


void CalibratorManager::onStepUp()
{
    stepUp();
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorManager::setUnit(int mode, int unit)
{
    if (calibratorIsConnected() == false)
    {
        return false;
    }

    if (mode < 0 || mode >= CALIBRATOR_MODE_COUNT)
    {
        return false;
    }

    if (unit < 0 || unit >= CALIBRATOR_UNIT_COUNT)
    {
        return false;
    }

    m_ready = true;

    emit calibratorSetUnit(mode, unit);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onModeUnitList(int)
{
    m_pSetValueButton->setEnabled(false);
    m_pStepDownButton->setEnabled(false);
    m_pStepUpButton->setEnabled(false);

    int mode = m_pModeList->currentIndex();
    int unit = m_pUnitList->currentIndex();

    if (setUnit(mode, unit) == false)
    {
        return;
    }

    if (mode == CALIBRATOR_MODE_SOURCE)
    {
        m_pSetValueButton->setEnabled(true);
        m_pStepDownButton->setEnabled(true);
        m_pStepUpButton->setEnabled(true);
    }

    emit calibratorGetValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onErrorList()
{
    if (m_pDialog == nullptr)
    {
        return;
    }

    if (m_pErrorDialog == nullptr)
    {
        return;
    }

    m_pErrorDialog->show();
    m_pErrorDialog->move(m_pDialog->geometry().center() - m_pErrorDialog->rect().center());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onRemoveControl()
{
    emit calibratorRemoveControl( m_pRemoteControlCheck->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::loadSettings()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    if (m_index == -1)
    {
        return;
    }

    QSettings s;

    QString portName = s.value( QString("%1Calibrator%2/port").arg(CALIBRATOR_OPTIONS_KEY).arg(m_index + 1), QString("COM%1").arg( m_index + 1)).toString();
    int type = s.value(QString("%1Calibrator%2/type").arg(CALIBRATOR_OPTIONS_KEY).arg(m_index + 1), CALIBRATOR_TYPE_TRXII).toInt();

    m_pCalibrator->setPortName(portName);
    m_pCalibrator->setType(type);

}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::saveSettings()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    if (m_index == -1)
    {
        return;
    }

    QSettings s;

    s.setValue(QString("%1Calibrator%2/port").arg(CALIBRATOR_OPTIONS_KEY).arg(m_index + 1), m_pCalibrator->portName());
    s.setValue(QString("%1Calibrator%2/type").arg(CALIBRATOR_OPTIONS_KEY).arg(m_index + 1), m_pCalibrator->type());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int CalibratorManagerList::count()
{
    int count = 0;

    m_mutex.lock();

        count = m_list.count();

    m_mutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

int CalibratorManagerList::append(CalibratorManager* pManager)
{
    if (pManager == nullptr)
    {
        return -1;
    }

    int count = 0;

    m_mutex.lock();

        m_list.append(pManager);
        count = m_list.count();

    m_mutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager* CalibratorManagerList::at(int index)
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    CalibratorManager* pManager = nullptr;

    m_mutex.lock();

        pManager = m_list.at(index);

    m_mutex.unlock();

    return pManager;
}

// -------------------------------------------------------------------------------------------------------------------

bool CalibratorManagerList::removeAt(int index)
{
    if (index < 0 || index >= count())
    {
        return false;
    }

    m_mutex.lock();

        m_list.removeAt(index);

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerList::clear()
{
    m_mutex.lock();

        m_list.clear();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
