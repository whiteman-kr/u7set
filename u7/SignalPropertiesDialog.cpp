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
#include <QSettings>
#include <QtVariantProperty>
#include "../include/Signal.h"
#include "SignalsTabPage.h"


SignalPropertiesDialog::SignalPropertiesDialog(Signal& signal, SignalType signalType, DataFormatList &dataFormatInfo, UnitList &unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent) :
	QDialog(parent),
	m_signal(signal),
	m_dataFormatInfo(dataFormatInfo),
	m_unitInfo(unitInfo),
	m_signalsModel(signalsModel)
{
	QSettings settings;
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
	QRegExp rx4ID("^#[A-Za-z][A-Za-z\\d_]*$");
	m_stringManager->setRegExp(m_strIDProperty, rx4ID);
	m_stringManager->setReadOnly(m_strIDProperty, readOnly);
    signalProperty->addSubProperty(m_strIDProperty);

	m_extStrIDProperty = m_stringManager->addProperty(tr("External ID"));
	m_stringManager->setValue(m_extStrIDProperty, signal.extStrID());
	QRegExp rx4ExtID("^[A-Za-z][A-Za-z\\d_]*$");
	m_stringManager->setRegExp(m_extStrIDProperty, rx4ExtID);
	m_stringManager->setReadOnly(m_extStrIDProperty, readOnly);
    signalProperty->addSubProperty(m_extStrIDProperty);

	m_nameProperty = m_stringManager->addProperty(tr("Name"));
	m_stringManager->setValue(m_nameProperty, signal.name());
	QRegExp rx4Name("^.+$");
	m_stringManager->setRegExp(m_nameProperty, rx4Name);
	m_stringManager->setReadOnly(m_nameProperty, readOnly);
    signalProperty->addSubProperty(m_nameProperty);

	m_dataFormatProperty = m_enumManager->addProperty(tr("Data format"));
	m_enumManager->setEnumNames(m_dataFormatProperty, dataFormatInfo.toList());
	m_enumManager->setValue(m_dataFormatProperty, dataFormatInfo.keyIndex(signal.dataFormatInt()));
	signalProperty->addSubProperty(m_dataFormatProperty);

	m_dataSizeProperty = m_intManager->addProperty(tr("Data size"));
	m_intManager->setRange(m_dataSizeProperty, 1, 100);
	if (signalType == SignalType::Analog)
	{
		m_intManager->setValue(m_dataSizeProperty, signal.dataSize());
		m_intManager->setReadOnly(m_dataSizeProperty, readOnly);
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
	m_intManager->setReadOnly(m_lowAdcProperty, readOnly);
	signalProperty->addSubProperty(m_lowAdcProperty);

	m_highAdcProperty = m_intManager->addProperty(tr("High ADC"));
	m_intManager->setRange(m_highAdcProperty, 0, 65535);
	m_intManager->setValue(m_highAdcProperty, signal.highADC());
	m_intManager->setReadOnly(m_highAdcProperty, readOnly);
	signalProperty->addSubProperty(m_highAdcProperty);

	m_lowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
	m_doubleManager->setValue(m_lowLimitProperty, signal.lowLimit());
	m_doubleManager->setReadOnly(m_lowLimitProperty, readOnly);
	signalProperty->addSubProperty(m_lowLimitProperty);

	m_highLimitProperty = m_doubleManager->addProperty(tr("High limit"));
	m_doubleManager->setValue(m_highLimitProperty, signal.highLimit());
	m_doubleManager->setReadOnly(m_highLimitProperty, readOnly);
	signalProperty->addSubProperty(m_highLimitProperty);

	QStringList unitStringList = unitInfo.toList();
	m_unitProperty = m_enumManager->addProperty(tr("Unit"));
	m_enumManager->setEnumNames(m_unitProperty, unitStringList);
	m_enumManager->setValue(m_unitProperty, unitInfo.keyIndex(signal.unitID()));
	signalProperty->addSubProperty(m_unitProperty);

	m_adjustmentProperty = m_doubleManager->addProperty(tr("Adjustment"));
	m_doubleManager->setValue(m_adjustmentProperty, signal.adjustment());
	m_doubleManager->setReadOnly(m_adjustmentProperty, readOnly);
	signalProperty->addSubProperty(m_adjustmentProperty);

	m_dropLimitProperty = m_doubleManager->addProperty(tr("Drop limit"));
	m_doubleManager->setValue(m_dropLimitProperty, signal.dropLimit());
	m_doubleManager->setReadOnly(m_dropLimitProperty, readOnly);
	signalProperty->addSubProperty(m_dropLimitProperty);

	m_excessLimitProperty = m_doubleManager->addProperty(tr("Excess limit"));
	m_doubleManager->setValue(m_excessLimitProperty, signal.excessLimit());
	m_doubleManager->setReadOnly(m_excessLimitProperty, readOnly);
	signalProperty->addSubProperty(m_excessLimitProperty);

	m_unbalanceLimitProperty = m_doubleManager->addProperty(tr("Unbalance limit"));
	m_doubleManager->setValue(m_unbalanceLimitProperty, signal.unbalanceLimit());
	m_doubleManager->setReadOnly(m_unbalanceLimitProperty, readOnly);
	signalProperty->addSubProperty(m_unbalanceLimitProperty);

	// Input sensor
	//
	m_inputTreeProperty = groupManager->addProperty(tr("Input sensor"));

	m_inputLowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
	m_doubleManager->setValue(m_inputLowLimitProperty, signal.inputLowLimit());
	m_doubleManager->setReadOnly(m_inputLowLimitProperty, readOnly);
	m_inputTreeProperty->addSubProperty(m_inputLowLimitProperty);

	m_inputHighLimitProperty = m_doubleManager->addProperty(tr("High limit"));
	m_doubleManager->setValue(m_inputHighLimitProperty, signal.inputHighLimit());
	m_doubleManager->setReadOnly(m_inputHighLimitProperty, readOnly);
	m_inputTreeProperty->addSubProperty(m_inputHighLimitProperty);

	m_inputUnitProperty = m_enumManager->addProperty(tr("Unit"));
	m_enumManager->setEnumNames(m_inputUnitProperty, unitStringList);
	m_enumManager->setValue(m_inputUnitProperty, unitInfo.keyIndex(signal.inputUnitID()));
	m_inputTreeProperty->addSubProperty(m_inputUnitProperty);

	QStringList sensorNames;
	for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
	{
		sensorNames << SensorTypeStr[i];
	}
	m_inputSensorProperty = m_enumManager->addProperty(tr("Sensor type"));
	m_enumManager->setEnumNames(m_inputSensorProperty, sensorNames);
	m_enumManager->setValue(m_inputSensorProperty, signal.inputSensorID());
	m_inputTreeProperty->addSubProperty(m_inputSensorProperty);

	signalProperty->addSubProperty(m_inputTreeProperty);

	// Output sensor
	//
	m_outputTreeProperty = groupManager->addProperty(tr("Output sensor"));

	m_outputLowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
	m_doubleManager->setValue(m_outputLowLimitProperty, signal.outputLowLimit());
	m_doubleManager->setReadOnly(m_outputLowLimitProperty, readOnly);
	m_outputTreeProperty->addSubProperty(m_outputLowLimitProperty);

	m_outputHighLimitProperty = m_doubleManager->addProperty(tr("High limit"));
	m_doubleManager->setValue(m_outputHighLimitProperty, signal.outputHighLimit());
	m_doubleManager->setReadOnly(m_outputHighLimitProperty, readOnly);
	m_outputTreeProperty->addSubProperty(m_outputHighLimitProperty);

	m_outputUnitProperty = m_enumManager->addProperty(tr("Unit"));
	m_enumManager->setEnumNames(m_outputUnitProperty, unitStringList);
	m_enumManager->setValue(m_outputUnitProperty, unitInfo.keyIndex(signal.outputUnitID()));
	m_outputTreeProperty->addSubProperty(m_outputUnitProperty);

	QStringList outputRangeModeNames;
	for (int i = 0; i < OUTPUT_RANGE_MODE_COUNT; i++)
	{
		outputRangeModeNames << OutputRangeModeStr[i];
	}
	m_outputRangeModeProperty = m_enumManager->addProperty(tr("Output range mode"));
	m_enumManager->setEnumNames(m_outputRangeModeProperty, outputRangeModeNames);
	m_enumManager->setValue(m_outputRangeModeProperty, signal.outputRangeMode());
	m_outputTreeProperty->addSubProperty(m_outputRangeModeProperty);

	m_outputSensorProperty = m_enumManager->addProperty(tr("Sensor type"));
	m_enumManager->setEnumNames(m_outputSensorProperty, sensorNames);
	m_enumManager->setValue(m_outputSensorProperty, signal.outputSensorID());
	m_outputTreeProperty->addSubProperty(m_outputSensorProperty);

	signalProperty->addSubProperty(m_outputTreeProperty);

	m_acquireProperty = m_boolManager->addProperty(tr("Acquire"));
	m_boolManager->setValue(m_acquireProperty, signal.acquire());
	signalProperty->addSubProperty(m_acquireProperty);

	m_calculatedProperty = m_boolManager->addProperty(tr("Calculated"));
	m_boolManager->setValue(m_calculatedProperty, signal.calculated());
	signalProperty->addSubProperty(m_calculatedProperty);

	m_normalStateProperty = m_intManager->addProperty(tr("Normal state"));
	m_intManager->setValue(m_normalStateProperty, signal.normalState());
	m_intManager->setReadOnly(m_normalStateProperty, readOnly);
	signalProperty->addSubProperty(m_normalStateProperty);

	m_decimalPlacesProperty = m_intManager->addProperty(tr("Decimal places"));
	m_intManager->setValue(m_decimalPlacesProperty, signal.decimalPlaces());
	m_intManager->setReadOnly(m_decimalPlacesProperty, readOnly);
	signalProperty->addSubProperty(m_decimalPlacesProperty);

	m_apertureProperty = m_doubleManager->addProperty(tr("Aperture"));
	m_doubleManager->setValue(m_apertureProperty, signal.aperture());
	m_doubleManager->setReadOnly(m_apertureProperty, readOnly);
	signalProperty->addSubProperty(m_apertureProperty);

	m_filteringTimeProperty = m_doubleManager->addProperty(tr("Filtering time"));
	m_doubleManager->setValue(m_filteringTimeProperty, signal.filteringTime());
	m_doubleManager->setReadOnly(m_filteringTimeProperty, readOnly);
	signalProperty->addSubProperty(m_filteringTimeProperty);

	m_maxDifferenceProperty = m_doubleManager->addProperty(tr("Max difference"));
	m_doubleManager->setValue(m_maxDifferenceProperty, signal.maxDifference());
	m_doubleManager->setReadOnly(m_maxDifferenceProperty, readOnly);
	signalProperty->addSubProperty(m_maxDifferenceProperty);

	QStringList inOutStringList;
	for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
	{
		inOutStringList << InOutTypeStr[i];
	}
	m_inOutTypeProperty = m_enumManager->addProperty(tr("Input-output type"));
	m_enumManager->setEnumNames(m_inOutTypeProperty, inOutStringList);
	m_enumManager->setValue(m_inOutTypeProperty, signal.inOutType());
	signalProperty->addSubProperty(m_inOutTypeProperty);

	QStringList byteOrderStringList;

	for (int i = 0; i < TO_INT(ByteOrder::Count); i++)
	{
		byteOrderStringList << ByteOrderStr[i];
	}

	m_byteOrderProperty = m_enumManager->addProperty(tr("Byte order"));
	m_enumManager->setEnumNames(m_byteOrderProperty, byteOrderStringList);
	m_enumManager->setValue(m_byteOrderProperty, signal.byteOrderInt());
	signalProperty->addSubProperty(m_byteOrderProperty);

	m_deviceIDProperty = m_stringManager->addProperty(tr("Device ID"));
	m_stringManager->setValue(m_deviceIDProperty, signal.deviceStrID());
	m_stringManager->setReadOnly(m_deviceIDProperty, readOnly);
    signalProperty->addSubProperty(m_deviceIDProperty);

	QtLineEditFactory* lineEditFactory = new QtLineEditFactory(this);
	QtEnumEditorFactory* enumEditFactory = new QtEnumEditorFactory(this);
	QtSpinBoxFactory* spinBoxFactory = new QtSpinBoxFactory(this);
	QtDoubleSpinBoxFactory* doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
	QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);

	m_browser = new QtTreePropertyBrowser(this);
	m_browser->setFactoryForManager(m_stringManager, lineEditFactory);
	m_browser->setFactoryForManager(m_enumManager, enumEditFactory);
	m_browser->setFactoryForManager(m_intManager, spinBoxFactory);
	m_browser->setFactoryForManager(m_doubleManager, doubleSpinBoxFactory);
	m_browser->setFactoryForManager(m_boolManager, checkBoxFactory);

	m_browser->addProperty(signalProperty);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(m_browser);

	QDialogButtonBox* buttonBox;

	if (!readOnly)
	{
		setWindowTitle("Signal properties editing");
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertiesDialog::checkAndSaveSignal);
	}
	else
	{
		setWindowTitle("Signal properties (read only)");
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertiesDialog::reject);
	}
	connect(buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertiesDialog::reject);
	connect(this, &SignalPropertiesDialog::finished, this, &SignalPropertiesDialog::saveDialogSettings);
	if (m_signalsModel != nullptr)
	{
		connect(this, &SignalPropertiesDialog::onError, m_signalsModel, static_cast<void (SignalsModel::*)(QString)>(&SignalsModel::showError));
	}
	connect(m_stringManager, &QtStringPropertyManager::propertyChanged, this, &SignalPropertiesDialog::checkoutSignal);
	connect(m_enumManager, &QtEnumPropertyManager::propertyChanged, this, &SignalPropertiesDialog::checkoutSignal);
	connect(m_intManager, &QtIntPropertyManager::propertyChanged, this, &SignalPropertiesDialog::checkoutSignal);
	connect(m_doubleManager, &QtDoublePropertyManager::propertyChanged, this, &SignalPropertiesDialog::checkoutSignal);
	connect(m_boolManager, &QtBoolPropertyManager::propertyChanged, this, &SignalPropertiesDialog::checkoutSignal);

	vl->addWidget(buttonBox);
	setLayout(vl);

	resize(settings.value("Signal properties dialog: size", QSize(320, 640)).toSize());
	m_browser->setExpanded(m_browser->items(m_inputTreeProperty)[0], settings.value("Signal properties dialog: input property: expanded", false).toBool());
	m_browser->setExpanded(m_browser->items(m_outputTreeProperty)[0], settings.value("Signal properties dialog: output property: expanded", false).toBool());
}


void SignalPropertiesDialog::checkAndSaveSignal()
{
	QString strID = m_stringManager->value(m_strIDProperty);
	QRegExp rx4ID("^#[A-Za-z][A-Za-z\\d_]*$");
	if (!rx4ID.exactMatch(strID))
	{
		QMessageBox::information(this, tr("Information"), tr("You have to set ID in correct format, "
															 "it should begin from '#' and latin letter "
															 "and be followed by any number of latin letters, '_' and digits"));
		return;
	}
	QString extStrID = m_stringManager->value(m_extStrIDProperty);
	QRegExp rx4ExtID("^[A-Za-z][A-Za-z\\d_]*$");
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
	if (dataFormatIndex >= 0 && dataFormatIndex < m_dataFormatInfo.count())
	{
		m_signal.setDataFormat(static_cast<DataFormat>(m_dataFormatInfo.key(dataFormatIndex)));
	}
	m_signal.setDataSize(m_intManager->value(m_dataSizeProperty));
	m_signal.setLowADC(m_intManager->value(m_lowAdcProperty));
	m_signal.setHighADC(m_intManager->value(m_highAdcProperty));
	m_signal.setLowLimit(m_doubleManager->value(m_lowLimitProperty));
	m_signal.setHighLimit(m_doubleManager->value(m_highLimitProperty));
	int unitIndex = m_enumManager->value(m_unitProperty);
	if (unitIndex >= 0 && unitIndex < m_unitInfo.count())
	{
		m_signal.setUnitID(m_unitInfo.key(unitIndex));
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
	if (unitIndex >= 0 && unitIndex < m_unitInfo.count())
	{
		m_signal.setInputUnitID(m_unitInfo.key(unitIndex));
	}
	else
	{
		m_signal.setInputUnitID(NO_UNIT_ID);
	}
	int sensorIndex = m_enumManager->value(m_inputSensorProperty);
	if (sensorIndex >= 0 && sensorIndex < SENSOR_TYPE_COUNT)
	{
		m_signal.setInputSensorID(sensorIndex);
	}

	// Output sensor
	//
	m_signal.setOutputLowLimit(m_doubleManager->value(m_outputLowLimitProperty));
	m_signal.setOutputHighLimit(m_doubleManager->value(m_outputHighLimitProperty));
	unitIndex = m_enumManager->value(m_outputUnitProperty);
	if (unitIndex >= 0 && unitIndex < m_unitInfo.count())
	{
		m_signal.setOutputUnitID(m_unitInfo.key(unitIndex));
	}
	else
	{
		m_signal.setOutputUnitID(NO_UNIT_ID);
	}
	sensorIndex = m_enumManager->value(m_outputSensorProperty);
	if (sensorIndex >= 0 && sensorIndex < SENSOR_TYPE_COUNT)
	{
		m_signal.setOutputSensorID(sensorIndex);
	}
	int outputRangeModeIndex = m_enumManager->value(m_outputRangeModeProperty);
	if (outputRangeModeIndex >= 0 && outputRangeModeIndex < OUTPUT_RANGE_MODE_COUNT)
	{
		m_signal.setOutputRangeMode(OutputRangeMode(outputRangeModeIndex));
	}

	m_signal.setAcquire(m_boolManager->value(m_acquireProperty));
	m_signal.setCalculated(m_boolManager->value(m_calculatedProperty));
	m_signal.setNormalState(m_intManager->value(m_normalStateProperty));
	m_signal.setDecimalPlaces(m_intManager->value(m_decimalPlacesProperty));
	m_signal.setAperture(m_doubleManager->value(m_apertureProperty));
	m_signal.setFilteringTime(m_doubleManager->value(m_filteringTimeProperty));
	m_signal.setMaxDifference(m_doubleManager->value(m_maxDifferenceProperty));
	int inOutTypeIndex = m_enumManager->value(m_inOutTypeProperty);
	if (inOutTypeIndex >= 0 && inOutTypeIndex < IN_OUT_TYPE_COUNT)
	{
		m_signal.setInOutType(SignalInOutType(inOutTypeIndex));
	}
	int byteOrderIndex = m_enumManager->value(m_byteOrderProperty);

	if (byteOrderIndex >= 0 && byteOrderIndex < TO_INT(ByteOrder::Count))
	{
		m_signal.setByteOrder(ByteOrder(byteOrderIndex));
	}
	m_signal.setDeviceStrID(m_stringManager->value(m_deviceIDProperty));

	accept();
}


void SignalPropertiesDialog::saveDialogSettings()
{
	QSettings settings;
	settings.setValue("Signal properties dialog: size", size());
	settings.setValue("Signal properties dialog: input property: expanded", m_browser->isExpanded(m_browser->items(m_inputTreeProperty)[0]));
	settings.setValue("Signal properties dialog: output property: expanded", m_browser->isExpanded(m_browser->items(m_outputTreeProperty)[0]));
}

void SignalPropertiesDialog::checkoutSignal()
{
	if (m_signalsModel == nullptr)
	{
		return;
	}

	int row = m_signalsModel->getKeyIndex(m_signal.ID());
	QString message;
	if (!m_signalsModel->checkoutSignal(row, message) && !message.isEmpty())
	{
		emit onError(message);
		return;
	}
}
