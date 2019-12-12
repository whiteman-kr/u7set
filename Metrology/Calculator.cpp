#include "Calculator.h"

#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------

Calculator::Calculator(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	initDialog();
}

// -------------------------------------------------------------------------------------------------------------------

Calculator::~Calculator()
{
	if (m_digitFont != nullptr)
	{
		delete m_digitFont;
		m_digitFont = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::createInterface()
{
	// create elements of interface
	//

	m_digitFont = new QFont("Arial", 16, 2);

	// Thermistor
	//
	QGroupBox* trGroup = new QGroupBox(tr("Thermistor"));
	QVBoxLayout *trLayout = new QVBoxLayout;

	m_pTrList = new QComboBox(this);

	QHBoxLayout *tr_C_Layout = new QHBoxLayout;

	m_pTrDegreeRadio = new QRadioButton(this);
	m_pTrDegreeEdit = new QLineEdit(tr("100"), this);
	QLabel* pTrDegreeLabel = new QLabel(tr("°C"), this);
	pTrDegreeLabel->setFixedWidth(30);
	m_pTrDegreeEdit->setFont(*m_digitFont);

	tr_C_Layout->addWidget(m_pTrDegreeRadio);
	tr_C_Layout->addWidget(m_pTrDegreeEdit);
	tr_C_Layout->addWidget(pTrDegreeLabel);
	tr_C_Layout->addStretch();

	QHBoxLayout *tr_Ohm_Layout = new QHBoxLayout;

	m_pTrElectricRadio = new QRadioButton(this);
	m_pTrElectricEdit = new QLineEdit(this);
	QLabel* pTrElectricLabel = new QLabel(tr("Ohm"), this);
	pTrElectricLabel->setFixedWidth(30);
	m_pTrElectricEdit->setFont(*m_digitFont);

	tr_Ohm_Layout->addWidget(m_pTrElectricRadio);
	tr_Ohm_Layout->addWidget(m_pTrElectricEdit);
	tr_Ohm_Layout->addWidget(pTrElectricLabel);
	tr_Ohm_Layout->addStretch();


	m_tr_R0_Layout = new QHBoxLayout;

	m_pTrR0Edit = new QLineEdit(tr("100"), this);
	QLabel* pTrR0Label = new QLabel(tr(""), this);
	pTrR0Label->setFixedWidth(30);
	m_pTrR0Edit->setFont(*m_digitFont);

	m_tr_R0_Layout->addWidget(new QLabel(tr("R0"), this));
	m_tr_R0_Layout->addWidget(m_pTrR0Edit);
	m_tr_R0_Layout->addWidget(pTrR0Label);
	m_tr_R0_Layout->addStretch();


	trLayout->addWidget(m_pTrList);
	trLayout->addLayout(tr_C_Layout);
	trLayout->addLayout(tr_Ohm_Layout);
	trLayout->addLayout(m_tr_R0_Layout);

	trGroup->setLayout(trLayout);

	// Thermocouple
	//
	QGroupBox* tcGroup = new QGroupBox(tr("Thermocouple"));
	QVBoxLayout *tcLayout = new QVBoxLayout;

	m_pTcList = new QComboBox(this);

	QHBoxLayout *tc_C_Layout = new QHBoxLayout;

	m_pTcDegreeRadio = new QRadioButton(this);
	m_pTcDegreeEdit = new QLineEdit(tr("400"), this);
	QLabel* pTcDegreeLabel = new QLabel(tr("°C"), this);
	pTcDegreeLabel->setFixedWidth(30);
	m_pTcDegreeEdit->setFont(*m_digitFont);

	tc_C_Layout->addWidget(m_pTcDegreeRadio);
	tc_C_Layout->addWidget(m_pTcDegreeEdit);
	tc_C_Layout->addWidget(pTcDegreeLabel);
	tc_C_Layout->addStretch();

	QHBoxLayout *tc_mV_Layout = new QHBoxLayout;

	m_pTcElectricRadio = new QRadioButton(this);
	m_pTcElectricEdit = new QLineEdit(this);
	QLabel* pTcElectricLabel = new QLabel(tr("mV"), this);
	pTcElectricLabel->setFixedWidth(30);
	m_pTcElectricEdit->setFont(*m_digitFont);

	tc_mV_Layout->addWidget(m_pTcElectricRadio);
	tc_mV_Layout->addWidget(m_pTcElectricEdit);
	tc_mV_Layout->addWidget(pTcElectricLabel);
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
	m_pLinInValEdit = new QLineEdit(tr("2.5"), this);
	QLabel* pLinInValLabel = new QLabel(tr("In"), this);
	pLinInValLabel->setFixedWidth(30);
	m_pLinInValEdit->setFont(*m_digitFont);

	lin_inval_Layout->addWidget(m_pLinInRadio);
	lin_inval_Layout->addWidget(m_pLinInValEdit);
	lin_inval_Layout->addWidget(pLinInValLabel);
	lin_inval_Layout->addStretch();

	QHBoxLayout *lin_outval_Layout = new QHBoxLayout;

	m_pLinOutRadio = new QRadioButton(this);
	m_pLinOutValEdit = new QLineEdit(tr("0"), this);
	QLabel* pLinOutValLabel = new QLabel(tr("Out"), this);
	pLinOutValLabel->setFixedWidth(30);
	m_pLinOutValEdit->setFont(*m_digitFont);

	lin_outval_Layout->addWidget(m_pLinOutRadio);
	lin_outval_Layout->addWidget(m_pLinOutValEdit);
	lin_outval_Layout->addWidget(pLinOutValLabel);
	lin_outval_Layout->addStretch();

	QHBoxLayout *lin_inrange_Layout = new QHBoxLayout;

	m_pLinInLowEdit = new QLineEdit(tr("0"), this);
	m_pLinInHighEdit = new QLineEdit(tr("5"), this);

	lin_inrange_Layout->addWidget(new QLabel(tr("min"), this));
	lin_inrange_Layout->addWidget(m_pLinInLowEdit);
	lin_inrange_Layout->addWidget(new QLabel(tr("max"), this));
	lin_inrange_Layout->addWidget(m_pLinInHighEdit);

	QHBoxLayout *lin_outrange_Layout = new QHBoxLayout;

	m_pLinOutLowEdit = new QLineEdit(tr("0"), this);
	m_pLinOutHighEdit = new QLineEdit(tr("100"), this);

	lin_outrange_Layout->addWidget(new QLabel(tr("min"), this));
	lin_outrange_Layout->addWidget(m_pLinOutLowEdit);
	lin_outrange_Layout->addWidget(new QLabel(tr("max"), this));
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

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void Calculator::initDialog()
{
	QRegExp rx("^[-]{0,1}[0-9]*[.]{1}[0-9]*$");
	QValidator *validator = new QRegExpValidator(rx, this);

	// init elements of interface
	//
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setFixedWidth(230);
	setWindowIcon(QIcon(":/icons/Calculator.png"));
	setWindowTitle(tr("Metrological calculator"));

	QMetaEnum mst = QMetaEnum::fromType<E::SensorType>();

	// Thermistor
	//
	m_pTrDegreeRadio->setChecked(true);

	for (int s = 0; s < SENSOR_TYPE_BY_UNIT_COUNT; s++)
	{
		UnitSensorTypePair pair = SensorTypeByUnit[s];
		if (pair.unitID != E::ElectricUnit::Ohm)
		{
			continue;
		}

		if (pair.sensorType < 0 || pair.sensorType >= mst.keyCount())
		{
			continue;
		}

		if (pair.sensorType == E::SensorType::NoSensor)
		{
			continue;
		}

		m_pTrList->addItem(mst.key(pair.sensorType), pair.sensorType);
	}
	m_pTrList->setCurrentIndex(0);

	connect(m_pTrList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Calculator::onTrSensorTypeChanged);
	connect(m_pTrDegreeRadio, &QRadioButton::clicked, this, &Calculator::onTrRadio);
	connect(m_pTrDegreeEdit, &QLineEdit::textChanged, this, &Calculator::onTrValue);
	connect(m_pTrElectricRadio, &QRadioButton::clicked, this, &Calculator::onTrRadio);
	connect(m_pTrElectricEdit, &QLineEdit::textChanged, this, &Calculator::onTrValue);
	connect(m_pTrR0Edit, &QLineEdit::textChanged, this, &Calculator::onTrValue);

	m_pTrDegreeEdit->setValidator(validator);
	m_pTrElectricEdit->setValidator(validator);
	m_pTrR0Edit->setValidator(validator);

	conversionTr();

	// Thermocouple
	//
	m_pTcDegreeRadio->setChecked(true);

	for (int s = 0; s < SENSOR_TYPE_BY_UNIT_COUNT; s++)
	{
		UnitSensorTypePair pair = SensorTypeByUnit[s];
		if (pair.unitID != E::ElectricUnit::mV)
		{
			continue;
		}

		if (pair.sensorType < 0 || pair.sensorType >= mst.keyCount())
		{
			continue;
		}

		if (pair.sensorType == E::SensorType::NoSensor)
		{
			continue;
		}

		m_pTcList->addItem(mst.key(pair.sensorType), pair.sensorType);
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

	switch (sensorType)
	{
		case E::SensorType::Ohm_Pt_a_391:
		case E::SensorType::Ohm_Pt_a_385:
		case E::SensorType::Ohm_Cu_a_428:
		case E::SensorType::Ohm_Cu_a_426:
		case E::SensorType::Ohm_Ni_a_617:	m_pTrR0Edit->setEnabled(true);		break;
		default:							m_pTrR0Edit->setEnabled(false);		break;
	}

	if (m_pTrDegreeRadio->isChecked() == true)
	{
		double val = conversion(m_pTrDegreeEdit->text().toDouble(), CT_PHYSICAL_TO_ELECTRIC, E::ElectricUnit::Ohm, sensorType, m_pTrR0Edit->text().toDouble());

		m_pTrDegreeEdit->setReadOnly(false);
		m_pTrElectricEdit->setText(QString::number(val, 'f', 4));
		m_pTrElectricEdit->setReadOnly(true);
	}

	if (m_pTrElectricRadio->isChecked() == true)
	{
		double val = conversion(m_pTrElectricEdit->text().toDouble(), CT_ELECTRIC_TO_PHYSICAL, E::ElectricUnit::Ohm, sensorType, m_pTrR0Edit->text().toDouble());

		m_pTrElectricEdit->setReadOnly(false);
		m_pTrDegreeEdit->setText(QString::number(val, 'f', 4));
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

	if (m_pTcDegreeRadio->isChecked() == true)
	{
		double val = conversion(m_pTcDegreeEdit->text().toDouble(), CT_PHYSICAL_TO_ELECTRIC, E::ElectricUnit::mV, sensorType);

		m_pTcDegreeEdit->setFocus();
		m_pTcDegreeEdit->setReadOnly(false);
		m_pTcElectricEdit->setText(QString::number(val, 'f', 4));
		m_pTcElectricEdit->setReadOnly(true);
	}

	if (m_pTcElectricRadio->isChecked() == true)
	{
		double val = conversion(m_pTcElectricEdit->text().toDouble(), CT_ELECTRIC_TO_PHYSICAL, E::ElectricUnit::mV, sensorType);

		m_pTcElectricEdit->setFocus();
		m_pTcElectricEdit->setReadOnly(false);
		m_pTcDegreeEdit->setText(QString::number(val, 'f', 4));
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
		m_pLinOutValEdit->setText(QString::number(val, 'f', 4));
		m_pLinOutValEdit->setReadOnly(true);
	}

	if (m_pLinOutRadio->isChecked() == true)
	{
		double val = (m_pLinOutValEdit->text().toDouble() - orl)*(irh-irl)/(orh-orl)+irl;

		m_pLinOutValEdit->setReadOnly(false);
		m_pLinInValEdit->setText(QString::number(val, 'f', 4));
		m_pLinInValEdit->setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------
