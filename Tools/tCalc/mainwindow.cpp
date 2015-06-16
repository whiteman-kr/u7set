#include "mainwindow.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    createDialog();
    initDialog();
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{

}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createDialog()
{
    // create elements of interface
    //

    // Thermistor
    //
    QGroupBox* trGroup = new QGroupBox(tr("Thermoresistor"));
    QVBoxLayout *trLayout = new QVBoxLayout;

    m_pTrList = new QComboBox(this);

    QHBoxLayout *tr_C_Layout = new QHBoxLayout;

    m_pTrDegreeRadio = new QRadioButton(this);
    m_pTrDegreeEdit = new QLineEdit(tr("0"), this);

    tr_C_Layout->addWidget(m_pTrDegreeRadio);
    tr_C_Layout->addWidget(m_pTrDegreeEdit);
    tr_C_Layout->addWidget(new QLabel(tr("°C"), this));
    tr_C_Layout->addStretch();

    QHBoxLayout *tr_Ohm_Layout = new QHBoxLayout;

    m_pTrElectricRadio = new QRadioButton(this);
    m_pTrElectricEdit = new QLineEdit(this);

    tr_Ohm_Layout->addWidget(m_pTrElectricRadio);
    tr_Ohm_Layout->addWidget(m_pTrElectricEdit);
    tr_Ohm_Layout->addWidget(new QLabel(tr("Ohm"), this));
    tr_Ohm_Layout->addStretch();

    trLayout->addWidget(m_pTrList);
    trLayout->addLayout(tr_C_Layout);
    trLayout->addLayout(tr_Ohm_Layout);

    trGroup->setLayout(trLayout);

    // Thermocouple
    //
    QGroupBox* tcGroup = new QGroupBox(tr("Thermocouple"));
    QVBoxLayout *tcLayout = new QVBoxLayout;

    m_pTcList = new QComboBox(this);

    QHBoxLayout *tc_C_Layout = new QHBoxLayout;

    m_pTcDegreeRadio = new QRadioButton(this);
    m_pTcDegreeEdit = new QLineEdit(tr("400"), this);

    tc_C_Layout->addWidget(m_pTcDegreeRadio);
    tc_C_Layout->addWidget(m_pTcDegreeEdit);
    tc_C_Layout->addWidget(new QLabel(tr("°C"), this));
    tc_C_Layout->addStretch();

    QHBoxLayout *tc_mV_Layout = new QHBoxLayout;

    m_pTcElectricRadio = new QRadioButton(this);
    m_pTcElectricEdit = new QLineEdit(this);

    tc_mV_Layout->addWidget(m_pTcElectricRadio);
    tc_mV_Layout->addWidget(m_pTcElectricEdit);
    tc_mV_Layout->addWidget(new QLabel(tr("mV"), this));
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

    m_pLinInRadio = new QRadioButton(this);
    m_pLinInValEdit = new QLineEdit(tr("12"), this);

    lin_inval_Layout->addWidget(m_pLinInRadio);
    lin_inval_Layout->addWidget(m_pLinInValEdit);
    lin_inval_Layout->addWidget(new QLabel(tr("in"), this));
    lin_inval_Layout->addStretch();

    QHBoxLayout *lin_outval_Layout = new QHBoxLayout;

    m_pLinOutRadio = new QRadioButton(this);
    m_pLinOutValEdit = new QLineEdit(tr("0"), this);

    lin_outval_Layout->addWidget(m_pLinOutRadio);
    lin_outval_Layout->addWidget(m_pLinOutValEdit);
    lin_outval_Layout->addWidget(new QLabel(tr("out"), this));
    lin_outval_Layout->addStretch();

    QHBoxLayout *lin_inrange_Layout = new QHBoxLayout;

    m_pLinInLowEdit = new QLineEdit(tr("4"), this);
    m_pLinInHighEdit = new QLineEdit(tr("20"), this);

    lin_inrange_Layout->addWidget(m_pLinInLowEdit);
    lin_inrange_Layout->addWidget(new QLabel(tr(".."), this));
    lin_inrange_Layout->addWidget(m_pLinInHighEdit);

    QHBoxLayout *lin_outrange_Layout = new QHBoxLayout;

    m_pLinOutLowEdit = new QLineEdit(tr("0"), this);
    m_pLinOutHighEdit = new QLineEdit(tr("100"), this);

    lin_outrange_Layout->addWidget(m_pLinOutLowEdit);
    lin_outrange_Layout->addWidget(new QLabel(tr(".."), this));
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

    QWidget *window = new QWidget(this);
    window->setLayout(mainLayout);

    setCentralWidget(window);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::initDialog()
{
    QRegExp rx( "^[-]{0,1}[0-9]*[.]{1}[0-9]*$" );
    QValidator *validator = new QRegExpValidator(rx, this);

    // init elements of interface
    //
    setWindowIcon(QIcon(":/icons/mdpi.png"));

    // Thermistor
    //
    m_pTrDegreeRadio->setChecked(true);

    for (int s = 0; s < INPUT_UNIT_SENSOR_COUNT; s++)
    {
        INPUT_UNIT_SENSOR ius = InputUnitSensorStr[s];
        if (ius.unit != INPUT_UNIT_OHM)
        {
            continue;
        }

        if (ius.sensor < 0 || ius.sensor >= INPUT_SENSOR_COUNT)
        {
            continue;
        }

        m_pTrList->addItem(InputSensorStr[ ius.sensor ], ius.sensor);
    }
    m_pTrList->setCurrentIndex(0);

    connect(m_pTrList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onTrSensorChanged);
    connect(m_pTrDegreeRadio, &QRadioButton::clicked, this, &MainWindow::onTrRadio);
    connect(m_pTrDegreeEdit, &QLineEdit::textChanged, this, &MainWindow::onTrValue);
    connect(m_pTrElectricRadio, &QRadioButton::clicked, this, &MainWindow::onTrRadio);
    connect(m_pTrElectricEdit, &QLineEdit::textChanged, this, &MainWindow::onTrValue);

    m_pTrDegreeEdit->setValidator(validator);
    m_pTrElectricEdit->setValidator(validator);

    conversionTr();

    // Thermocouple
    //
    m_pTcDegreeRadio->setChecked(true);

    for (int s = 0; s < INPUT_UNIT_SENSOR_COUNT; s++)
    {
        INPUT_UNIT_SENSOR ius = InputUnitSensorStr[s];
        if (ius.unit != INPUT_UNIT_MV)
        {
            continue;
        }

        if (ius.sensor < 0 || ius.sensor >= INPUT_SENSOR_COUNT)
        {
            continue;
        }

        m_pTcList->addItem(InputSensorStr[ ius.sensor ], ius.sensor);
    }
    m_pTcList->setCurrentIndex(0);


    connect(m_pTcList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onTcSensorChanged);
    connect(m_pTcDegreeRadio, &QRadioButton::clicked, this, &MainWindow::onTcRadio);
    connect(m_pTcDegreeEdit, &QLineEdit::textChanged, this, &MainWindow::onTcValue);
    connect(m_pTcElectricRadio, &QRadioButton::clicked, this, &MainWindow::onTcRadio);
    connect(m_pTcElectricEdit, &QLineEdit::textChanged, this, &MainWindow::onTcValue);

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

    connect(m_pLinInRadio, &QRadioButton::clicked, this, &MainWindow::onLinRadio);
    connect(m_pLinInValEdit, &QLineEdit::textChanged, this, &MainWindow::onLinValue);
    connect(m_pLinOutRadio, &QRadioButton::clicked, this, &MainWindow::onLinRadio);
    connect(m_pLinOutValEdit, &QLineEdit::textChanged, this, &MainWindow::onLinValue);
    connect(m_pLinInLowEdit, &QLineEdit::textChanged, this, &MainWindow::onLinValue);
    connect(m_pLinInHighEdit, &QLineEdit::textChanged, this, &MainWindow::onLinValue);
    connect(m_pLinOutLowEdit, &QLineEdit::textChanged, this, &MainWindow::onLinValue);
    connect(m_pLinOutHighEdit, &QLineEdit::textChanged, this, &MainWindow::onLinValue);

    conversionLin();

    // Select first dialog item
    //
    m_pTrList->setFocus();

}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::conversionTr()
{
    int index = m_pTrList->currentIndex();
    if (index == -1)
    {
        return;
    }

    int sensor = m_pTrList->itemData(index).toInt();
    if (sensor < 0 || sensor >= INPUT_SENSOR_COUNT)
    {
        return;
    }

    if (m_pTrDegreeRadio->isChecked() == true)
    {
        double val = conversion(m_pTrDegreeEdit->text().toDouble(), CT_PHYSICAL_TO_ELECTRIC, INPUT_UNIT_OHM, sensor);

        m_pTrDegreeEdit->setFocus();
        m_pTrDegreeEdit->setReadOnly(false);
        m_pTrElectricEdit->setText( QString::number(val, 3, 4) );
        m_pTrElectricEdit->setReadOnly(true);
    }

    if (m_pTrElectricRadio->isChecked() == true)
    {
        double val = conversion(m_pTrElectricEdit->text().toDouble(), CT_ELECTRIC_TO_PHYSICAL, INPUT_UNIT_OHM, sensor);

        m_pTrElectricEdit->setFocus();
        m_pTrElectricEdit->setReadOnly(false);
        m_pTrDegreeEdit->setText( QString::number(val, 3, 3) );
        m_pTrDegreeEdit->setReadOnly(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::conversionTc()
{
    int index = m_pTcList->currentIndex();
    if (index == -1)
    {
        return;
    }

    int sensor = m_pTcList->itemData(index).toInt();
    if (sensor < 0 || sensor >= INPUT_SENSOR_COUNT)
    {
        return;
    }

    if (m_pTcDegreeRadio->isChecked() == true)
    {
        double val = conversion(m_pTcDegreeEdit->text().toDouble(), CT_PHYSICAL_TO_ELECTRIC, INPUT_UNIT_MV, sensor);

        m_pTcDegreeEdit->setFocus();
        m_pTcDegreeEdit->setReadOnly(false);
        m_pTcElectricEdit->setText( QString::number(val, 3, 4) );
        m_pTcElectricEdit->setReadOnly(true);
    }

    if (m_pTcElectricRadio->isChecked() == true)
    {
        double val = conversion(m_pTcElectricEdit->text().toDouble(), CT_ELECTRIC_TO_PHYSICAL, INPUT_UNIT_MV, sensor);

        m_pTcElectricEdit->setFocus();
        m_pTcElectricEdit->setReadOnly(false);
        m_pTcDegreeEdit->setText( QString::number(val, 3, 3) );
        m_pTcDegreeEdit->setReadOnly(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::conversionLin()
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
