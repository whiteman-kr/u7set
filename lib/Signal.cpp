#include "../include/Signal.h"


DataFormatList::DataFormatList()
{
	for(int i = 0; i < ENUM_COUNT(DataFormat); i++)
	{
		append(i, DataFormatStr[i]);
	}
}


Signal::Signal() :
	QObject()
{
}


Signal::Signal(const Hardware::DeviceSignal& deviceSignal)
{
	Hardware::DeviceSignal::SignalType deviceSignalType = deviceSignal.type();

	if (deviceSignalType == Hardware::DeviceSignal::SignalType::InputAnalog ||
		deviceSignalType == Hardware::DeviceSignal::SignalType::OutputAnalog)
	{
		m_type = SignalType::Analog;
	}
	else
	{
		if (deviceSignalType == Hardware::DeviceSignal::SignalType::InputDiscrete ||
			deviceSignalType == Hardware::DeviceSignal::SignalType::OutputDiscrete)
		{
			m_type = SignalType::Discrete;
		}
		else
		{
			assert(false);			// invalid deviceSignalType
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

	if (m_type == SignalType::Analog)
	{
		m_dataFormat = DataFormat::SignedInt;
	}
	else
	{
		m_dataFormat = DataFormat::UnsignedInt;
	}

#pragma message("################################## uncomment this line")
	//m_dataSize = deviceSignal.dataSize();
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
