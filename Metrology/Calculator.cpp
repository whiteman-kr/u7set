#include "Calculator.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "Conversion.h"

#include "../lib/Types.h"

// -------------------------------------------------------------------------------------------------------------------

Calculator::Calculator(QWidget* parent) :
    QObject(parent),
    m_parentWidget(parent)
{
    createDialog();
    initDialog();
}

// -------------------------------------------------------------------------------------------------------------------

Calculator::~Calculator()
{
}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::createDialog()
{
    // create elements of interface
    //
    m_pDialog = new QDialog(m_parentWidget);

    // Thermistor
    //
    QGroupBox* trGroup = new QGroupBox(tr("Thermistor"));
    QVBoxLayout *trLayout = new QVBoxLayout;

    m_pTrList = new QComboBox(m_pDialog);

    QHBoxLayout *tr_C_Layout = new QHBoxLayout;

    m_pTrDegreeRadio = new QRadioButton(m_pDialog);
    m_pTrDegreeEdit = new QLineEdit(tr("0"), m_pDialog);

    tr_C_Layout->addWidget(m_pTrDegreeRadio);
    tr_C_Layout->addWidget(m_pTrDegreeEdit);
    tr_C_Layout->addWidget(new QLabel(tr("°C"), m_pDialog));
    tr_C_Layout->addStretch();

    QHBoxLayout *tr_Ohm_Layout = new QHBoxLayout;

    m_pTrElectricRadio = new QRadioButton(m_pDialog);
    m_pTrElectricEdit = new QLineEdit(m_pDialog);

    tr_Ohm_Layout->addWidget(m_pTrElectricRadio);
    tr_Ohm_Layout->addWidget(m_pTrElectricEdit);
    tr_Ohm_Layout->addWidget(new QLabel(tr("Ohm"), m_pDialog));
    tr_Ohm_Layout->addStretch();

    trLayout->addWidget(m_pTrList);
    trLayout->addLayout(tr_C_Layout);
    trLayout->addLayout(tr_Ohm_Layout);

    trGroup->setLayout(trLayout);

    // Thermocouple
    //
    QGroupBox* tcGroup = new QGroupBox(tr("Thermocouple"));
    QVBoxLayout *tcLayout = new QVBoxLayout;

    m_pTcList = new QComboBox(m_pDialog);

    QHBoxLayout *tc_C_Layout = new QHBoxLayout;

    m_pTcDegreeRadio = new QRadioButton(m_pDialog);
    m_pTcDegreeEdit = new QLineEdit(tr("400"), m_pDialog);

    tc_C_Layout->addWidget(m_pTcDegreeRadio);
    tc_C_Layout->addWidget(m_pTcDegreeEdit);
    tc_C_Layout->addWidget(new QLabel(tr("°C"), m_pDialog));
    tc_C_Layout->addStretch();

    QHBoxLayout *tc_mV_Layout = new QHBoxLayout;

    m_pTcElectricRadio = new QRadioButton(m_pDialog);
    m_pTcElectricEdit = new QLineEdit(m_pDialog);

    tc_mV_Layout->addWidget(m_pTcElectricRadio);
    tc_mV_Layout->addWidget(m_pTcElectricEdit);
    tc_mV_Layout->addWidget(new QLabel(tr("mV"), m_pDialog));
    tc_mV_Layout->addStretch();

    tcLayout->addWidget(m_pTcList);
    tcLayout->addLayout(tc_C_Layout);
    tcLayout->addLayout(tc_mV_Layout);

    tcGroup->setLayout(tcLayout);

    // Linearity
    //
    QGroupBox* linGroup = new QGroupBox(tr("Linearity"));
    QVBoxLayout *linLayout = new QVBoxLayout;

    QHBoxLayout *lin_inval_Layout = new QHBoxLayout;

    m_pLinInRadio = new QRadioButton(m_pDialog);
    m_pLinInValEdit = new QLineEdit(tr("2.5"), m_pDialog);

    lin_inval_Layout->addWidget(m_pLinInRadio);
    lin_inval_Layout->addWidget(m_pLinInValEdit);
    lin_inval_Layout->addWidget(new QLabel(tr("in"), m_pDialog));
    lin_inval_Layout->addStretch();

    QHBoxLayout *lin_outval_Layout = new QHBoxLayout;

    m_pLinOutRadio = new QRadioButton(m_pDialog);
    m_pLinOutValEdit = new QLineEdit(tr("0"), m_pDialog);

    lin_outval_Layout->addWidget(m_pLinOutRadio);
    lin_outval_Layout->addWidget(m_pLinOutValEdit);
    lin_outval_Layout->addWidget(new QLabel(tr("out"), m_pDialog));
    lin_outval_Layout->addStretch();

    QHBoxLayout *lin_inrange_Layout = new QHBoxLayout;

    m_pLinInLowEdit = new QLineEdit(tr("0"), m_pDialog);
    m_pLinInHighEdit = new QLineEdit(tr("5"), m_pDialog);

    lin_inrange_Layout->addWidget(m_pLinInLowEdit);
    lin_inrange_Layout->addWidget(new QLabel(tr(".."), m_pDialog));
    lin_inrange_Layout->addWidget(m_pLinInHighEdit);

    QHBoxLayout *lin_outrange_Layout = new QHBoxLayout;

    m_pLinOutLowEdit = new QLineEdit(tr("0"), m_pDialog);
    m_pLinOutHighEdit = new QLineEdit(tr("100"), m_pDialog);

    lin_outrange_Layout->addWidget(m_pLinOutLowEdit);
    lin_outrange_Layout->addWidget(new QLabel(tr(".."), m_pDialog));
    lin_outrange_Layout->addWidget(m_pLinOutHighEdit);

    linLayout->addLayout(lin_inval_Layout);
    linLayout->addLayout(lin_outval_Layout);
    linLayout->addLayout(lin_inrange_Layout);
    linLayout->addLayout(lin_outrange_Layout);

    linGroup->setLayout(linLayout);

    // Main
    //
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(trGroup);
    mainLayout->addWidget(tcGroup);
    mainLayout->addWidget(linGroup);
    mainLayout->addStretch();

    m_pDialog->setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::initDialog()
{
    QRegExp rx( "^[-]{0,1}[0-9]*[.]{1}[0-9]*$" );
    QValidator *validator = new QRegExpValidator(rx, m_pDialog);

    // init elements of interface
    //
    m_pDialog->setWindowFlags(Qt::Drawer);
    m_pDialog->setFixedWidth(230);
    m_pDialog->setWindowIcon(QIcon(":/icons/Calculator.png"));
    m_pDialog->setWindowTitle(tr("Metrological calculator"));

    // Thermistor
    //
    m_pTrDegreeRadio->setChecked(true);

    for (int s = 0; s < SENSOR_TYPE_BY_UNIT_COUNT; s++)
    {
        UnitSensorTypePair pair = SensorTypeByUnit[s];
        if (pair.unitID != E::InputUnit::Ohm)
        {
            continue;
        }

        if (pair.sensorType < 0 || pair.sensorType >= SENSOR_TYPE_COUNT)
        {
            continue;
        }

        m_pTrList->addItem(SensorTypeStr[ pair.sensorType ], pair.sensorType);
    }
    m_pTrList->setCurrentIndex(0);

    connect(m_pTrList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Calculator::onTrSensorTypeChanged);
    connect(m_pTrDegreeRadio, &QRadioButton::clicked, this, &Calculator::onTrRadio);
    connect(m_pTrDegreeEdit, &QLineEdit::textChanged, this, &Calculator::onTrValue);
    connect(m_pTrElectricRadio, &QRadioButton::clicked, this, &Calculator::onTrRadio);
    connect(m_pTrElectricEdit, &QLineEdit::textChanged, this, &Calculator::onTrValue);

    m_pTrDegreeEdit->setValidator(validator);
    m_pTrElectricEdit->setValidator(validator);

    conversionTr();

    // Thermocouple
    //
    m_pTcDegreeRadio->setChecked(true);

    for (int s = 0; s < SENSOR_TYPE_BY_UNIT_COUNT; s++)
    {
        UnitSensorTypePair pair = SensorTypeByUnit[s];
        if (pair.unitID != E::InputUnit::mV)
        {
            continue;
        }

        if (pair.sensorType < 0 || pair.sensorType >= SENSOR_TYPE_COUNT)
        {
            continue;
        }

        m_pTcList->addItem(SensorTypeStr[ pair.sensorType ], pair.sensorType);
    }
    m_pTcList->setCurrentIndex(0);


    connect(m_pTcList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Calculator::onTcSensorTypeChanged);
    connect(m_pTcDegreeRadio, &QRadioButton::clicked, this, &Calculator::onTcRadio);
    connect(m_pTcDegreeEdit, &QLineEdit::textChanged, this, &Calculator::onTcValue);
    connect(m_pTcElectricRadio, &QRadioButton::clicked, this, &Calculator::onTcRadio);
    connect(m_pTcElectricEdit, &QLineEdit::textChanged, this, &Calculator::onTcValue);

    m_pTcDegreeEdit->setValidator(validator);
    m_pTcElectricEdit->setValidator(validator);

    conversionTc();

    // Linearity
    //
    m_pLinInRadio->setChecked(true);

    m_pLinInValEdit->setValidator(validator);
    m_pLinOutValEdit->setValidator(validator);
    m_pLinInLowEdit->setValidator(validator);
    m_pLinInHighEdit->setValidator(validator);
    m_pLinOutLowEdit->setValidator(validator);
    m_pLinOutHighEdit->setValidator(validator);

    connect(m_pLinInRadio, &QRadioButton::clicked, this, &Calculator::onLinRadio);
    connect(m_pLinInValEdit, &QLineEdit::textChanged, this, &Calculator::onLinValue);
    connect(m_pLinOutRadio, &QRadioButton::clicked, this, &Calculator::onLinRadio);
    connect(m_pLinOutValEdit, &QLineEdit::textChanged, this, &Calculator::onLinValue);
    connect(m_pLinInLowEdit, &QLineEdit::textChanged, this, &Calculator::onLinValue);
    connect(m_pLinInHighEdit, &QLineEdit::textChanged, this, &Calculator::onLinValue);
    connect(m_pLinOutLowEdit, &QLineEdit::textChanged, this, &Calculator::onLinValue);
    connect(m_pLinOutHighEdit, &QLineEdit::textChanged, this, &Calculator::onLinValue);

    conversionLin();

    // Select first dialog item
    //
    m_pTrList->setFocus();

}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::conversionTr()
{
    int index = m_pTrList->currentIndex();
    if (index == -1)
    {
        return;
    }

    E::SensorType sensorType = static_cast<E::SensorType>(m_pTrList->itemData(index).toInt());
    if (sensorType < 0 || sensorType >= SENSOR_TYPE_COUNT)
    {
        return;
    }

    if (m_pTrDegreeRadio->isChecked() == true)
    {
        double val = conversion(m_pTrDegreeEdit->text().toDouble(), CT_PHYSICAL_TO_ELECTRIC, E::InputUnit::Ohm, sensorType);

        m_pTrDegreeEdit->setFocus();
        m_pTrDegreeEdit->setReadOnly(false);
        m_pTrElectricEdit->setText( QString::number(val, 3, 4) );
        m_pTrElectricEdit->setReadOnly(true);
    }

    if (m_pTrElectricRadio->isChecked() == true)
    {
        double val = conversion(m_pTrElectricEdit->text().toDouble(), CT_ELECTRIC_TO_PHYSICAL, E::InputUnit::Ohm, sensorType);

        m_pTrElectricEdit->setFocus();
        m_pTrElectricEdit->setReadOnly(false);
        m_pTrDegreeEdit->setText( QString::number(val, 3, 3) );
        m_pTrDegreeEdit->setReadOnly(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::conversionTc()
{
    int index = m_pTcList->currentIndex();
    if (index == -1)
    {
        return;
    }

    E::SensorType sensorType = static_cast<E::SensorType>(m_pTcList->itemData(index).toInt());
    if (sensorType < 0 || sensorType >= SENSOR_TYPE_COUNT)
    {
        return;
    }

    if (m_pTcDegreeRadio->isChecked() == true)
    {
        double val = conversion(m_pTcDegreeEdit->text().toDouble(), CT_PHYSICAL_TO_ELECTRIC, E::InputUnit::mV, sensorType);

        m_pTcDegreeEdit->setFocus();
        m_pTcDegreeEdit->setReadOnly(false);
        m_pTcElectricEdit->setText( QString::number(val, 3, 4) );
        m_pTcElectricEdit->setReadOnly(true);
    }

    if (m_pTcElectricRadio->isChecked() == true)
    {
        double val = conversion(m_pTcElectricEdit->text().toDouble(), CT_ELECTRIC_TO_PHYSICAL, E::InputUnit::mV, sensorType);

        m_pTcElectricEdit->setFocus();
        m_pTcElectricEdit->setReadOnly(false);
        m_pTcDegreeEdit->setText( QString::number(val, 3, 3) );
        m_pTcDegreeEdit->setReadOnly(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::conversionLin()
{
    double irl = m_pLinInLowEdit->text().toDouble();
    double irh = m_pLinInHighEdit->text().toDouble();
    double orl = m_pLinOutLowEdit->text().toDouble();
    double orh = m_pLinOutHighEdit->text().toDouble();

    if (m_pLinInRadio->isChecked() == true)
    {
        double val = (m_pLinInValEdit->text().toDouble() - irl)*(orh-orl)/(irh-irl)+orl;

        m_pLinInValEdit->setReadOnly(false);
        m_pLinOutValEdit->setText( QString::number(val, 10, 4) );
        m_pLinOutValEdit->setReadOnly(true);
    }

    if (m_pLinOutRadio->isChecked() == true)
    {
        double val = (m_pLinOutValEdit->text().toDouble() - orl)*(irh-irl)/(orh-orl)+irl;

        m_pLinOutValEdit->setReadOnly(false);
        m_pLinInValEdit->setText( QString::number(val, 10, 4) );
        m_pLinInValEdit->setReadOnly(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------
