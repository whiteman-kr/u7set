#include "DialogCalculator.h"

#include "Calibrator.h"

// -------------------------------------------------------------------------------------------------------------------

DialogCalculator::DialogCalculator(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	initDialog();
}

// -------------------------------------------------------------------------------------------------------------------

DialogCalculator::~DialogCalculator()
{
	if (m_digitFont != nullptr)
	{
		delete m_digitFont;
		m_digitFont = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::createInterface()
{
	// create elements of interface
	//

	m_digitFont = new QFont("Arial", 16, 2);

	// Linearity
	//
	QGroupBox* linGroup = new QGroupBox(tr("Linearity"));
	QVBoxLayout* linLayout = new QVBoxLayout;

	QHBoxLayout* lin_inval_Layout = new QHBoxLayout;

	m_pLinInRadio = new QRadioButton(this);
	m_pLinInValEdit = new QLineEdit("2.5", this);
	QLabel* pLinInValLabel = new QLabel(tr("In"), this);
	pLinInValLabel->setFixedWidth(30);
	m_pLinInValEdit->setFont(*m_digitFont);

	lin_inval_Layout->addWidget(m_pLinInRadio);
	lin_inval_Layout->addWidget(m_pLinInValEdit);
	lin_inval_Layout->addWidget(pLinInValLabel);
	lin_inval_Layout->addStretch();

	QHBoxLayout* lin_outval_Layout = new QHBoxLayout;

	m_pLinOutRadio = new QRadioButton(this);
	m_pLinOutValEdit = new QLineEdit("0", this);
	QLabel* pLinOutValLabel = new QLabel(tr("Out"), this);
	pLinOutValLabel->setFixedWidth(30);
	m_pLinOutValEdit->setFont(*m_digitFont);

	lin_outval_Layout->addWidget(m_pLinOutRadio);
	lin_outval_Layout->addWidget(m_pLinOutValEdit);
	lin_outval_Layout->addWidget(pLinOutValLabel);
	lin_outval_Layout->addStretch();

	QHBoxLayout* lin_inrange_Layout = new QHBoxLayout;

	m_pLinInLowEdit = new QLineEdit("0", this);
	m_pLinInHighEdit = new QLineEdit("5", this);

	lin_inrange_Layout->addWidget(new QLabel(tr("min"), this));
	lin_inrange_Layout->addWidget(m_pLinInLowEdit);
	lin_inrange_Layout->addWidget(new QLabel(tr("max"), this));
	lin_inrange_Layout->addWidget(m_pLinInHighEdit);

	QHBoxLayout* lin_outrange_Layout = new QHBoxLayout;

	m_pLinOutLowEdit = new QLineEdit("0", this);
	m_pLinOutHighEdit = new QLineEdit("100", this);

	lin_outrange_Layout->addWidget(new QLabel(tr("min"), this));
	lin_outrange_Layout->addWidget(m_pLinOutLowEdit);
	lin_outrange_Layout->addWidget(new QLabel(tr("max"), this));
	lin_outrange_Layout->addWidget(m_pLinOutHighEdit);

	linLayout->addLayout(lin_inval_Layout);
	linLayout->addLayout(lin_outval_Layout);
	linLayout->addLayout(lin_inrange_Layout);
	linLayout->addLayout(lin_outrange_Layout);

	linGroup->setLayout(linLayout);

	// Thermistor
	//
	QGroupBox* trGroup = new QGroupBox(tr("Thermistor - GOST 6651-2009"));
	QVBoxLayout* trLayout = new QVBoxLayout;

	m_pTrList = new QComboBox(this);

	QHBoxLayout* tr_C_Layout = new QHBoxLayout;

	m_pTrDegreeRadio = new QRadioButton(this);
	m_pTrDegreeEdit = new QLineEdit("100", this);
	QLabel* pTrDegreeLabel = new QLabel(tr("°C"), this);
	pTrDegreeLabel->setFixedWidth(30);
	m_pTrDegreeEdit->setFont(*m_digitFont);

	tr_C_Layout->addWidget(m_pTrDegreeRadio);
	tr_C_Layout->addWidget(m_pTrDegreeEdit);
	tr_C_Layout->addWidget(pTrDegreeLabel);
	tr_C_Layout->addStretch();

	QHBoxLayout* tr_Ohm_Layout = new QHBoxLayout;

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

	m_pTrR0Edit = new QLineEdit("100", this);
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
	QGroupBox* tcGroup = new QGroupBox(tr("Thermocouple - GOST 8.585-2001"));
	QVBoxLayout* tcLayout = new QVBoxLayout;

	m_pTcList = new QComboBox(this);

	QHBoxLayout* tc_C_Layout = new QHBoxLayout;

	m_pTcDegreeRadio = new QRadioButton(this);
	m_pTcDegreeEdit = new QLineEdit("400", this);
	QLabel* pTcDegreeLabel = new QLabel(tr("°C"), this);
	pTcDegreeLabel->setFixedWidth(30);
	m_pTcDegreeEdit->setFont(*m_digitFont);

	tc_C_Layout->addWidget(m_pTcDegreeRadio);
	tc_C_Layout->addWidget(m_pTcDegreeEdit);
	tc_C_Layout->addWidget(pTcDegreeLabel);
	tc_C_Layout->addStretch();

	QHBoxLayout* tc_mV_Layout = new QHBoxLayout;

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

	// dp->F
	//
	QGroupBox* dpfGroup = new QGroupBox(tr("Pressure -> Flow"));
	QVBoxLayout* dpfLayout = new QVBoxLayout;

	QHBoxLayout* dpf_inval_Layout = new QHBoxLayout;

	m_pDpfPRadio = new QRadioButton(this);
	m_pDpfPValEdit = new QLineEdit("2.5", this);
	QLabel* pDpfInValLabel = new QLabel(tr("P"), this);
	pDpfInValLabel->setFixedWidth(30);
	m_pDpfPValEdit->setFont(*m_digitFont);

	dpf_inval_Layout->addWidget(m_pDpfPRadio);
	dpf_inval_Layout->addWidget(m_pDpfPValEdit);
	dpf_inval_Layout->addWidget(pDpfInValLabel);
	dpf_inval_Layout->addStretch();

	QHBoxLayout* dpf_outval_Layout = new QHBoxLayout;

	m_pDpfFRadio = new QRadioButton(this);
	m_pDpfFValEdit = new QLineEdit("0", this);
	QLabel* pDpfOutValLabel = new QLabel(tr("F"), this);
	pDpfOutValLabel->setFixedWidth(30);
	m_pDpfFValEdit->setFont(*m_digitFont);

	dpf_outval_Layout->addWidget(m_pDpfFRadio);
	dpf_outval_Layout->addWidget(m_pDpfFValEdit);
	dpf_outval_Layout->addWidget(pDpfOutValLabel);
	dpf_outval_Layout->addStretch();

	QHBoxLayout* dpf_inrange_Layout = new QHBoxLayout;

	m_pDpfPLowEdit = new QLineEdit("0", this);
	m_pDpfPHighEdit = new QLineEdit("10", this);

	dpf_inrange_Layout->addWidget(new QLabel(tr("min P"), this));
	dpf_inrange_Layout->addWidget(m_pDpfPLowEdit);
	dpf_inrange_Layout->addWidget(new QLabel(tr("max P"), this));
	dpf_inrange_Layout->addWidget(m_pDpfPHighEdit);

	QHBoxLayout* dpf_outrange_Layout = new QHBoxLayout;

	m_pDpfFLowEdit = new QLineEdit("0", this);
	m_pDpfFHighEdit = new QLineEdit("100", this);

	dpf_outrange_Layout->addWidget(new QLabel(tr("min F"), this));
	dpf_outrange_Layout->addWidget(m_pDpfFLowEdit);
	dpf_outrange_Layout->addWidget(new QLabel(tr("max F"), this));
	dpf_outrange_Layout->addWidget(m_pDpfFHighEdit);

	dpfLayout->addLayout(dpf_inval_Layout);
	dpfLayout->addLayout(dpf_outval_Layout);
	dpfLayout->addLayout(dpf_inrange_Layout);
	dpfLayout->addLayout(dpf_outrange_Layout);

	dpfGroup->setLayout(dpfLayout);

	// Degree
	//
	QGroupBox* drGroup = new QGroupBox(tr("Degree"));
	QVBoxLayout* drLayout = new QVBoxLayout;

	QHBoxLayout* dr_C_Layout = new QHBoxLayout;

	m_pDrCelsiusRadio = new QRadioButton(this);
	m_pDrCelsiusEdit = new QLineEdit("100", this);
    QLabel* pDrCelsiusLabel = new QLabel(tr("°C"), this);
    pDrCelsiusLabel->setFixedWidth(30);
	m_pDrCelsiusEdit->setFont(*m_digitFont);

	dr_C_Layout->addWidget(m_pDrCelsiusRadio);
	dr_C_Layout->addWidget(m_pDrCelsiusEdit);
    dr_C_Layout->addWidget(pDrCelsiusLabel);
	dr_C_Layout->addStretch();

	QHBoxLayout* dr_F_Layout = new QHBoxLayout;

	m_pDrFahrenheitRadio = new QRadioButton(this);
	m_pDrFahrenheitEdit = new QLineEdit(this);
	QLabel* pDrFahrenheitLabel = new QLabel(tr("°F"), this);
	pDrFahrenheitLabel->setFixedWidth(30);
	m_pDrFahrenheitEdit->setFont(*m_digitFont);

	dr_F_Layout->addWidget(m_pDrFahrenheitRadio);
	dr_F_Layout->addWidget(m_pDrFahrenheitEdit);
	dr_F_Layout->addWidget(pDrFahrenheitLabel);
	dr_F_Layout->addStretch();

	drLayout->addLayout(dr_C_Layout);
	drLayout->addLayout(dr_F_Layout);

	drGroup->setLayout(drLayout);

	// Main
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(linGroup);
	mainLayout->addWidget(trGroup);
	mainLayout->addWidget(tcGroup);
	mainLayout->addWidget(dpfGroup);
	mainLayout->addWidget(drGroup);

	mainLayout->addStretch();

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::initDialog()
{
	QRegularExpression rx("^[-]{0,1}[0-9]*[.]{1}[0-9]*$");
	QValidator* validator = new QRegularExpressionValidator(rx, this);

	// init elements of interface
	//
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setFixedWidth(230);
	setWindowIcon(QIcon(":/icons/Calculator.png"));
	setWindowTitle(tr("Metrological calculator"));

	QMetaEnum mst = QMetaEnum::fromType<E::SensorType>();

	// Linearity
	//
	m_pLinInRadio->setChecked(true);

	m_pLinInValEdit->setValidator(validator);
	m_pLinOutValEdit->setValidator(validator);
	m_pLinInLowEdit->setValidator(validator);
	m_pLinInHighEdit->setValidator(validator);
	m_pLinOutLowEdit->setValidator(validator);
	m_pLinOutHighEdit->setValidator(validator);

	connect(m_pLinInRadio, &QRadioButton::clicked, this, &DialogCalculator::onLinRadio);
	connect(m_pLinInValEdit, &QLineEdit::textChanged, this, &DialogCalculator::onLinValue);
	connect(m_pLinOutRadio, &QRadioButton::clicked, this, &DialogCalculator::onLinRadio);
	connect(m_pLinOutValEdit, &QLineEdit::textChanged, this, &DialogCalculator::onLinValue);
	connect(m_pLinInLowEdit, &QLineEdit::textChanged, this, &DialogCalculator::onLinValue);
	connect(m_pLinInHighEdit, &QLineEdit::textChanged, this, &DialogCalculator::onLinValue);
	connect(m_pLinOutLowEdit, &QLineEdit::textChanged, this, &DialogCalculator::onLinValue);
	connect(m_pLinOutHighEdit, &QLineEdit::textChanged, this, &DialogCalculator::onLinValue);

	conversionLin();

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

	connect(m_pTrList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogCalculator::onTrSensorTypeChanged);
	connect(m_pTrDegreeRadio, &QRadioButton::clicked, this, &DialogCalculator::onTrRadio);
	connect(m_pTrDegreeEdit, &QLineEdit::textChanged, this, &DialogCalculator::onTrValue);
	connect(m_pTrElectricRadio, &QRadioButton::clicked, this, &DialogCalculator::onTrRadio);
	connect(m_pTrElectricEdit, &QLineEdit::textChanged, this, &DialogCalculator::onTrValue);
	connect(m_pTrR0Edit, &QLineEdit::textChanged, this, &DialogCalculator::onTrValue);

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


	connect(m_pTcList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogCalculator::onTcSensorTypeChanged);
	connect(m_pTcDegreeRadio, &QRadioButton::clicked, this, &DialogCalculator::onTcRadio);
	connect(m_pTcDegreeEdit, &QLineEdit::textChanged, this, &DialogCalculator::onTcValue);
	connect(m_pTcElectricRadio, &QRadioButton::clicked, this, &DialogCalculator::onTcRadio);
	connect(m_pTcElectricEdit, &QLineEdit::textChanged, this, &DialogCalculator::onTcValue);

	m_pTcDegreeEdit->setValidator(validator);
	m_pTcElectricEdit->setValidator(validator);

	conversionTc();

	// dp->F
	//
	m_pDpfPRadio->setChecked(true);

	m_pDpfPValEdit->setValidator(validator);
	m_pDpfFValEdit->setValidator(validator);
	m_pDpfPLowEdit->setValidator(validator);
	m_pDpfPHighEdit->setValidator(validator);
	m_pDpfFLowEdit->setValidator(validator);
	m_pDpfFHighEdit->setValidator(validator);

	connect(m_pDpfPRadio, &QRadioButton::clicked, this, &DialogCalculator::onDpfRadio);
	connect(m_pDpfPValEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDpfValue);
	connect(m_pDpfFRadio, &QRadioButton::clicked, this, &DialogCalculator::onDpfRadio);
	connect(m_pDpfFValEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDpfValue);
	connect(m_pDpfPLowEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDpfValue);
	connect(m_pDpfPHighEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDpfValue);
	connect(m_pDpfFLowEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDpfValue);
	connect(m_pDpfFHighEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDpfValue);

	conversionDpf();

	// Degrees
	//
	m_pDrCelsiusRadio->setChecked(true);

	connect(m_pDrCelsiusRadio, &QRadioButton::clicked, this, &DialogCalculator::onDrRadio);
	connect(m_pDrCelsiusEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDrValue);
	connect(m_pDrFahrenheitRadio, &QRadioButton::clicked, this, &DialogCalculator::onDrRadio);
	connect(m_pDrFahrenheitEdit, &QLineEdit::textChanged, this, &DialogCalculator::onDrValue);

	m_pDrCelsiusEdit->setValidator(validator);
	m_pDrFahrenheitEdit->setValidator(validator);

	conversionDr();

	// Select first dialog item
	//
	m_pTrList->setFocus();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::conversionLin()
{
	double irl = m_pLinInLowEdit->text().toDouble();
	double irh = m_pLinInHighEdit->text().toDouble();
	double orl = m_pLinOutLowEdit->text().toDouble();
	double orh = m_pLinOutHighEdit->text().toDouble();

	if (m_pLinInRadio->isChecked() == true)
	{
		double val = m_pLinInValEdit->text().toDouble();
		double retVal = m_uc.conversionLinearity(val, UnitsConvertType::PhysicalToElectric, irl, irh, orl, orh);

		m_pLinInValEdit->setReadOnly(false);
		m_pLinOutValEdit->setText(QString::number(retVal, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		m_pLinOutValEdit->setReadOnly(true);
	}

	if (m_pLinOutRadio->isChecked() == true)
	{
		double val = m_pLinOutValEdit->text().toDouble();
		double retVal = m_uc.conversionLinearity(val, UnitsConvertType::ElectricToPhysical, irl, irh, orl, orh);

		m_pLinOutValEdit->setReadOnly(false);
		m_pLinInValEdit->setText(QString::number(retVal, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		m_pLinInValEdit->setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::conversionTr()
{
	int index = m_pTrList->currentIndex();
	if (index == -1)
	{
		return;
	}

	E::ElectricUnit unit = E::ElectricUnit::Ohm;
	E::SensorType sensorType = static_cast<E::SensorType>(m_pTrList->itemData(index).toInt());

	SignalElectricLimit electricLimit = m_uc.getElectricLimit(unit, sensorType);
	if(electricLimit.isValid() == false)
	{
		return;
	}

	double degreeVal = m_pTrDegreeEdit->text().toDouble();
	double electricVal = m_pTrElectricEdit->text().toDouble();

	double r0 = m_pTrR0Edit->text().toDouble();

	if (UnitsConverter::r0_OhmIsValid(r0) == false)
	{
		r0 = 100.0;
	}

	if (m_pTrDegreeRadio->isChecked() == true)
	{
		double degreeLowLimit = m_uc.conversionDegree(electricLimit.lowLimit*  r0 / 100, UnitsConvertType::ElectricToPhysical, unit, sensorType, r0);
		double degreeHighLimit = m_uc.conversionDegree(electricLimit.highLimit*  r0 / 100, UnitsConvertType::ElectricToPhysical, unit, sensorType, r0);

		if (degreeVal < degreeLowLimit || degreeVal > degreeHighLimit)
		{
			m_pTrElectricEdit->setText(	tr("Out of range: %1 .. %2").
										arg(QString::number(degreeLowLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION),
											QString::number(degreeHighLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION)));

			m_pTrElectricEdit->setCursorPosition(0);
		}
		else
		{
			double val = m_uc.conversionDegree(degreeVal, UnitsConvertType::PhysicalToElectric, unit, sensorType, r0);

			m_pTrElectricEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		}

		m_pTrDegreeEdit->setReadOnly(false);
		m_pTrElectricEdit->setReadOnly(true);
	}

	if (m_pTrElectricRadio->isChecked() == true)
	{
		if (electricVal < electricLimit.lowLimit*  r0 / 100 || electricVal > electricLimit.highLimit*  r0 / 100)
		{
			m_pTrDegreeEdit->setText(	tr("Out of range: %1 .. %2").
										arg(QString::number(electricLimit.lowLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION),
											QString::number(electricLimit.highLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION)));

			m_pTrDegreeEdit->setCursorPosition(0);
		}
		else
		{
			double val = m_uc.conversionDegree(electricVal, UnitsConvertType::ElectricToPhysical, unit, sensorType, r0);

			m_pTrDegreeEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		}

		m_pTrElectricEdit->setReadOnly(false);
		m_pTrDegreeEdit->setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::conversionTc()
{
	int index = m_pTcList->currentIndex();
	if (index == -1)
	{
		return;
	}

	E::ElectricUnit unit = E::ElectricUnit::mV;
	E::SensorType sensorType = static_cast<E::SensorType>(m_pTcList->itemData(index).toInt());

	SignalElectricLimit electricLimit = m_uc.getElectricLimit(unit, sensorType);
	if(electricLimit.isValid() == false)
	{
		return;
	}

	double degreeVal = m_pTcDegreeEdit->text().toDouble();
	double electricVal = m_pTcElectricEdit->text().toDouble();

	if (m_pTcDegreeRadio->isChecked() == true)
	{
		double degreeLowLimit = m_uc.conversionDegree(electricLimit.lowLimit, UnitsConvertType::ElectricToPhysical, unit, sensorType);
		double degreeHighLimit = m_uc.conversionDegree(electricLimit.highLimit, UnitsConvertType::ElectricToPhysical, unit, sensorType);

		if (degreeVal < degreeLowLimit || degreeVal > degreeHighLimit)
		{
			m_pTcElectricEdit->setText(	tr("Out of range: %1 .. %2").
										arg(QString::number(degreeLowLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION),
											QString::number(degreeHighLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION)));

			m_pTcElectricEdit->setCursorPosition(0);
		}
		else
		{
			double val = m_uc.conversionDegree(degreeVal, UnitsConvertType::PhysicalToElectric, unit, sensorType);

			m_pTcElectricEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		}

		m_pTcDegreeEdit->setFocus();
		m_pTcDegreeEdit->setReadOnly(false);
		m_pTcElectricEdit->setReadOnly(true);
	}

	if (m_pTcElectricRadio->isChecked() == true)
	{
		if (electricVal < electricLimit.lowLimit || electricVal > electricLimit.highLimit)
		{
			m_pTcDegreeEdit->setText(	tr("Out of range: %1 .. %2").
										arg(QString::number(electricLimit.lowLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION),
											QString::number(electricLimit.highLimit, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION)));

			m_pTcDegreeEdit->setCursorPosition(0);
		}
		else
		{
			double val = m_uc.conversionDegree(electricVal, UnitsConvertType::ElectricToPhysical, unit, sensorType);

			m_pTcDegreeEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		}

		m_pTcElectricEdit->setFocus();
		m_pTcElectricEdit->setReadOnly(false);
		m_pTcDegreeEdit->setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::conversionDpf()
{
	double prl = m_pDpfPLowEdit->text().toDouble();
	double prh = m_pDpfPHighEdit->text().toDouble();
	double frl = m_pDpfFLowEdit->text().toDouble();
	double frh = m_pDpfFHighEdit->text().toDouble();

	double K = (frh - frl) / sqrt(prh - prl);

	if (m_pDpfPRadio->isChecked() == true)
	{
		double val = K*  sqrt(m_pDpfPValEdit->text().toDouble());

		m_pDpfPValEdit->setReadOnly(false);
		m_pDpfFValEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		m_pDpfFValEdit->setReadOnly(true);
	}

	if (m_pDpfFRadio->isChecked() == true)
	{
		double val = pow(m_pDpfFValEdit->text().toDouble() / K, 2);

		m_pDpfFValEdit->setReadOnly(false);
		m_pDpfPValEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		m_pDpfPValEdit->setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogCalculator::conversionDr()
{
	if (m_pDrCelsiusRadio->isChecked() == true)
	{
		double val = m_uc.conversionDegree(m_pDrCelsiusEdit->text().toDouble(), UnitsConvertType::CelsiusToFahrenheit);

		m_pDrCelsiusEdit->setFocus();
		m_pDrCelsiusEdit->setReadOnly(false);
		m_pDrFahrenheitEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		m_pDrFahrenheitEdit->setReadOnly(true);
	}

	if (m_pDrFahrenheitRadio->isChecked() == true)
	{
		double val = m_uc.conversionDegree(m_pDrFahrenheitEdit->text().toDouble(), UnitsConvertType::FahrenheitToCelsius);

		m_pDrFahrenheitEdit->setFocus();
		m_pDrFahrenheitEdit->setReadOnly(false);
		m_pDrCelsiusEdit->setText(QString::number(val, 'f', DEFAULT_ELECTRIC_UNIT_PRECESION));
		m_pDrCelsiusEdit->setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------
