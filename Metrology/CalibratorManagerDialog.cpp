#include "CalibratorManagerDialog.h"
#include "ui_CalibratorManagerDialog.h"

#include <QTime>
#include <QTimer>
#include <QMessageBox>

// -------------------------------------------------------------------------------------------------------------------

CalibratorManagerDialog::CalibratorManagerDialog(Calibrator* pCalibrator, QWidget *parent) :
    QDialog(parent),
    m_pCalibrator( pCalibrator ),
    ui(new Ui::QCalibratorManagerDialog)
{
    ui->setupUi(this);

    if (m_pCalibrator == nullptr)
    {
        return;
    }

    setWindowFlags(Qt::Drawer | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

    for (int m = 0; m < CALIBRATOR_MODE_COUNT; m++)
    {
        ui->modeList->addItem( CalibratorMode[m] );
    }
    ui->modeList->setCurrentIndex(m_pCalibrator->getMode());


    for (int u = 0; u < CALIBRATOR_UNIT_COUNT; u++)
    {
        ui->unitList->addItem(CalibratorUnit[u]);
    }

    switch(m_pCalibrator->getMode())
    {
        case CALIBRATOR_MODE_MEASURE:   ui->unitList->setCurrentIndex(m_pCalibrator->getMeasureUnit()); break;
        case CALIBRATOR_MODE_SOURCE:    ui->unitList->setCurrentIndex(m_pCalibrator->getSourceUnit());  break;
        default:                        ui->unitList->setCurrentIndex(CALIBRATOR_UNIT_UNKNOWN);         break;
    }

    ui->remoteCheck->setChecked(true);

    if (m_pCalibrator->getType() == CALIBRATOR_TYPE_TRXII)
    {
        ui->remoteCheck->setVisible(false);
    }

    ui->errorList->setMinimumContentsLength(150);

    connect(m_pCalibrator, &Calibrator::connected, this, &CalibratorManagerDialog::onConnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::disconnected, this, &CalibratorManagerDialog::onDisconnect, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::unitIsChanged, this, &CalibratorManagerDialog::unitIsChanged, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsRequested , this, &CalibratorManagerDialog::changingValue, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::valueIsReceived, this, &CalibratorManagerDialog::updateValue, Qt::QueuedConnection);
    connect(m_pCalibrator, &Calibrator::error_control, this, &CalibratorManagerDialog::onCalibratorError, Qt::QueuedConnection);

    connect(ui->modeList, SIGNAL(currentIndexChanged(int)), this, SLOT(modeList(int)), Qt::QueuedConnection);
    connect(ui->unitList, SIGNAL(currentIndexChanged(int)), this, SLOT(unitList(int)), Qt::QueuedConnection);
    connect(this, &CalibratorManagerDialog::calibratorSetUnit, m_pCalibrator, &Calibrator::setUnit, Qt::QueuedConnection);

    connect(ui->setButton, &QPushButton::clicked, this, &CalibratorManagerDialog::setValue, Qt::QueuedConnection);
    connect(this, &CalibratorManagerDialog::calibratorSetValue, m_pCalibrator, &Calibrator::setValue, Qt::QueuedConnection);

    connect(this, &CalibratorManagerDialog::calibratorGetValue, m_pCalibrator, &Calibrator::getValue, Qt::QueuedConnection);

    connect(ui->stepDownButton, &QPushButton::clicked, m_pCalibrator, &Calibrator::stepDown, Qt::QueuedConnection);
    connect(ui->stepUpButton, &QPushButton::clicked, m_pCalibrator, &Calibrator::stepUp, Qt::QueuedConnection);

    connect(ui->resetButton, &QPushButton::clicked, this, &CalibratorManagerDialog::reset, Qt::QueuedConnection);
    connect(this, &CalibratorManagerDialog::calibratorReset, m_pCalibrator, &Calibrator::reset, Qt::QueuedConnection);

    connect(ui->remoteCheck, &QCheckBox::clicked, this, &CalibratorManagerDialog::removeControl, Qt::QueuedConnection);
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

CalibratorManagerDialog::~CalibratorManagerDialog()
{
    delete ui;
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::enableItemCtrl(bool enable)
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    if (enable == true && m_pCalibrator->isConnected() == false)
    {
        enable = false;
    }

    ui->measureEdit->setEnabled(enable);
    ui->sourceEdit->setEnabled(enable);
    ui->valueEdit->setEnabled(enable);
    ui->setButton->setEnabled(enable);
    ui->modeList->setEnabled(enable);
    ui->unitList->setEnabled(enable);
    ui->stepDownButton->setEnabled(enable);
    ui->stepUpButton->setEnabled(enable);
    ui->resetButton->setEnabled(enable);
    ui->remoteCheck->setEnabled(enable);

}

// -------------------------------------------------------------------------------------------------------------------

void  CalibratorManagerDialog::onCalibratorError(QString err)
{
    QTime time;
    time.currentTime();

    QString error = QTime::currentTime().toString("hh:mm:ss.zzz - ") + err;
    ui->errorList->addItem(error);
    ui->errorList->setCurrentIndex( ui->errorList->count() - 1 );
    ui->errorList->setToolTip(error);

    QSizePolicy  SizePolicy;
    SizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);

    ui->errorList->setSizePolicy(SizePolicy);


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
    enableItemCtrl(true);

    emit calibratorGetValue();

    if (m_pCalibrator == nullptr)
    {
        return;
    }

    setWindowTitle(QString("c:%1 ").arg(m_pCalibrator->getIndex() + 1) + m_pCalibrator->getName() + QString(" %1").arg(m_pCalibrator->getSerialNo()) );
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::onDisconnect()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    setWindowTitle(QString("c:%1 - Disconnected").arg(m_pCalibrator->getIndex() + 1));

    enableItemCtrl(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::unitIsChanged()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    ui->modeList->setCurrentIndex(m_pCalibrator->getMode());

    switch(m_pCalibrator->getMode())
    {
        case CALIBRATOR_MODE_MEASURE:   ui->unitList->setCurrentIndex(m_pCalibrator->getMeasureUnit()); break;
        case CALIBRATOR_MODE_SOURCE:    ui->unitList->setCurrentIndex(m_pCalibrator->getSourceUnit());  break;
        default:                        ui->unitList->setCurrentIndex(CALIBRATOR_UNIT_UNKNOWN);         break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::changingValue()
{
    enableItemCtrl(false);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::updateValue()
{
    if (m_pCalibrator == nullptr)
    {
        return;
    }

    enableItemCtrl(true);

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

    ui->measureEdit->setText(measureValue);
    ui->measureEdit->setCursorPosition(0);
    ui->sourceEdit->setText(sourceValue);
    ui->sourceEdit->setCursorPosition(0);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::modeList(int mode)
{
    int unit = ui->unitList->currentIndex();

    if (mode == -1 || unit == -1)
    {
        return;
    }

    emit calibratorSetUnit(mode, unit);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::unitList(int unit)
{
    int mode = ui->modeList->currentIndex();

    if (mode == -1 || unit == -1)
    {
        return;
    }

    emit calibratorSetUnit(mode, unit);
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::setValue()
{
    QString value = ui->valueEdit->text();

    if (value.isEmpty() == true)
    {
        QMessageBox msg;
        msg.setText(tr("Value is empty!"));
        msg.exec();

        return;
    }

    emit calibratorSetValue(value.toDouble());
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::reset()
{
    emit calibratorReset( CALIBRATOR_RESET_HARD );
}

// -------------------------------------------------------------------------------------------------------------------

void CalibratorManagerDialog::removeControl()
{
    emit calibratorRemoveControl( ui->remoteCheck->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------
