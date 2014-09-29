#include "CalibratorManagerDialog.h"

#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDesktopWidget>

// -------------------------------------------------------------------------------------------------------------------

CalibratorManagerDialog::CalibratorManagerDialog(Calibrator* pCalibrator, QWidget *parent) :
    QDialog(parent),
    m_pCalibrator( pCalibrator )
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    createInterfaceItems();
    initInterfaceItems();
}

// -------------------------------------------------------------------------------------------------------------------

CalibratorManagerDialog::~CalibratorManagerDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::createInterfaceItems()
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

    setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::initInterfaceItems()
{
    // init elements of interface
    //
    QDesktopWidget desktop;
    int width = desktop.geometry().width();
    setGeometry(width - 210, 0, 190, 350);

    setWindowFlags(Qt::Drawer | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

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

    m_pErrorList = new QTextEdit;
    m_pErrorList->setMinimumSize(700, 50);
    m_pErrorList->setReadOnly(true);

    m_pRemoteControlCheck->setLayoutDirection(Qt::RightToLeft);
    m_pRemoteControlCheck->setChecked(true);
    if (m_pCalibrator->getType() == CALIBRATOR_TYPE_TRXII)
    {
        m_pRemoteControlCheck->setVisible(false);
    }

    connect(m_pCalibrator, &Calibrator::connected, this, &CalibratorManagerDialog::onConnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::disconnected, this, &CalibratorManagerDialog::onDisconnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::unitIsChanged, this, &CalibratorManagerDialog::onUnitChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsRequested , this, &CalibratorManagerDialog::onValueChanging, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsReceived, this, &CalibratorManagerDialog::onValueChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::error_control, this, &CalibratorManagerDialog::onCalibratorError, Qt::QueuedConnection);

    connect(m_pSetValueButton, &QPushButton::clicked, this, &CalibratorManagerDialog::onSetValue, Qt::QueuedConnection);
    connect(this, &CalibratorManagerDialog::calibratorSetValue, m_pCalibrator, &Calibrator::setValue, Qt::QueuedConnection);

    connect(this, &CalibratorManagerDialog::calibratorGetValue, m_pCalibrator, &Calibrator::getValue, Qt::QueuedConnection);

    connect(m_pStepDownButton, &QPushButton::clicked, m_pCalibrator, &Calibrator::stepDown, Qt::QueuedConnection);
    connect(m_pStepUpButton, &QPushButton::clicked, m_pCalibrator, &Calibrator::stepUp, Qt::QueuedConnection);

    connect(m_pModeList, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeList(int)), Qt::QueuedConnection);
    connect(m_pUnitList, SIGNAL(currentIndexChanged(int)), this, SLOT(onUnitList(int)), Qt::QueuedConnection);
    connect(this, &CalibratorManagerDialog::calibratorSetUnit, m_pCalibrator, &Calibrator::setUnit, Qt::QueuedConnection);

    connect(m_pErrorsButton, &QPushButton::clicked, this, &CalibratorManagerDialog::onErrorList, Qt::QueuedConnection);

    connect(m_pRemoteControlCheck, &QCheckBox::clicked, this, &CalibratorManagerDialog::onRemoveControl, Qt::QueuedConnection);
    connect(this, &CalibratorManagerDialog::calibratorRemoveControl, m_pCalibrator, &Calibrator::setRemoteControl, Qt::QueuedConnection);

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

void CalibratorManagerDialog::enableInterfaceItems(bool enable)
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

void  CalibratorManagerDialog::onCalibratorError(QString err)
{
    QTime time;
    time.currentTime();

    QString error = QTime::currentTime().toString("hh:mm:ss.zzz - ") + err;

    m_pErrorList->append(error);
    m_pErrorsButton->setText( QString("List errors (%1)").arg(m_pErrorList->document()->lineCount() ) );

    if(isVisible() == false)
    {
        return;
    }

    QMessageBox msg;
    msg.setText(err);
    msg.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onConnect()
{
    enableInterfaceItems(true);

    emit calibratorGetValue();

    if (m_pCalibrator == nullptr)
    {
        return;
    }

    QString title = QString("c:%1 ").arg(m_pCalibrator->getIndex() + 1) + m_pCalibrator->getName() + QString(" %1").arg(m_pCalibrator->getSerialNo()) ;

    setWindowTitle( title );
    m_pErrorList->setWindowTitle(title + tr(" : List errors"));
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onDisconnect()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    QString title = QString("c:%1 - Disconnected").arg(m_pCalibrator->getIndex() + 1);

    setWindowTitle(title);
    m_pErrorList->setWindowTitle(title + tr(" : List errors"));

    enableInterfaceItems(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onUnitChanged()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    m_pModeList->setCurrentIndex(m_pCalibrator->getMode());

    switch(m_pCalibrator->getMode())
    {
        case CALIBRATOR_MODE_MEASURE:   m_pUnitList->setCurrentIndex(m_pCalibrator->getMeasureUnit()); break;
        case CALIBRATOR_MODE_SOURCE:    m_pUnitList->setCurrentIndex(m_pCalibrator->getSourceUnit());  break;
        default:                        m_pUnitList->setCurrentIndex(CALIBRATOR_UNIT_UNKNOWN);         break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onValueChanging()
{
    enableInterfaceItems(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onValueChanged()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    enableInterfaceItems(true);

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

void CalibratorManagerDialog::onSetValue()
{
    QString value = m_pValueEdit->text();

    if (value.isEmpty() == true)
    {
        return;
    }

    emit calibratorSetValue(value.toDouble());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onModeList(int mode)
{
    int unit = m_pUnitList->currentIndex();

    if (mode == -1 || unit == -1)
    {
        return;
    }

    emit calibratorSetUnit(mode, unit);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onUnitList(int unit)
{
    int mode = m_pModeList->currentIndex();

    if (mode == -1 || unit == -1)
    {
        return;
    }

    emit calibratorSetUnit(mode, unit);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onErrorList()
{
    if (m_pErrorList == nullptr)
    {
        return;
    }

    m_pErrorList->show();
    m_pErrorList->activateWindow();
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onRemoveControl()
{
    emit calibratorRemoveControl( m_pRemoteControlCheck->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------
