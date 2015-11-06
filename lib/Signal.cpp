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


Signal::Signal() :
	QObject()
{
}


Signal::Signal(const Hardware::DeviceSignal& deviceSignal)
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
		m_inOutType = SignalInOutType::Input;
	}
	else
	{
		if (deviceSignal.isOutputSignal())
		{
			m_inOutType = SignalInOutType::Output;
		}
		else
		{
			m_inOutType = SignalInOutType::Internal;
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

	m_name = QString("Signal #%1").arg(deviceSignalStrID);
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
	m_name = signal.name();
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

	return *this;
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

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(OutputRangeMode))
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
			(this->*setter)(OutputRangeMode(i));
			return;
		}
	}
	assert(false);
}

void Signal::serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(SignalInOutType))
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
			(this->*setter)(SignalInOutType(i));
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
			(this->*setter)(static_cast<E::DataFormat>(dataFormatInfo.key(i)));
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
			(this->*setter)(unitInfo.key(i));
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
	serializeField(attr, "signalGroupID", &Signal::setSignalGroupID);
	serializeField(attr, "signalInstanceID", &Signal::setSignalInstanceID);
	serializeField(attr, "channel", &Signal::setChannel);
	serializeField(attr, "type", &Signal::setType);
	serializeField(attr, "strID", &Signal::setStrID);
	serializeField(attr, "extStrID", &Signal::setExtStrID);
	serializeField(attr, "name", &Signal::setName);
	serializeField(attr, "dataFormat", dataFormatInfo, &Signal::setDataFormat);
	serializeField(attr, "dataSize", &Signal::setDataSize);
	serializeField(attr, "lowADC", &Signal::setLowADC);
	serializeField(attr, "highADC", &Signal::setHighADC);
	serializeField(attr, "lowLimit", &Signal::setLowLimit);
	serializeField(attr, "highLimit", &Signal::setHighLimit);
	serializeField(attr, "unitID", unitInfo, &Signal::setUnitID);
	serializeField(attr, "adjustment", &Signal::setAdjustment);
	serializeField(attr, "dropLimit", &Signal::setDropLimit);
	serializeField(attr, "excessLimit", &Signal::setExcessLimit);
	serializeField(attr, "unbalanceLimit", &Signal::setUnbalanceLimit);
	serializeField(attr, "inputLowLimit", &Signal::setInputLowLimit);
	serializeField(attr, "inputHighLimit", &Signal::setInputHighLimit);
	serializeField(attr, "inputUnitID", unitInfo, &Signal::setInputUnitID);
	serializeSensorField(attr, "inputSensorID", &Signal::setInputSensorID);
	serializeField(attr, "outputLowLimit", &Signal::setOutputLowLimit);
	serializeField(attr, "outputHighLimit", &Signal::setOutputHighLimit);
	serializeField(attr, "outputUnitID", unitInfo, &Signal::setOutputUnitID);
	serializeField(attr, "outputRangeMode", &Signal::setOutputRangeMode);
	serializeSensorField(attr, "outputSensorID", &Signal::setOutputSensorID);
	serializeField(attr, "acquire", &Signal::setAcquire);
	serializeField(attr, "calculated", &Signal::setCalculated);
	serializeField(attr, "normalState", &Signal::setNormalState);
	serializeField(attr, "decimalPlaces", &Signal::setDecimalPlaces);
	serializeField(attr, "aperture", &Signal::setAperture);
	serializeField(attr, "inOutType", &Signal::setInOutType);
	serializeField(attr, "deviceStrID", &Signal::setDeviceStrID);
	serializeField(attr, "filteringTime", &Signal::setFilteringTime);
	serializeField(attr, "maxDifference", &Signal::setMaxDifference);
	serializeField(attr, "byteOrder", &Signal::setByteOrder);
	serializeField(attr, "ramAddr", &Signal::setRamAddr);
	serializeField(attr, "regAddr", &Signal::setRegAddr);
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


void Address16::fromString(QString str)
{
	const QStringList& list = str.split(":");
	m_offset = list[0].toInt();
	m_bit = list[1].toInt();
}


void SerializeSignalsFromXml(UnitList& unitInfo, SignalSet& signalSet)
{
	QXmlStreamReader applicationSignalsReader;
	QFile file("applicationSignals.xml");

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
				if (applicationSignalsReader.name() == "unit")
				{
					unitInfo.append(attr.value("ID").toInt(), attr.value("name").toString());
				}
				if (applicationSignalsReader.name() == "signal")
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
		if (currentModule->propertyValue("RegIP").isValid())
		{
			int key = dataSources.count() + 1;
			QString ipStr = currentModule->propertyValue("RegIP").toString();
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
	});
}

