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

CalibratorManager::CalibratorManager(Calibrator* pCalibrator, QWidget *parent) :
    QDialog(parent),
    m_pCalibrator( pCalibrator )
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    createInterface();
    initInterface();
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManager::~CalibratorManager()
{
    m_index = -1;
    m_pCalibrator = nullptr;
    m_ready = false;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::createInterface()
{
    // create elements of interface
    //
    QGroupBox* valueGroup = new QGroupBox;
    QVBoxLayout *valueLayout = new QVBoxLayout;

    m_pMeasureLabel = new QLabel(tr("Measure"));
    m_pMeasureEdit = new QLineEdit;

    m_pSourceLabel = new QLabel(tr("Source"));
    m_pSourceEdit = new QLineEdit;

    valueLayout->addWidget(m_pMeasureLabel);
    valueLayout->addWidget(m_pMeasureEdit);
    valueLayout->addWidget(m_pSourceLabel);
    valueLayout->addWidget(m_pSourceEdit);

    valueGroup->setLayout(valueLayout);

    m_pValueEdit = new QLineEdit;

    QGroupBox* buttonGroup = new QGroupBox;
    QVBoxLayout *buttonLayout = new QVBoxLayout;

    m_pSetValueButton = new QPushButton(tr("set value"));

    QHBoxLayout *stepButtonLayout = new QHBoxLayout;

    m_pStepDownButton = new QPushButton(tr("Step down"));
    m_pStepUpButton = new QPushButton(tr("Step up"));

    stepButtonLayout->addWidget(m_pStepDownButton);
    stepButtonLayout->addWidget(m_pStepUpButton);

    buttonLayout->addWidget(m_pSetValueButton);
    buttonLayout->addLayout(stepButtonLayout);

    buttonGroup->setLayout(buttonLayout);

    QHBoxLayout *unitLayout = new QHBoxLayout;

    m_pModeList = new QComboBox;
    m_pUnitList = new QComboBox;

    unitLayout->addWidget(m_pModeList);
    unitLayout->addWidget(m_pUnitList);

    m_pRemoteControlCheck = new QCheckBox(tr("Remote control"));

    m_pErrorsButton = new QPushButton(tr("List errors"));

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(valueGroup);
    mainLayout->addWidget(m_pValueEdit);
    mainLayout->addWidget(buttonGroup);
    mainLayout->addLayout(unitLayout);
    mainLayout->addWidget(m_pErrorsButton);
    mainLayout->addWidget(m_pRemoteControlCheck);

    m_pErrorDialog = new QDialog(this);

    QVBoxLayout *errorLayout = new QVBoxLayout;

    m_pErrorList = new QTextEdit( );
    errorLayout->addWidget(m_pErrorList);

    m_pErrorDialog->setLayout(errorLayout);

    setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::initInterface()
{
    // init elements of interface
    //
    setWindowFlags(Qt::Drawer);
    setFixedSize(190,350);

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
    QValidator *validator = new QRegExpValidator(rx, this);

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
    m_pModeList->setCurrentIndex(m_pCalibrator->getMode());


    for (int u = 0; u < CALIBRATOR_UNIT_COUNT; u++)
    {
        m_pUnitList->addItem(CalibratorUnit[u]);
    }

    switch(m_pCalibrator->getMode())
    {
        case CALIBRATOR_MODE_MEASURE:   m_pUnitList->setCurrentIndex(m_pCalibrator->getMeasureUnit()); break;
        case CALIBRATOR_MODE_SOURCE:    m_pUnitList->setCurrentIndex(m_pCalibrator->getSourceUnit());  break;
        default:                        m_pUnitList->setCurrentIndex(CALIBRATOR_UNIT_UNKNOWN);         break;
    }

    m_pRemoteControlCheck->setLayoutDirection(Qt::RightToLeft);
    m_pRemoteControlCheck->setChecked(true);
    if (m_pCalibrator->getType() == CALIBRATOR_TYPE_TRXII)
    {
        m_pRemoteControlCheck->setVisible(false);
    }

    m_pErrorDialog->setWindowFlags(Qt::Drawer);
    m_pErrorDialog->setMinimumSize(700, 50);
    m_pErrorList->setReadOnly(true);

    connect(m_pCalibrator, &Calibrator::connected, this, &CalibratorManager::onConnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::disconnected, this, &CalibratorManager::onDisconnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::unitIsChanged, this, &CalibratorManager::onUnitChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsRequested , this, &CalibratorManager::onValueChanging, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsReceived, this, &CalibratorManager::onValueChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::error, this, &CalibratorManager::onCalibratorError, Qt::QueuedConnection);

    connect(this, &CalibratorManager::calibratorGetValue, m_pCalibrator, &Calibrator::getValue, Qt::QueuedConnection);

    connect(m_pSetValueButton, &QPushButton::clicked, this, &CalibratorManager::onSetValue, Qt::QueuedConnection);
    connect(this, &CalibratorManager::calibratorSetValue, m_pCalibrator, &Calibrator::setValue, Qt::QueuedConnection);
    connect(m_pStepDownButton, &QPushButton::clicked, this, &CalibratorManager::onStepDown, Qt::QueuedConnection);
    connect(this, &CalibratorManager::calibratorStepDown, m_pCalibrator, &Calibrator::stepDown, Qt::QueuedConnection);
    connect(m_pStepUpButton, &QPushButton::clicked, this, &CalibratorManager::onStepUp, Qt::QueuedConnection);
    connect(this, &CalibratorManager::calibratorStepUp, m_pCalibrator, &Calibrator::stepUp, Qt::QueuedConnection);

    connect(m_pModeList, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeList(int)), Qt::QueuedConnection);
    connect(m_pUnitList, SIGNAL(currentIndexChanged(int)), this, SLOT(onUnitList(int)), Qt::QueuedConnection);
    connect(this, &CalibratorManager::calibratorSetUnit, m_pCalibrator, &Calibrator::setUnit, Qt::QueuedConnection);

    connect(m_pErrorsButton, &QPushButton::clicked, this, &CalibratorManager::onErrorList, Qt::QueuedConnection);

    connect(m_pRemoteControlCheck, &QCheckBox::clicked, this, &CalibratorManager::onRemoveControl, Qt::QueuedConnection);
    connect(this, &CalibratorManager::calibratorRemoveControl, m_pCalibrator, &Calibrator::setRemoteControl, Qt::QueuedConnection);

    if (m_pCalibrator->isConnected() == true)
    {
        onConnect();
    }
    else
    {
        onDisconnect();
    }

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
}

// -------------------------------------------------------------------------------------------------------------------

void  CalibratorManager::onCalibratorError(QString text)
{
    QTime time;
    time.currentTime();

    QString error = QTime::currentTime().toString("hh:mm:ss.zzz - ") + text;

    m_pErrorList->append(error);
    m_pErrorsButton->setText( QString("List errors (%1)").arg(m_pErrorList->document()->lineCount() ) );

    if(isVisible() == false)
    {
        return;
    }

    QMessageBox msg;
    msg.setText(text);
    msg.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onConnect()
{
    enableInterface(true);

    emit calibratorGetValue();

    if (m_pCalibrator == nullptr)
    {
        return;
    }

    QString title = QString("c:%1 ").arg(m_index + 1) + m_pCalibrator->getName() + QString(" %1").arg(m_pCalibrator->getSerialNo()) ;

    setWindowTitle( title );
    m_pErrorDialog->setWindowTitle(title + tr(" : List errors"));
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onDisconnect()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    m_ready = true;

    QString title = QString("c:%1 (%2) - Disconnected").arg(m_index + 1).arg(m_pCalibrator->getPortName()) ;

    setWindowTitle(title);
    m_pErrorDialog->setWindowTitle(title + tr(" : List errors"));

    enableInterface(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onUnitChanged()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    return;

    m_pModeList->setCurrentIndex(m_pCalibrator->getMode());

    switch(m_pCalibrator->getMode())
    {
        case CALIBRATOR_MODE_MEASURE:   m_pUnitList->setCurrentIndex(m_pCalibrator->getMeasureUnit());      break;
        case CALIBRATOR_MODE_SOURCE:    m_pUnitList->setCurrentIndex(m_pCalibrator->getSourceUnit());       break;
        default:                        m_pUnitList->setCurrentIndex(CALIBRATOR_UNIT_UNKNOWN);              break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onValueChanging()
{
    enableInterface(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onValueChanged()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    m_ready = true;

    enableInterface(true);

    int measureUnit = m_pCalibrator->getMeasureUnit();
    int sourceUnit = m_pCalibrator->getSourceUnit();

    QString measureValue, sourceValue;

    if (measureUnit < 0 || measureUnit >= CALIBRATOR_UNIT_COUNT)
    {
        measureValue = QString::number(m_pCalibrator->getMeasureValue(), 10, 3);
    }
    else
    {
        measureValue = QString::number(m_pCalibrator->getMeasureValue(), 10, 3) + " " + CalibratorUnit[ measureUnit ];
    }

    if (sourceUnit < 0 || sourceUnit >= CALIBRATOR_UNIT_COUNT)
    {
        sourceValue = QString::number(m_pCalibrator->getSourceValue(), 10, 3);
    }
    else
    {
        sourceValue = QString::number(m_pCalibrator->getSourceValue(), 10, 3) + " " + CalibratorUnit[ sourceUnit ];
    }

    m_pMeasureEdit->setText(measureValue);
    m_pMeasureEdit->setCursorPosition(0);
    m_pSourceEdit->setText(sourceValue);
    m_pSourceEdit->setCursorPosition(0);
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

void CalibratorManager::getValue()
{
    emit calibratorGetValue();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::setValue(double value)
{
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

void CalibratorManager::onModeList(int mode)
{
    int unit = m_pUnitList->currentIndex();

    setUnit(mode, unit);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onUnitList(int unit)
{
    int mode = m_pModeList->currentIndex();

    setUnit(mode, unit);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManager::onErrorList()
{
    m_pErrorDialog->show();
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

    s.setValue(QString("%1Calibrator%2/port").arg(CALIBRATOR_OPTIONS_KEY).arg(m_index + 1), m_pCalibrator->getPortName());
    s.setValue(QString("%1Calibrator%2/type").arg(CALIBRATOR_OPTIONS_KEY).arg(m_index + 1), m_pCalibrator->getType());
}

// -------------------------------------------------------------------------------------------------------------------
