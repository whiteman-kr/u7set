#include "SignalPropertiesDialog.h"
#include <QtStringPropertyManager>
#include <QtIntPropertyManager>
#include <QtEnumPropertyManager>
#include <QtDoublePropertyManager>
#include <QtBoolPropertyManager>
#include <QtProperty>
#include <QtTreePropertyBrowser>
#include <QtLineEditFactory>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "../include/Signal.h"


SignalPropertiesDialog::SignalPropertiesDialog(Signal& signal, SignalType signalType, DataFormatList &dataFormatInfo, UnitList &unitInfo, QWidget *parent) :
	QDialog(parent),
	m_signal(signal),
	m_dataFormatInfo(dataFormatInfo),
	m_unitInfo(unitInfo)
{
	QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
	m_stringManager = new QtStringPropertyManager(this);
	m_enumManager = new QtEnumPropertyManager(this);
	m_intManager = new QtIntPropertyManager(this);
	m_doubleManager = new QtDoublePropertyManager(this);
	m_boolManager = new QtBoolPropertyManager(this);

	QtProperty *signalProperty = groupManager->addProperty(tr("Signal"));

    m_strIDProperty = m_stringManager->addProperty(tr("ID"));
	QString strID = signal.strID();
	if (strID[0] != '#')
	{
		strID = '#' + strID;
	}
	m_stringManager->setValue(m_strIDProperty, strID);
	QRegExp rx4ID("^#[A-Z][A-Z\\d_]*$");
	m_stringManager->setRegExp(m_strIDProperty, rx4ID);
    signalProperty->addSubProperty(m_strIDProperty);

	m_extStrIDProperty = m_stringManager->addProperty(tr("External ID"));
	m_stringManager->setValue(m_extStrIDProperty, signal.extStrID());
	QRegExp rx4ExtID("^[A-Z][A-Z\\d_]*$");
	m_stringManager->setRegExp(m_extStrIDProperty, rx4ExtID);
    signalProperty->addSubProperty(m_extStrIDProperty);

	m_nameProperty = m_stringManager->addProperty(tr("Name"));
	m_stringManager->setValue(m_nameProperty, signal.name());
	QRegExp rx4Name("^.+$");
	m_stringManager->setRegExp(m_nameProperty, rx4Name);
    signalProperty->addSubProperty(m_nameProperty);


	assert(false);			// refactor
	/*QStringList dataFormatNames;
	int selected = -1;
	for (int i = 0; i < dataFormatInfo.count(); i++)
	{
		dataFormatNames << dataFormatInfo[i].name;
		if (dataFormatInfo[i].ID == signal.dataFormat())
		{
			selected = i;
		}
	}
	m_dataFormatProperty = m_enumManager->addProperty(tr("Data format"));
	m_enumManager->setEnumNames(m_dataFormatProperty, dataFormatNames);
	m_enumManager->setValue(m_dataFormatProperty, selected);
	signalProperty->addSubProperty(m_dataFormatProperty);*/

	m_dataSizeProperty = m_intManager->addProperty(tr("Data size"));
	m_intManager->setRange(m_dataSizeProperty, 1, 100);
	if (signalType == SignalType::analog)
	{
		m_intManager->setValue(m_dataSizeProperty, signal.dataSize());
	}
	else
	{
		m_intManager->setValue(m_dataSizeProperty, 1);
		m_intManager->setReadOnly(m_dataSizeProperty, true);
	}
	signalProperty->addSubProperty(m_dataSizeProperty);

	m_lowAdcProperty = m_intManager->addProperty(tr("Low ADC"));
	m_intManager->setRange(m_lowAdcProperty, 0, 65535);
	m_intManager->setValue(m_lowAdcProperty, signal.lowADC());
	signalProperty->addSubProperty(m_lowAdcProperty);

	m_highAdcProperty = m_intManager->addProperty(tr("High ADC"));
	m_intManager->setRange(m_highAdcProperty, 0, 65535);
	m_intManager->setValue(m_highAdcProperty, signal.highADC());
	signalProperty->addSubProperty(m_highAdcProperty);

	m_lowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
	m_doubleManager->setValue(m_lowLimitProperty, signal.lowLimit());
	signalProperty->addSubProperty(m_lowLimitProperty);

	m_highLimitProperty = m_doubleManager->addProperty(tr("High limit"));
	m_doubleManager->setValue(m_highLimitProperty, signal.highLimit());
	signalProperty->addSubProperty(m_highLimitProperty);

	assert(false);	// refactor
/*	QStringList unitNames;
	selected = -1;
	for (int i = 0; i < unitInfo.count(); i++)
	{
		unitNames << unitInfo[i].nameEn;
		if (unitInfo[i].ID == signal.unitID())
		{
			selected = i;
		}
	}
	m_unitProperty = m_enumManager->addProperty(tr("Unit"));
	m_enumManager->setEnumNames(m_unitProperty, unitNames);
	m_enumManager->setValue(m_unitProperty, selected);
	signalProperty->addSubProperty(m_unitProperty);*/

	m_adjustmentProperty = m_doubleManager->addProperty(tr("Adjustment"));
	m_doubleManager->setValue(m_adjustmentProperty, signal.adjustment());
	signalProperty->addSubProperty(m_adjustmentProperty);

	m_dropLimitProperty = m_doubleManager->addProperty(tr("Drop limit"));
	m_doubleManager->setValue(m_dropLimitProperty, signal.dropLimit());
	signalProperty->addSubProperty(m_dropLimitProperty);

	m_excessLimitProperty = m_doubleManager->addProperty(tr("Excess limit"));
	m_doubleManager->setValue(m_excessLimitProperty, signal.excessLimit());
	signalProperty->addSubProperty(m_excessLimitProperty);

	m_unbalanceLimitProperty = m_doubleManager->addProperty(tr("Unbalance limit"));
	m_doubleManager->setValue(m_unbalanceLimitProperty, signal.unbalanceLimit());
	signalProperty->addSubProperty(m_unbalanceLimitProperty);

	// Input sensor
	//
	QtProperty *inputProperty = groupManager->addProperty(tr("Input sensor"));

	m_inputLowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
	m_doubleManager->setValue(m_inputLowLimitProperty, signal.inputLowLimit());
	inputProperty->addSubProperty(m_inputLowLimitProperty);

	m_inputHighLimitProperty = m_doubleManager->addProperty(tr("High limit"));
	m_doubleManager->setValue(m_inputHighLimitProperty, signal.inputHighLimit());
	inputProperty->addSubProperty(m_inputHighLimitProperty);

	assert(false);	// refactor
/*	selected = -1;
	for (int i = 0; i < unitInfo.count(); i++)
	{
		if (unitInfo[i].ID == signal.inputUnitID())
		{
			selected = i;
		}
	}
	m_inputUnitProperty = m_enumManager->addProperty(tr("Unit"));
	m_enumManager->setEnumNames(m_inputUnitProperty, unitNames);
	m_enumManager->setValue(m_inputUnitProperty, selected);
	inputProperty->addSubProperty(m_inputUnitProperty);*/

	QStringList sensorNames;
	for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
	{
		sensorNames << SensorTypeStr[i];
	}
	m_inputSensorProperty = m_enumManager->addProperty(tr("Sensor type"));
	m_enumManager->setEnumNames(m_inputSensorProperty, sensorNames);
	m_enumManager->setValue(m_inputSensorProperty, signal.inputSensorID());
	inputProperty->addSubProperty(m_inputSensorProperty);

	signalProperty->addSubProperty(inputProperty);

	// Output sensor
	//
	QtProperty *outputProperty = groupManager->addProperty(tr("Output sensor"));

	m_outputLowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
	m_doubleManager->setValue(m_outputLowLimitProperty, signal.outputLowLimit());
	outputProperty->addSubProperty(m_outputLowLimitProperty);

	m_outputHighLimitProperty = m_doubleManager->addProperty(tr("High limit"));
	m_doubleManager->setValue(m_outputHighLimitProperty, signal.outputHighLimit());
	outputProperty->addSubProperty(m_outputHighLimitProperty);

	assert(false);	// refactor
/*	selected = -1;
	for (int i = 0; i < unitInfo.count(); i++)
	{
		if (unitInfo[i].ID == signal.outputUnitID())
		{
			selected = i;
		}
	}
	m_outputUnitProperty = m_enumManager->addProperty(tr("Unit"));
	m_enumManager->setEnumNames(m_outputUnitProperty, unitNames);
	m_enumManager->setValue(m_outputUnitProperty, selected);
	outputProperty->addSubProperty(m_outputUnitProperty);*/

	m_outputSensorProperty = m_enumManager->addProperty(tr("Sensor type"));
	m_enumManager->setEnumNames(m_outputSensorProperty, sensorNames);
	m_enumManager->setValue(m_outputSensorProperty, signal.outputSensorID());
	outputProperty->addSubProperty(m_outputSensorProperty);

	signalProperty->addSubProperty(outputProperty);

	m_acquireProperty = m_boolManager->addProperty(tr("Acquire"));
	m_boolManager->setValue(m_acquireProperty, signal.acquire());
	signalProperty->addSubProperty(m_acquireProperty);

	m_calculatedProperty = m_boolManager->addProperty(tr("Calculated"));
	m_boolManager->setValue(m_calculatedProperty, signal.calculated());
	signalProperty->addSubProperty(m_calculatedProperty);

	m_normalStateProperty = m_intManager->addProperty(tr("Normal state"));
	m_intManager->setValue(m_normalStateProperty, signal.normalState());
	signalProperty->addSubProperty(m_normalStateProperty);

	m_decimalPlacesProperty = m_intManager->addProperty(tr("Decimal places"));
	m_intManager->setValue(m_decimalPlacesProperty, signal.decimalPlaces());
	signalProperty->addSubProperty(m_decimalPlacesProperty);

	m_apertureProperty = m_doubleManager->addProperty(tr("Aperture"));
	m_doubleManager->setValue(m_apertureProperty, signal.aperture());
	signalProperty->addSubProperty(m_apertureProperty);

	m_deviceIDProperty = m_stringManager->addProperty(tr("Device ID"));
	m_stringManager->setValue(m_deviceIDProperty, signal.deviceStrID());
    signalProperty->addSubProperty(m_deviceIDProperty);

	QtLineEditFactory* lineEditFactory = new QtLineEditFactory(this);
	QtEnumEditorFactory* enumEditFactory = new QtEnumEditorFactory(this);
	QtSpinBoxFactory* spinBoxFactory = new QtSpinBoxFactory(this);
	QtDoubleSpinBoxFactory* doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
	QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);

	QtAbstractPropertyBrowser *browser = new QtTreePropertyBrowser(this);
	browser->setFactoryForManager(m_stringManager, lineEditFactory);
	browser->setFactoryForManager(m_enumManager, enumEditFactory);
	browser->setFactoryForManager(m_intManager, spinBoxFactory);
	browser->setFactoryForManager(m_doubleManager, doubleSpinBoxFactory);
	browser->setFactoryForManager(m_boolManager, checkBoxFactory);

	browser->addProperty(signalProperty);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(browser);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(checkAndSaveSignal()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	vl->addWidget(buttonBox);
	setLayout(vl);

	setFixedHeight(640);
	setFixedWidth(320);
}

void SignalPropertiesDialog::checkAndSaveSignal()
{
	QString strID = m_stringManager->value(m_strIDProperty);
	QRegExp rx4ID("^#[A-Z][A-Z\\d_]*$");
	if (!rx4ID.exactMatch(strID))
	{
		QMessageBox::information(this, tr("Information"), tr("You have to set ID in correct format, "
															 "it should begin from '#' and latin letter "
															 "and be followed by any number of latin letters, '_' and digits"));
		return;
	}
	QString extStrID = m_stringManager->value(m_extStrIDProperty);
	QRegExp rx4ExtID("^[A-Z][A-Z\\d_]*$");
	if (!rx4ExtID.exactMatch(extStrID))
	{
		QMessageBox::information(this, tr("Information"), tr("You have to set External ID in correct format, "
															 "it should begin from latin letter "
															 "and be followed by any number of latin letters, '_' and digits"));
		return;
	}
	QString name = m_stringManager->value(m_nameProperty);
	if (name.isEmpty())
	{
		QMessageBox::information(this, tr("Information"), tr("You should fill signal Name, it could not be empty"));
		return;
	}
	m_signal.setStrID(strID);
	m_signal.setExtStrID(extStrID);
	m_signal.setName(name);
	int dataFormatIndex = m_enumManager->value(m_dataFormatProperty);
	if (dataFormatIndex > 0 && dataFormatIndex < m_dataFormatInfo.count())
	{
		assert(false);	// refactor
		//m_signal.setDataFormat(m_dataFormatInfo[dataFormatIndex].ID);
	}
	m_signal.setDataSize(m_intManager->value(m_dataSizeProperty));
	m_signal.setLowADC(m_intManager->value(m_lowAdcProperty));
	m_signal.setHighADC(m_intManager->value(m_highAdcProperty));
	m_signal.setLowLimit(m_doubleManager->value(m_lowLimitProperty));
	m_signal.setHighLimit(m_doubleManager->value(m_highLimitProperty));
	int unitIndex = m_enumManager->value(m_unitProperty);
	if (unitIndex > 0 && unitIndex < m_unitInfo.count())
	{
		assert(false);	// refactor
		//m_signal.setUnitID(m_unitInfo[unitIndex].ID);
	}
	else
	{
		m_signal.setUnitID(NO_UNIT_ID);
	}
	m_signal.setAdjustment(m_doubleManager->value(m_adjustmentProperty));
	m_signal.setExcessLimit(m_doubleManager->value(m_excessLimitProperty));
	m_signal.setUnbalanceLimit(m_doubleManager->value(m_unbalanceLimitProperty));

	// Input sensor
	//
	m_signal.setInputLowLimit(m_doubleManager->value(m_inputLowLimitProperty));
	m_signal.setInputHighLimit(m_doubleManager->value(m_inputHighLimitProperty));
	unitIndex = m_enumManager->value(m_inputUnitProperty);
	if (unitIndex > 0 && unitIndex < m_unitInfo.count())
	{
		assert(false);	// refactor
		//m_signal.setInputUnitID(m_unitInfo[unitIndex].ID);
	}
	else
	{
		m_signal.setInputUnitID(NO_UNIT_ID);
	}
	int sensorIndex = m_enumManager->value(m_inputSensorProperty);
	if (sensorIndex > 0 && sensorIndex < SENSOR_TYPE_COUNT)
	{
		m_signal.setInputSensorID(sensorIndex);
	}

	// Output sensor
	//
	m_signal.setOutputLowLimit(m_doubleManager->value(m_outputLowLimitProperty));
	m_signal.setOutputHighLimit(m_doubleManager->value(m_outputHighLimitProperty));
	unitIndex = m_enumManager->value(m_outputUnitProperty);
	if (unitIndex > 0 && unitIndex < m_unitInfo.count())
	{
		assert(false);	// refactor
		//m_signal.setOutputUnitID(m_unitInfo[unitIndex].ID);
	}
	else
	{
		m_signal.setOutputUnitID(NO_UNIT_ID);
	}
	sensorIndex = m_enumManager->value(m_outputSensorProperty);
	if (sensorIndex > 0 && sensorIndex < SENSOR_TYPE_COUNT)
	{
		m_signal.setOutputSensorID(sensorIndex);
	}

	m_signal.setAcquire(m_boolManager->value(m_acquireProperty));
	m_signal.setCalculated(m_boolManager->value(m_calculatedProperty));
	m_signal.setNormalState(m_intManager->value(m_normalStateProperty));
	m_signal.setDecimalPlaces(m_intManager->value(m_decimalPlacesProperty));
	m_signal.setAperture(m_doubleManager->value(m_apertureProperty));
	m_signal.setDeviceStrID(m_stringManager->value(m_deviceIDProperty));

	accept();
}
