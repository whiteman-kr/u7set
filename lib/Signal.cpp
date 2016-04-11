#include "../include/Signal.h"
#include <QXmlStreamAttributes>
#include <QFile>

DataFormatList::DataFormatList()
{
	auto enumValues = E::enumValues<E::DataFormat>();

	for (auto v : enumValues)
	{
		append(v.first, v.second);
	}
}


std::shared_ptr<UnitList> Signal::m_unitList = std::make_shared<UnitList>();


Signal::Signal() :
	PropertyObject()
{
	InitProperties();
}


Signal::Signal(const Hardware::DeviceSignal& deviceSignal) : PropertyObject()
{
	if (deviceSignal.isDiagSignal())
	{
		assert(false);
		return;
	}

	if (deviceSignal.isAnalogSignal())
	{
		m_type = E::SignalType::Analog;
	}
	else
	{
		if (deviceSignal.isDiscreteSignal())
		{
			m_type = E::SignalType::Discrete;
		}
		else
		{
			assert(false);			// invalid deviceSignalType
		}
	}

	if (deviceSignal.isInputSignal() || deviceSignal.isValiditySignal())
	{
		m_inOutType = E::SignalInOutType::Input;
	}
	else
	{
		if (deviceSignal.isOutputSignal())
		{
			m_inOutType = E::SignalInOutType::Output;
		}
		else
		{
			m_inOutType = E::SignalInOutType::Internal;
		}
	}

	m_strID = QString("#%1").arg(deviceSignal.strId());
	m_extStrID = deviceSignal.strId();

	QString deviceSignalStrID = deviceSignal.strId();

	int pos = deviceSignalStrID.lastIndexOf(QChar('_'));

	if (pos != -1)
	{
		deviceSignalStrID = deviceSignalStrID.mid(pos + 1);
	}

	m_caption = QString("Signal #%1").arg(deviceSignalStrID);
	m_deviceStrID = deviceSignal.strId();

	if (m_type == E::SignalType::Analog)
	{
		m_dataFormat = E::DataFormat::Float;
		m_dataSize = 32;
	}
	else
	{
		m_dataFormat = E::DataFormat::UnsignedInt;
		m_dataSize = deviceSignal.size();
	}

	InitProperties();
}


Signal& Signal::operator =(const Signal& signal)
{
	m_ID = signal.ID();
	m_signalGroupID = signal.signalGroupID();
	m_signalInstanceID = signal.signalInstanceID();
	m_changesetID = signal.changesetID();
	m_checkedOut = signal.checkedOut();
	m_userID = signal.userID();
	m_channel = signal.channel();
	m_type = signal.type();
	m_created = signal.created();
	m_deleted = signal.deleted();
	m_instanceCreated = signal.instanceCreated();
	m_instanceAction = signal.instanceAction();

	m_strID = signal.strID();
	m_extStrID = signal.extStrID();
	m_caption = signal.caption();
	m_dataFormat = signal.dataFormat();
	m_dataSize = signal.dataSize();
	m_lowADC = signal.lowADC();
	m_highADC = signal.highADC();
	m_lowLimit = signal.lowLimit();
	m_highLimit = signal.highLimit();
	m_unitID = signal.unitID();
	m_adjustment = signal.adjustment();
	m_dropLimit = signal.dropLimit();
	m_excessLimit = signal.excessLimit();
	m_unbalanceLimit = signal.unbalanceLimit();
	m_inputLowLimit = signal.inputLowLimit();
	m_inputHighLimit = signal.inputHighLimit();
	m_inputUnitID = signal.inputUnitID();
	m_inputSensorID = signal.inputSensorID();
	m_outputLowLimit = signal.outputLowLimit();
	m_outputHighLimit = signal.outputHighLimit();
	m_outputUnitID = signal.outputUnitID();
	m_outputRangeMode = signal.outputRangeMode();
	m_outputSensorID = signal.outputSensorID();
	m_acquire = signal.acquire();
	m_calculated = signal.calculated();
	m_normalState = signal.normalState();
	m_decimalPlaces = signal.decimalPlaces();
	m_aperture = signal.aperture();
	m_inOutType = signal.inOutType();
	m_deviceStrID = signal.deviceStrID();
	m_filteringTime = signal.filteringTime();
	m_maxDifference = signal.maxDifference();
	m_byteOrder = signal.byteOrder();
	m_enableTuning = signal.enableTuning();

	return *this;
}

void Signal::InitProperties()
{
	ADD_PROPERTY_GETTER(int, ID, false, Signal::ID);
	ADD_PROPERTY_GETTER(int, SignalGroupID, false, Signal::signalGroupID);
	ADD_PROPERTY_GETTER(int, SignalInstanceID, false, Signal::signalInstanceID);
	ADD_PROPERTY_GETTER(int, ChangesetID, false, Signal::changesetID);
	ADD_PROPERTY_GETTER(bool, CheckedOut, false, Signal::checkedOut);
	ADD_PROPERTY_GETTER(int, UserID, false, Signal::userID);
	ADD_PROPERTY_GETTER(int, Channel, false, Signal::channel);
	ADD_PROPERTY_GETTER(QDateTime, Created, false, Signal::created);
	ADD_PROPERTY_GETTER(bool, Deleted, false, Signal::deleted);
	ADD_PROPERTY_GETTER(QDateTime, InstanceCreated, false, Signal::instanceCreated);
	ADD_PROPERTY_GETTER(E::InstanceAction, InstanceAction, false, Signal::instanceAction);

	ADD_PROPERTY_GETTER_SETTER(E::SignalType, Type, false, Signal::type, Signal::setType);

	auto strIdProperty = ADD_PROPERTY_GETTER_SETTER(QString, StrID, true, Signal::strID, Signal::setStrID);
	strIdProperty->setValidator("^#[A-Za-z][A-Za-z\\d_]*$");
	auto extStrIdProperty = ADD_PROPERTY_GETTER_SETTER(QString, ExtStrID, true, Signal::extStrID, Signal::setExtStrID);
	extStrIdProperty->setValidator("^[A-Za-z][A-Za-z\\d_]*$");
	auto nameProperty = ADD_PROPERTY_GETTER_SETTER(QString, Caption, true, Signal::caption, Signal::setCaption);
	nameProperty->setValidator("^.+$");
	ADD_PROPERTY_GETTER_SETTER(E::DataFormat, DataFormat, true, Signal::dataFormat, Signal::setDataFormat);
	ADD_PROPERTY_GETTER_SETTER(int, DataSize, true, Signal::dataSize, Signal::setDataSize);
	if (isAnalog())
	{
		static std::shared_ptr<OrderedHash<int, QString>> sensorList = std::make_shared<OrderedHash<int, QString>>();
		if (sensorList->isEmpty())
		{
			for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
			{
				sensorList->append(i, SensorTypeStr[i]);
			}
		}
		ADD_PROPERTY_GETTER_SETTER(int, LowADC, true, Signal::lowADC, Signal::setLowADC);
		ADD_PROPERTY_GETTER_SETTER(int, HighADC, true, Signal::highADC, Signal::setHighADC);
		ADD_PROPERTY_GETTER_SETTER(double, LowLimit, true, Signal::lowLimit, Signal::setLowLimit);
		ADD_PROPERTY_GETTER_SETTER(double, HighLimit, true, Signal::highLimit, Signal::setHighLimit);
		ADD_PROPERTY_DYNAMIC_ENUM(Unit, true, m_unitList, Signal::unitID, Signal::setUnitID);
		ADD_PROPERTY_GETTER_SETTER(double, Adjustment, true, Signal::adjustment, Signal::setAdjustment);
		ADD_PROPERTY_GETTER_SETTER(double, DropLimit, true, Signal::dropLimit, Signal::setDropLimit);
		ADD_PROPERTY_GETTER_SETTER(double, ExcessLimit, true, Signal::excessLimit, Signal::setExcessLimit);
		ADD_PROPERTY_GETTER_SETTER(double, UnbalanceLimit, true, Signal::unbalanceLimit, Signal::setUnbalanceLimit);
		auto inputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, InputLowLimit, true, Signal::inputLowLimit, Signal::setInputLowLimit);
		inputLowLimitPropetry->setCategory("Input sensor");
		auto inputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, InputHighLimit, true, Signal::inputHighLimit, Signal::setInputHighLimit);
		inputHighLimitPropetry->setCategory("Input sensor");
		auto inputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(InputUnit, true, m_unitList, Signal::inputUnitID, Signal::setInputUnitID);/*ADD_PROPERTY_GETTER_SETTER(int, InputUnitID, true, Signal::inputUnitID, Signal::setInputUnitID);*/
		inputUnitIDPropetry->setCategory("Input sensor");
		auto inputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(InputSensor, true, sensorList, Signal::inputSensorID, Signal::setInputSensorID);
		inputSensorPropetry->setCategory("Input sensor");
		auto outputLowLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, OutputLowLimit, true, Signal::outputLowLimit, Signal::setOutputLowLimit);
		outputLowLimitPropetry->setCategory("Output sensor");
		auto outputHighLimitPropetry = ADD_PROPERTY_GETTER_SETTER(double, OutputHighLimit, true, Signal::outputHighLimit, Signal::setOutputHighLimit);
		outputHighLimitPropetry->setCategory("Output sensor");
		auto outputUnitIDPropetry = ADD_PROPERTY_DYNAMIC_ENUM(OutputUnit, true, m_unitList, Signal::outputUnitID, Signal::setOutputUnitID);/*ADD_PROPERTY_GETTER_SETTER(int, OutputUnitID, true, Signal::outputUnitID, Signal::setOutputUnitID);*/
		outputUnitIDPropetry->setCategory("Output sensor");
		auto outputRangeModePropetry = ADD_PROPERTY_GETTER_SETTER(E::OutputRangeMode, OutputRangeMode, true, Signal::outputRangeMode, Signal::setOutputRangeMode);
		outputRangeModePropetry->setCategory("Output sensor");
		auto outputSensorPropetry = ADD_PROPERTY_DYNAMIC_ENUM(OutputSensor, true, sensorList, Signal::outputSensorID, Signal::setOutputSensorID);
		outputSensorPropetry->setCategory("Output sensor");
	}
	ADD_PROPERTY_GETTER_SETTER(bool, Acquire, true, Signal::acquire, Signal::setAcquire);
	if (isAnalog())
	{
		ADD_PROPERTY_GETTER_SETTER(bool, Calculated, true, Signal::calculated, Signal::setCalculated);
		ADD_PROPERTY_GETTER_SETTER(int, NormalState, true, Signal::normalState, Signal::setNormalState);
		ADD_PROPERTY_GETTER_SETTER(int, DecimalPlaces, true, Signal::decimalPlaces, Signal::setDecimalPlaces);
		ADD_PROPERTY_GETTER_SETTER(double, Aperture, true, Signal::aperture, Signal::setAperture);
		ADD_PROPERTY_GETTER_SETTER(double, FilteringTime, true, Signal::filteringTime, Signal::setFilteringTime);
		ADD_PROPERTY_GETTER_SETTER(double, MaxDifference, true, Signal::maxDifference, Signal::setMaxDifference);
	}
	ADD_PROPERTY_GETTER_SETTER(E::SignalInOutType, InOutType, true, Signal::inOutType, Signal::setInOutType);
	ADD_PROPERTY_GETTER_SETTER(E::ByteOrder, ByteOrder, true, Signal::byteOrder, Signal::setByteOrder);
	ADD_PROPERTY_GETTER_SETTER(QString, DeviceStrID, true, Signal::deviceStrID, Signal::setDeviceStrID);
	ADD_PROPERTY_GETTER_SETTER(bool, EnableTuning, true, Signal::enableTuning, Signal::setEnableTuning);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(bool))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	if (strValue == "true")
	{
		(this->*setter)(true);
		return;
	}
	if (strValue == "false")
	{
		(this->*setter)(false);
		return;
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	bool ok = false;
	int intValue = strValue.toInt(&ok);
	if (!ok)
	{
		assert(false);
		return;
	}
	(this->*setter)(intValue);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(double))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	bool ok = false;
	double doubleValue = strValue.toDouble(&ok);
	if (!ok)
	{
		assert(false);
		return;
	}
	(this->*setter)(doubleValue);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const QString&))
{
	const QStringRef& strValue = attr.value(fieldName);
	(this->*setter)(strValue.toString());
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalType))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	if (strValue == "Analog")
	{
		(this->*setter)(E::SignalType::Analog);
		return;
	}
	if (strValue == "Discrete")
	{
		(this->*setter)(E::SignalType::Discrete);
		return;
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::OutputRangeMode))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < OUTPUT_RANGE_MODE_COUNT; i++)
	{
		if (strValue == OutputRangeModeStr[i])
		{
			(this->*setter)(static_cast<E::OutputRangeMode>(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
	{
		if (strValue == InOutTypeStr[i])
		{
			(this->*setter)(static_cast<E::SignalInOutType>(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ByteOrder))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}

	auto el = E::enumValues<E::ByteOrder>();

	for (auto e : el)
	{
		if (e.second == strValue)
		{
			(this->*setter)(static_cast<E::ByteOrder>(e.first));
			return;
		}
	}

	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const Address16&))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	Address16 address;
	address.fromString(strValue.toString());
	(this->*setter)(address);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, DataFormatList& dataFormatInfo, void (Signal::*setter)(E::DataFormat))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < dataFormatInfo.count(); i++)
	{
		if (strValue == dataFormatInfo[i])
		{
			(this->*setter)(static_cast<E::DataFormat>(dataFormatInfo.keyAt(i)));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(int))
{
	const QStringRef& strValue = attr.value(fieldName);
	for (int i = 0; i < unitInfo.count(); i++)
	{
		if (strValue == unitInfo[i])
		{
			(this->*setter)(unitInfo.keyAt(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeSensorField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int))
{
	const QStringRef& strValue = attr.value(fieldName);
	if (strValue.isEmpty())
	{
		assert(false);
		return;
	}
	for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
	{
		if (strValue == SensorTypeStr[i])
		{
			(this->*setter)(i);
			return;
		}
	}
	assert(false);
}

void Signal::serializeFields(const QXmlStreamAttributes& attr, DataFormatList& dataFormatInfo, UnitList &unitInfo)
{
	serializeField(attr, "ID", &Signal::setID);
	serializeField(attr, "SignalGroupID", &Signal::setSignalGroupID);
	serializeField(attr, "SignalInstanceID", &Signal::setSignalInstanceID);
	serializeField(attr, "Channel", &Signal::setChannel);
	serializeField(attr, "Type", &Signal::setType);
	serializeField(attr, "StrID", &Signal::setStrID);
	serializeField(attr, "ExtStrID", &Signal::setExtStrID);
	serializeField(attr, "Name", &Signal::setCaption);
	serializeField(attr, "DataFormat", dataFormatInfo, &Signal::setDataFormat);
	serializeField(attr, "DataSize", &Signal::setDataSize);
	serializeField(attr, "LowADC", &Signal::setLowADC);
	serializeField(attr, "HighADC", &Signal::setHighADC);
	serializeField(attr, "LowLimit", &Signal::setLowLimit);
	serializeField(attr, "HighLimit", &Signal::setHighLimit);
	serializeField(attr, "UnitID", unitInfo, &Signal::setUnitID);
	serializeField(attr, "Adjustment", &Signal::setAdjustment);
	serializeField(attr, "DropLimit", &Signal::setDropLimit);
	serializeField(attr, "ExcessLimit", &Signal::setExcessLimit);
	serializeField(attr, "UnbalanceLimit", &Signal::setUnbalanceLimit);
	serializeField(attr, "InputLowLimit", &Signal::setInputLowLimit);
	serializeField(attr, "InputHighLimit", &Signal::setInputHighLimit);
	serializeField(attr, "InputUnitID", unitInfo, &Signal::setInputUnitID);
	serializeSensorField(attr, "InputSensorID", &Signal::setInputSensorID);
	serializeField(attr, "OutputLowLimit", &Signal::setOutputLowLimit);
	serializeField(attr, "OutputHighLimit", &Signal::setOutputHighLimit);
	serializeField(attr, "OutputUnitID", unitInfo, &Signal::setOutputUnitID);
	serializeField(attr, "OutputRangeMode", &Signal::setOutputRangeMode);
	serializeSensorField(attr, "OutputSensorID", &Signal::setOutputSensorID);
	serializeField(attr, "Acquire", &Signal::setAcquire);
	serializeField(attr, "Calculated", &Signal::setCalculated);
	serializeField(attr, "NormalState", &Signal::setNormalState);
	serializeField(attr, "DecimalPlaces", &Signal::setDecimalPlaces);
	serializeField(attr, "Aperture", &Signal::setAperture);
	serializeField(attr, "InOutType", &Signal::setInOutType);
	serializeField(attr, "DeviceStrID", &Signal::setDeviceStrID);
	serializeField(attr, "FilteringTime", &Signal::setFilteringTime);
	serializeField(attr, "MaxDifference", &Signal::setMaxDifference);
	serializeField(attr, "ByteOrder", &Signal::setByteOrder);
	serializeField(attr, "RamAddr", &Signal::setRamAddr);
	serializeField(attr, "RegAddr", &Signal::setRegAddr);
}



bool Signal::isCompatibleDataFormat(Afb::AfbDataFormat afbDataFormat) const
{
	if (m_dataFormat == E::DataFormat::Float && afbDataFormat == Afb::AfbDataFormat::Float)
	{
		return true;
	}

	if (m_dataFormat == E::DataFormat::SignedInt && afbDataFormat == Afb::AfbDataFormat::SignedInt)
	{
		return true;
	}

	if (m_dataFormat == E::DataFormat::UnsignedInt && afbDataFormat == Afb::AfbDataFormat::UnsignedInt)
	{
		return true;
	}

	return false;
}

void Signal::setReadOnly(bool value)
{
	for (auto property : properties())
	{
		property->setReadOnly(value);
	}
}


SignalSet::SignalSet()
{
}


SignalSet::~SignalSet()
{
	clear();
}


void SignalSet::clear()
{
	SignalPtrOrderedHash::clear();

	m_groupSignals.clear();
}


void SignalSet::append(const int& signalID, Signal *signal)
{
	SignalPtrOrderedHash::append(signalID, signal);

	m_groupSignals.insert(signal->signalGroupID(), signalID);
}


void SignalSet::remove(const int& signalID)
{
	Signal signal = value(signalID);

	SignalPtrOrderedHash::remove(signalID);

	m_groupSignals.remove(signal.signalGroupID(), signalID);
}


void SignalSet::removeAt(const int index)
{
	const Signal& signal = SignalPtrOrderedHash::operator [](index);

	int signalGroupID = signal.signalGroupID();
	int signalID = signal.ID();

	SignalPtrOrderedHash::removeAt(index);

	m_groupSignals.remove(signalGroupID, signalID);
}


QVector<int> SignalSet::getChannelSignalsID(const Signal& signal)
{
	return getChannelSignalsID(signal.signalGroupID());
}


QVector<int> SignalSet::getChannelSignalsID(int signalGroupID)
{
	QVector<int> channelSignalsID;

	QList<int> signalsID = m_groupSignals.values(signalGroupID);

	int signalCount = signalsID.count();

	for(int i = 0; i< signalCount; i++)
	{
		channelSignalsID.append(signalsID.at(i));
	}

	return channelSignalsID;
}


void SignalSet::reserve(int n)
{
	SignalPtrOrderedHash::reserve(n);
	m_groupSignals.reserve(n);
}


void SignalSet::resetAddresses()
{
	int signalCount = count();

	for(int i = 0; i < signalCount; i++)
	{
		(*this)[i].resetAddresses();
	}
}


void SerializeSignalsFromXml(const QString& filePath, UnitList& unitInfo, SignalSet& signalSet)
{
	QXmlStreamReader applicationSignalsReader;
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly))
	{
		DataFormatList dataFormatInfo;
		applicationSignalsReader.setDevice(&file);

		while (!applicationSignalsReader.atEnd())
		{
			QXmlStreamReader::TokenType token = applicationSignalsReader.readNext();

			switch (token)
			{
			case QXmlStreamReader::StartElement:
			{
				const QXmlStreamAttributes& attr = applicationSignalsReader.attributes();
				if (applicationSignalsReader.name() == "Unit")
				{
					unitInfo.append(attr.value("ID").toInt(), attr.value("Name").toString());
				}
				if (applicationSignalsReader.name() == "Signal")
				{
					Signal* pSignal = new Signal;
					pSignal->serializeFields(attr, dataFormatInfo, unitInfo);
					signalSet.append(pSignal->ID(), pSignal);
				}
				break;
			}
			default:
				continue;
			}
		}
		if (applicationSignalsReader.hasError())
		{
			qDebug() << "Parse applicationSignals.xml error";
		}
	}
}


void InitDataSources(QHash<quint32, DataSource>& dataSources, Hardware::DeviceObject* deviceRoot, const SignalSet& signalSet)
{
	dataSources.clear();

	if (deviceRoot == nullptr)
	{
		return;
	}

	Hardware::equipmentWalker(deviceRoot, [&dataSources, &signalSet](Hardware::DeviceObject* currentDevice)
	{
		if (currentDevice == nullptr)
		{
			return;
		}
		if (typeid(*currentDevice) != typeid(Hardware::DeviceModule))
		{
			return;
		}
		Hardware::DeviceModule* currentModule = dynamic_cast<Hardware::DeviceModule*>(currentDevice);
		if (currentModule == nullptr)
		{
			return;
		}
		if (currentModule->moduleFamily() != Hardware::DeviceModule::LM)
		{
			return;
		}
		QStringList propertyList = QStringList() << "RegIP1" << "RegIP2";
		for (QString prop : propertyList)
		{
			if (currentModule->propertyValue(prop).isValid())
			{
				int key = dataSources.count() + 1;
				QString ipStr = currentModule->propertyValue(prop).toString();
				QHostAddress ha(ipStr);
				quint32 ip = ha.toIPv4Address();
				DataSource ds(ip, QString("Data Source %1").arg(key), ha, 1);

				QString signalPrefix = currentModule->parent()->strId();
				int signalPrefixLength = signalPrefix.length();
				for (int i = 0; i < signalSet.count(); i++)
				{
					if (signalSet[i].deviceStrID().left(signalPrefixLength) == signalPrefix)
					{
						ds.addSignalIndex(i);
					}
				}
				dataSources.insert(key, ds);
			}
		}
	});
}

