#include "../include/Signal.h"


Signal::Signal() :
	QObject(),
	m_ID(0),
	m_signalGroupID(0),
	m_signalInstanceID(0),
	m_changesetID(0),
	m_checkedOut(false),
	m_userID(0),
	m_channel(1),
	m_type(SignalType::analog),
	m_deleted(false),
	m_instanceAction(InstanceAction::added),

	m_dataFormat(static_cast<int>(DataFormatType::binary_LE_unsigned)),
	m_lowADC(0),
	m_highADC(0),
	m_lowLimit(0),
	m_highLimit(0),
	m_unitID(NO_UNIT_ID),
	m_adjustment(0),
	m_dropLimit(0),
	m_excessLimit(0),
	m_unbalanceLimit(0),
	m_inputLowLimit(0),
	m_inputHighLimit(0),
	m_inputUnitID(NO_UNIT_ID),
	m_inputSensorID(0),
	m_outputLowLimit(0),
	m_outputHighLimit(0),
	m_outputUnitID(NO_UNIT_ID),
	m_outputSensorID(0),
	m_acquire(true),
	m_calculated(false),
	m_normalState(0),
	m_decimalPlaces(2),
	m_aperture(0),
	m_inOutType(SignalInOutType::internal)
{
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
	m_outputSensorID = signal.outputSensorID();
	m_acquire = signal.acquire();
	m_calculated = signal.calculated();
	m_normalState = signal.normalState();
	m_decimalPlaces = signal.decimalPlaces();
	m_aperture = signal.aperture();
	m_inOutType = signal.inOutType();
	m_deviceStrID = signal.deviceStrID();

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
	OrderedHash<int, Signal>::clear();

	m_groupSignals.clear();
}


void SignalSet::append(const int& signalID, const Signal& signal)
{
	OrderedHash<int, Signal>::append(signalID, signal);

	m_groupSignals.insert(signal.signalGroupID(), signalID);
}


void SignalSet::remove(const int& signalID)
{
	Signal signal = value(signalID);

	OrderedHash<int, Signal>::remove(signalID);

	m_groupSignals.remove(signal.signalGroupID(), signalID);
}


void SignalSet::removeAt(const int index)
{
	const Signal& signal = OrderedHash<int, Signal>::operator [](index);

	int signalGroupID = signal.signalGroupID();
	int signalID = signal.ID();

	OrderedHash<int, Signal>::removeAt(index);

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
