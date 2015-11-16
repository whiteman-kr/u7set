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
#include "../include/PropertyEditor.h"
#include <QHBoxLayout>


bool isSameFieldValue(QVector<Signal*>& signalVector, std::function<bool (Signal*, Signal*)> equalityOf2Signals)
{
	for (int i = 0; i < signalVector.count() - 1; i++)
	{
		for (int j = i + 1; j < signalVector.count(); j++)
		{
			if (!equalityOf2Signals(signalVector[i], signalVector[j]))
			{
				return false;
			}
		}
	}
	return true;
}


#define SET_PROPERTY_VALUE(manager, propertyName, fieldName) manager->setValue(propertyName, signalVector[0]->fieldName());

#define SET_STRING_PROPERTY_VALUE(propertyName, fieldName) \
	if (isSameFieldValue(signalVector, [](Signal* first, Signal* second){ return first->fieldName()==second->fieldName(); })) \
		m_stringManager->setValue(propertyName, signalVector[0]->fieldName());

#define SET_ENUM_PROPERTY_VALUE(manager, propertyName, enumInfo, fieldName) manager->setValue(propertyName, enumInfo.keyIndex(signalVector[0]->fieldName()));

#define SET_SIGNAL_STRING_FIELD_VALUE(setter, value) \
	if (m_signalVector[0]->value() != value) \
		signal.setter(value);

#define SET_SIGNAL_FIELD_VALUE(getter, setter, manager, property) \
	if (m_signalVector[0]->getter() != manager->value(property)) \
		signal.setter(manager->value(property));

#define SET_SIGNAL_UNIT_FIELD_VALUE(getter, setter, property) \
	{ \
		int unitIndex = m_enumManager->value(property); \
		if (unitIndex >= 0 && unitIndex < m_unitInfo.count()) \
		{ \
			int unitID = m_unitInfo.key(unitIndex); \
			if (m_signalVector[0]->getter() != unitID) \
				signal.setter(unitID); \
		} \
		else \
			signal.setter(NO_UNIT_ID); \
	}

#define SET_SIGNAL_LIST_FIELD_VALUE(listSize, getter, setter, property) \
	{ \
		int index = m_enumManager->value(property); \
		if (index >= 0 && index < listSize && m_signalVector[0]->getter() != index) \
		{ \
			signal.setter(index); \
		} \
	}

#define SAVE_PROPERTY_VALUE(type, propertyName) settings.setValue("LastEditedSignal: "#propertyName, m_##type##Manager->value(m_##propertyName##Property))
#define SAVE_PROPERTY_VALUE2(type, propertyName, localPropertyName) settings.setValue("LastEditedSignal: "#propertyName, m_##type##Manager->value(m_##localPropertyName##Property))


SignalPropertiesDialog::SignalPropertiesDialog(Signal& signal, E::SignalType signalType, DataFormatList &dataFormatInfo, UnitList &unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent) :
	SignalPropertiesDialog::SignalPropertiesDialog(QVector<Signal*>() << &signal, signalType, dataFormatInfo, unitInfo, readOnly, signalsModel, parent)
{

}

SignalPropertiesDialog::SignalPropertiesDialog(QVector<Signal*> signalVector, E::SignalType signalType, DataFormatList &dataFormatInfo, UnitList &unitInfo, bool readOnly, SignalsModel* signalsModel, QWidget *parent) :
	QDialog(parent),
	m_signalVector(signalVector),
	m_dataFormatInfo(dataFormatInfo),
	m_unitInfo(unitInfo),
	m_signalsModel(signalsModel),
	m_signalType(signalType)
{
	QSettings settings;
	QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
	m_stringManager = new QtStringPropertyManager(this);
	m_enumManager = new QtEnumPropertyManager(this);
	m_intManager = new QtIntPropertyManager(this);
	m_doubleManager = new QtDoublePropertyManager(this);
	m_boolManager = new QtBoolPropertyManager(this);

	QtProperty *signalProperty = groupManager->addProperty(tr("Signal"));

	if (signalVector.count() == 1)
	{
		m_strIDProperty = m_stringManager->addProperty(tr("ID"));
		QString strID = signalVector[0]->strID();
		if (strID[0] != '#')
		{
			strID = '#' + strID;
		}
		m_stringManager->setValue(m_strIDProperty, strID);
		QRegExp rx4ID("^#[A-Za-z][A-Za-z\\d_]*$");
		m_stringManager->setRegExp(m_strIDProperty, rx4ID);
		m_stringManager->setReadOnly(m_strIDProperty, readOnly);
		signalProperty->addSubProperty(m_strIDProperty);
	}

	m_extStrIDProperty = m_stringManager->addProperty(tr("External ID"));
	SET_STRING_PROPERTY_VALUE(m_extStrIDProperty, extStrID);
	QRegExp rx4ExtID("^[A-Za-z][A-Za-z\\d_]*$");
	m_stringManager->setRegExp(m_extStrIDProperty, rx4ExtID);
	m_stringManager->setReadOnly(m_extStrIDProperty, readOnly);
	signalProperty->addSubProperty(m_extStrIDProperty);

	m_nameProperty = m_stringManager->addProperty(tr("Name"));
	SET_STRING_PROPERTY_VALUE(m_nameProperty, name);
	QRegExp rx4Name("^.+$");
	m_stringManager->setRegExp(m_nameProperty, rx4Name);
	m_stringManager->setReadOnly(m_nameProperty, readOnly);
	signalProperty->addSubProperty(m_nameProperty);

	m_dataFormatProperty = m_enumManager->addProperty(tr("Data format"));
	if (signalType == E::SignalType::Analog)
	{
		m_dataFormatInfo.remove(TO_INT(E::DataFormat::UnsignedInt));
	}
	m_enumManager->setEnumNames(m_dataFormatProperty, m_dataFormatInfo.toList());
	if (signalType == E::SignalType::Analog && signalVector[0]->dataFormat() == E::DataFormat::UnsignedInt)
	{
		m_enumManager->setValue(m_dataFormatProperty, m_dataFormatInfo.keyIndex(TO_INT(E::DataFormat::SignedInt)));
	}
	else
	{
		SET_ENUM_PROPERTY_VALUE(m_enumManager, m_dataFormatProperty, m_dataFormatInfo, dataFormatInt);
	}
	signalProperty->addSubProperty(m_dataFormatProperty);

	m_dataSizeProperty = m_intManager->addProperty(tr("Data size"));
	m_intManager->setRange(m_dataSizeProperty, 1, 100);
	if (signalType == E::SignalType::Analog)
	{
		SET_PROPERTY_VALUE(m_intManager, m_dataSizeProperty, dataSize);
		m_intManager->setReadOnly(m_dataSizeProperty, readOnly);
	}
	else
	{
		m_intManager->setValue(m_dataSizeProperty, 1);
		m_intManager->setReadOnly(m_dataSizeProperty, true);
	}
	signalProperty->addSubProperty(m_dataSizeProperty);

	if (signalType == E::SignalType::Analog)
	{
		m_lowAdcProperty = m_intManager->addProperty(tr("Low ADC"));
		m_intManager->setRange(m_lowAdcProperty, 0, 65535);
		SET_PROPERTY_VALUE(m_intManager, m_lowAdcProperty, lowADC);
		m_intManager->setReadOnly(m_lowAdcProperty, readOnly);
		signalProperty->addSubProperty(m_lowAdcProperty);

		m_highAdcProperty = m_intManager->addProperty(tr("High ADC"));
		m_intManager->setRange(m_highAdcProperty, 0, 65535);
		SET_PROPERTY_VALUE(m_intManager, m_highAdcProperty, highADC);
		m_intManager->setReadOnly(m_highAdcProperty, readOnly);
		signalProperty->addSubProperty(m_highAdcProperty);

		m_lowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_lowLimitProperty, lowLimit);
		m_doubleManager->setReadOnly(m_lowLimitProperty, readOnly);
		signalProperty->addSubProperty(m_lowLimitProperty);

		m_highLimitProperty = m_doubleManager->addProperty(tr("High limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_highLimitProperty, highLimit);
		m_doubleManager->setReadOnly(m_highLimitProperty, readOnly);
		signalProperty->addSubProperty(m_highLimitProperty);

		QStringList unitStringList = unitInfo.toList();
		m_unitProperty = m_enumManager->addProperty(tr("Unit"));
		m_enumManager->setEnumNames(m_unitProperty, unitStringList);
		SET_ENUM_PROPERTY_VALUE(m_doubleManager, m_unitProperty, unitInfo, unitID);
		signalProperty->addSubProperty(m_unitProperty);

		m_adjustmentProperty = m_doubleManager->addProperty(tr("Adjustment"));
		SET_PROPERTY_VALUE(m_doubleManager, m_adjustmentProperty, adjustment);
		m_doubleManager->setReadOnly(m_adjustmentProperty, readOnly);
		signalProperty->addSubProperty(m_adjustmentProperty);

		m_dropLimitProperty = m_doubleManager->addProperty(tr("Drop limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_dropLimitProperty, dropLimit);
		m_doubleManager->setReadOnly(m_dropLimitProperty, readOnly);
		signalProperty->addSubProperty(m_dropLimitProperty);

		m_excessLimitProperty = m_doubleManager->addProperty(tr("Excess limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_excessLimitProperty, excessLimit);
		m_doubleManager->setReadOnly(m_excessLimitProperty, readOnly);
		signalProperty->addSubProperty(m_excessLimitProperty);

		m_unbalanceLimitProperty = m_doubleManager->addProperty(tr("Unbalance limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_unbalanceLimitProperty, unbalanceLimit);
		m_doubleManager->setReadOnly(m_unbalanceLimitProperty, readOnly);
		signalProperty->addSubProperty(m_unbalanceLimitProperty);

		// Input sensor
		//
		m_inputTreeProperty = groupManager->addProperty(tr("Input sensor"));

		m_inputLowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_inputLowLimitProperty, inputLowLimit);
		m_doubleManager->setReadOnly(m_inputLowLimitProperty, readOnly);
		m_inputTreeProperty->addSubProperty(m_inputLowLimitProperty);

		m_inputHighLimitProperty = m_doubleManager->addProperty(tr("High limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_inputHighLimitProperty, inputHighLimit);
		m_doubleManager->setReadOnly(m_inputHighLimitProperty, readOnly);
		m_inputTreeProperty->addSubProperty(m_inputHighLimitProperty);

		m_inputUnitProperty = m_enumManager->addProperty(tr("Unit"));
		m_enumManager->setEnumNames(m_inputUnitProperty, unitStringList);
		SET_ENUM_PROPERTY_VALUE(m_enumManager, m_inputUnitProperty, unitInfo, inputUnitID);
		m_inputTreeProperty->addSubProperty(m_inputUnitProperty);

		QStringList sensorNames;
		for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
		{
			sensorNames << SensorTypeStr[i];
		}
		m_inputSensorProperty = m_enumManager->addProperty(tr("Sensor type"));
		m_enumManager->setEnumNames(m_inputSensorProperty, sensorNames);
		SET_PROPERTY_VALUE(m_enumManager, m_inputSensorProperty, inputSensorID);
		m_inputTreeProperty->addSubProperty(m_inputSensorProperty);

		signalProperty->addSubProperty(m_inputTreeProperty);

		// Output sensor
		//
		m_outputTreeProperty = groupManager->addProperty(tr("Output sensor"));

		m_outputLowLimitProperty = m_doubleManager->addProperty(tr("Low limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_outputLowLimitProperty, outputLowLimit);
		m_doubleManager->setReadOnly(m_outputLowLimitProperty, readOnly);
		m_outputTreeProperty->addSubProperty(m_outputLowLimitProperty);

		m_outputHighLimitProperty = m_doubleManager->addProperty(tr("High limit"));
		SET_PROPERTY_VALUE(m_doubleManager, m_outputHighLimitProperty, outputHighLimit);
		m_doubleManager->setReadOnly(m_outputHighLimitProperty, readOnly);
		m_outputTreeProperty->addSubProperty(m_outputHighLimitProperty);

		m_outputUnitProperty = m_enumManager->addProperty(tr("Unit"));
		m_enumManager->setEnumNames(m_outputUnitProperty, unitStringList);
		SET_ENUM_PROPERTY_VALUE(m_enumManager, m_outputUnitProperty, unitInfo, outputUnitID);
		m_outputTreeProperty->addSubProperty(m_outputUnitProperty);

		QStringList outputRangeModeNames;
		for (int i = 0; i < OUTPUT_RANGE_MODE_COUNT; i++)
		{
			outputRangeModeNames << OutputRangeModeStr[i];
		}
		m_outputRangeModeProperty = m_enumManager->addProperty(tr("Output range mode"));
		m_enumManager->setEnumNames(m_outputRangeModeProperty, outputRangeModeNames);
		SET_PROPERTY_VALUE(m_enumManager, m_outputRangeModeProperty, outputRangeMode);
		m_outputTreeProperty->addSubProperty(m_outputRangeModeProperty);

		m_outputSensorProperty = m_enumManager->addProperty(tr("Sensor type"));
		m_enumManager->setEnumNames(m_outputSensorProperty, sensorNames);
		SET_PROPERTY_VALUE(m_enumManager, m_outputSensorProperty, outputSensorID);
		m_outputTreeProperty->addSubProperty(m_outputSensorProperty);

		signalProperty->addSubProperty(m_outputTreeProperty);
	}

	m_acquireProperty = m_boolManager->addProperty(tr("Acquire"));
	SET_PROPERTY_VALUE(m_boolManager, m_acquireProperty, acquire);
	signalProperty->addSubProperty(m_acquireProperty);

	if (signalType == E::SignalType::Analog)
	{
		m_calculatedProperty = m_boolManager->addProperty(tr("Calculated"));
		SET_PROPERTY_VALUE(m_boolManager, m_calculatedProperty, calculated);
		signalProperty->addSubProperty(m_calculatedProperty);

		m_normalStateProperty = m_intManager->addProperty(tr("Normal state"));
		SET_PROPERTY_VALUE(m_intManager, m_normalStateProperty, normalState);
		m_intManager->setReadOnly(m_normalStateProperty, readOnly);
		signalProperty->addSubProperty(m_normalStateProperty);

		m_decimalPlacesProperty = m_intManager->addProperty(tr("Decimal places"));
		SET_PROPERTY_VALUE(m_intManager, m_decimalPlacesProperty, decimalPlaces);
		m_intManager->setReadOnly(m_decimalPlacesProperty, readOnly);
		signalProperty->addSubProperty(m_decimalPlacesProperty);

		m_apertureProperty = m_doubleManager->addProperty(tr("Aperture"));
		SET_PROPERTY_VALUE(m_doubleManager, m_apertureProperty, aperture);
		m_doubleManager->setReadOnly(m_apertureProperty, readOnly);
		signalProperty->addSubProperty(m_apertureProperty);

		m_filteringTimeProperty = m_doubleManager->addProperty(tr("Filtering time"));
		SET_PROPERTY_VALUE(m_doubleManager, m_filteringTimeProperty, filteringTime);
		m_doubleManager->setReadOnly(m_filteringTimeProperty, readOnly);
		signalProperty->addSubProperty(m_filteringTimeProperty);

		m_maxDifferenceProperty = m_doubleManager->addProperty(tr("Max difference"));
		SET_PROPERTY_VALUE(m_doubleManager, m_maxDifferenceProperty, maxDifference);
		m_doubleManager->setReadOnly(m_maxDifferenceProperty, readOnly);
		signalProperty->addSubProperty(m_maxDifferenceProperty);
	}

	QStringList inOutStringList;
	for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
	{
		inOutStringList << InOutTypeStr[i];
	}
	m_inOutTypeProperty = m_enumManager->addProperty(tr("Input-output type"));
	m_enumManager->setEnumNames(m_inOutTypeProperty, inOutStringList);
	SET_PROPERTY_VALUE(m_enumManager, m_inOutTypeProperty, inOutType);
	signalProperty->addSubProperty(m_inOutTypeProperty);


	auto byteOrderValues = E::enumValues<E::ByteOrder>();

	QStringList byteOrderStringList;
	for (auto bo : byteOrderValues)
	{
		byteOrderStringList << QString(bo.second);
	}

	m_byteOrderProperty = m_enumManager->addProperty(tr("Byte order"));
	m_enumManager->setEnumNames(m_byteOrderProperty, byteOrderStringList);
	SET_PROPERTY_VALUE(m_enumManager, m_byteOrderProperty, byteOrderInt);
	signalProperty->addSubProperty(m_byteOrderProperty);

	m_deviceIDProperty = m_stringManager->addProperty(tr("Device ID"));
	SET_STRING_PROPERTY_VALUE(m_deviceIDProperty, deviceStrID);
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
	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(m_browser);
	/*ExtWidgets::PropertyEditor* pe = new ExtWidgets::PropertyEditor(this);

	QList<std::shared_ptr<PropertyObject>> objList;
	for (int i = 0; i < signalVector.count(); i++)
	{
		objList.push_back(std::shared_ptr<Signal>(new Signal(*signalVector[i])));
	}
	pe->setObjects(objList);
	hl->addWidget(pe);*/
	vl->addLayout(hl);
	//vl->addWidget(m_browser);

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
	if (signalType == E::SignalType::Analog)
	{
		m_browser->setExpanded(m_browser->items(m_inputTreeProperty)[0], settings.value("Signal properties dialog: input property: expanded", false).toBool());
		m_browser->setExpanded(m_browser->items(m_outputTreeProperty)[0], settings.value("Signal properties dialog: output property: expanded", false).toBool());
	}
}


void SignalPropertiesDialog::checkAndSaveSignal()
{
	QString strID = m_stringManager->value(m_strIDProperty);
	QRegExp rx4ID("^#[A-Za-z][A-Za-z\\d_]*$");
	if (m_signalVector.count() == 1 && !rx4ID.exactMatch(strID))
	{
		QMessageBox::information(this, tr("Information"), tr("You have to set ID in correct format, "
															 "it should begin from '#' and latin letter "
															 "and be followed by any number of latin letters, '_' and digits"));
		return;
	}
	QString extStrID = m_stringManager->value(m_extStrIDProperty);
	QRegExp rx4ExtID("^[A-Za-z][A-Za-z\\d_]*$");
	if ((m_signalVector.count() == 1 && !rx4ExtID.exactMatch(extStrID)) ||
			(m_signalVector.count() != 1 && !extStrID.isEmpty() && !rx4ExtID.exactMatch(extStrID)))
	{
		QMessageBox::information(this, tr("Information"), tr("You have to set External ID in correct format, "
															 "it should begin from latin letter "
															 "and be followed by any number of latin letters, '_' and digits"));
		return;
	}
	QString name = m_stringManager->value(m_nameProperty);
	if (m_signalVector.count() == 1 && name.isEmpty())
	{
		QMessageBox::information(this, tr("Information"), tr("You should fill signal Name, it could not be empty"));
		return;
	}
	for (int i = m_signalVector.count() - 1; i >= 0; i--)
	{
		Signal& signal = *m_signalVector[i];

		if (!strID.isEmpty())
		{
			SET_SIGNAL_STRING_FIELD_VALUE(setStrID, strID);
		}
		if (!extStrID.isEmpty())
		{
			SET_SIGNAL_STRING_FIELD_VALUE(setExtStrID, extStrID);
		}
		if (!name.isEmpty())
		{
			SET_SIGNAL_STRING_FIELD_VALUE(setName, name);
		}
		int dataFormatIndex = m_enumManager->value(m_dataFormatProperty);
		if (dataFormatIndex >= 0 && dataFormatIndex < m_dataFormatInfo.count())
		{
			E::DataFormat dataFormat = static_cast<E::DataFormat>(m_dataFormatInfo.key(dataFormatIndex));
			if (dataFormat != m_signalVector[0]->dataFormat())
			{
				signal.setDataFormat(dataFormat);
			}
		}
		SET_SIGNAL_FIELD_VALUE(dataSize, setDataSize, m_intManager, m_dataSizeProperty);
		SET_SIGNAL_FIELD_VALUE(lowADC, setLowADC, m_intManager, m_lowAdcProperty);
		SET_SIGNAL_FIELD_VALUE(highADC, setHighADC, m_intManager, m_highAdcProperty);
		SET_SIGNAL_FIELD_VALUE(lowLimit, setLowLimit, m_doubleManager, m_lowLimitProperty);
		SET_SIGNAL_FIELD_VALUE(highLimit, setHighLimit, m_doubleManager, m_highLimitProperty);
		SET_SIGNAL_UNIT_FIELD_VALUE(unitID, setUnitID, m_unitProperty);
		SET_SIGNAL_FIELD_VALUE(adjustment, setAdjustment, m_doubleManager, m_adjustmentProperty);
		SET_SIGNAL_FIELD_VALUE(dropLimit, setDropLimit, m_doubleManager, m_dropLimitProperty);
		SET_SIGNAL_FIELD_VALUE(excessLimit, setExcessLimit, m_doubleManager, m_excessLimitProperty);
		SET_SIGNAL_FIELD_VALUE(unbalanceLimit, setUnbalanceLimit, m_doubleManager, m_unbalanceLimitProperty);

		// Input sensor
		//
		SET_SIGNAL_FIELD_VALUE(inputLowLimit, setInputLowLimit, m_doubleManager, m_inputLowLimitProperty);
		SET_SIGNAL_FIELD_VALUE(inputHighLimit, setInputHighLimit, m_doubleManager, m_inputHighLimitProperty);
		SET_SIGNAL_UNIT_FIELD_VALUE(inputUnitID, setInputUnitID, m_inputUnitProperty);
		SET_SIGNAL_LIST_FIELD_VALUE(SENSOR_TYPE_COUNT, inputSensorID, setInputSensorID, m_inputSensorProperty);

		// Output sensor
		//
		SET_SIGNAL_FIELD_VALUE(outputLowLimit, setOutputLowLimit, m_doubleManager, m_outputLowLimitProperty);
		SET_SIGNAL_FIELD_VALUE(outputHighLimit, setOutputHighLimit, m_doubleManager, m_outputHighLimitProperty);
		SET_SIGNAL_UNIT_FIELD_VALUE(outputUnitID, setOutputUnitID, m_outputUnitProperty);
		SET_SIGNAL_LIST_FIELD_VALUE(SENSOR_TYPE_COUNT, outputSensorID, setOutputSensorID, m_outputSensorProperty);

		int outputRangeModeIndex = m_enumManager->value(m_outputRangeModeProperty);
		if (outputRangeModeIndex >= 0 && outputRangeModeIndex < OUTPUT_RANGE_MODE_COUNT && m_signalVector[0]->outputRangeMode() != outputRangeModeIndex)
		{
			signal.setOutputRangeMode(static_cast<E::OutputRangeMode>(outputRangeModeIndex));
		}

		SET_SIGNAL_FIELD_VALUE(acquire, setAcquire, m_boolManager, m_acquireProperty);
		SET_SIGNAL_FIELD_VALUE(calculated, setCalculated, m_boolManager, m_calculatedProperty);
		SET_SIGNAL_FIELD_VALUE(normalState, setNormalState, m_intManager, m_normalStateProperty);
		SET_SIGNAL_FIELD_VALUE(decimalPlaces, setDecimalPlaces, m_intManager, m_decimalPlacesProperty);
		SET_SIGNAL_FIELD_VALUE(aperture, setAperture, m_doubleManager, m_apertureProperty);
		SET_SIGNAL_FIELD_VALUE(filteringTime, setFilteringTime, m_doubleManager, m_filteringTimeProperty);
		SET_SIGNAL_FIELD_VALUE(maxDifference, setMaxDifference, m_doubleManager, m_maxDifferenceProperty);
		int inOutTypeIndex = m_enumManager->value(m_inOutTypeProperty);
		if (inOutTypeIndex >= 0 && inOutTypeIndex < IN_OUT_TYPE_COUNT && m_signalVector[0]->inOutType() != inOutTypeIndex)
		{
			signal.setInOutType(static_cast<E::SignalInOutType>(inOutTypeIndex));
		}

		int byteOrderIndex = m_enumManager->value(m_byteOrderProperty);

		if (m_signalVector[0]->byteOrder() != static_cast<E::ByteOrder>(byteOrderIndex))
		{
			signal.setByteOrder(static_cast<E::ByteOrder>(byteOrderIndex));
		}

		QString deviceStrID = m_stringManager->value(m_deviceIDProperty);
		SET_SIGNAL_STRING_FIELD_VALUE(setDeviceStrID, deviceStrID);
	}

	saveLastEditedSignalProperties();

	accept();
}


void SignalPropertiesDialog::saveDialogSettings()
{
	QSettings settings;
	settings.setValue("Signal properties dialog: size", size());
	if (m_signalType == E::SignalType::Analog)
	{
		settings.setValue("Signal properties dialog: input property: expanded", m_browser->isExpanded(m_browser->items(m_inputTreeProperty)[0]));
		settings.setValue("Signal properties dialog: output property: expanded", m_browser->isExpanded(m_browser->items(m_outputTreeProperty)[0]));
	}
}

void SignalPropertiesDialog::checkoutSignal()
{
	if (m_signalsModel == nullptr)
	{
		return;
	}

	for (int i = 0; i < m_signalVector.count(); i++)
	{
		int row = m_signalsModel->keyIndex(m_signalVector[i]->ID());
		QString message;
		if (!m_signalsModel->checkoutSignal(row, message) && !message.isEmpty())
		{
			emit onError(message);
			return;
		}
	}
}

void SignalPropertiesDialog::saveLastEditedSignalProperties()
{
	QSettings settings;

	SAVE_PROPERTY_VALUE(enum, dataFormat);
	SAVE_PROPERTY_VALUE(int, dataSize);
	SAVE_PROPERTY_VALUE2(int, lowADC, lowAdc);
	SAVE_PROPERTY_VALUE2(int, highADC, highAdc);
	SAVE_PROPERTY_VALUE(double, lowLimit);
	SAVE_PROPERTY_VALUE(double, highLimit);
	SAVE_PROPERTY_VALUE2(enum, unitID, unit);
	SAVE_PROPERTY_VALUE(double, adjustment);
	SAVE_PROPERTY_VALUE(double, dropLimit);
	SAVE_PROPERTY_VALUE(double, excessLimit);
	SAVE_PROPERTY_VALUE(double, unbalanceLimit);

	SAVE_PROPERTY_VALUE(double, inputLowLimit);
	SAVE_PROPERTY_VALUE(double, inputHighLimit);
	SAVE_PROPERTY_VALUE2(enum, inputUnitID, inputUnit);
	SAVE_PROPERTY_VALUE2(enum, inputSensorID, inputSensor);

	SAVE_PROPERTY_VALUE(double, outputLowLimit);
	SAVE_PROPERTY_VALUE(double, outputHighLimit);
	SAVE_PROPERTY_VALUE2(enum, outputUnitID, outputUnit);
	SAVE_PROPERTY_VALUE2(enum, outputSensorID, outputSensor);

	SAVE_PROPERTY_VALUE(enum, outputRangeMode);

	SAVE_PROPERTY_VALUE(bool, acquire);
	SAVE_PROPERTY_VALUE(bool, calculated);
	SAVE_PROPERTY_VALUE(int, normalState);
	SAVE_PROPERTY_VALUE(int, decimalPlaces);
	SAVE_PROPERTY_VALUE(double, aperture);
	SAVE_PROPERTY_VALUE(double, filteringTime);
	SAVE_PROPERTY_VALUE(double, maxDifference);
	SAVE_PROPERTY_VALUE(enum, inOutType);
	SAVE_PROPERTY_VALUE(enum, byteOrder);
}
