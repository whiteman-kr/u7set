#pragma once

#include <QString>
#include <QMultiHash>
#include "../lib/Types.h"
#include "../lib/DbStruct.h"
#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/Address16.h"
#include "../VFrame30/Afb.h"
#include "../lib/ProtobufHelper.h"
#include "../lib/Hash.h"


class QXmlStreamAttributes;
class XmlWriteHelper;
class XmlReadHelper;

const char* const ElectricUnitStr[] =
{
	"NoUnit",
	"mA",
	"mV",
	"Ohm",
	"V",
};

const int ELECTRIC_UNIT_COUNT = sizeof(ElectricUnitStr) / sizeof(ElectricUnitStr[0]);

const char* const OutputModeStr[] =
{
	"0 .. 5 V",
	"4 .. 20 mA",
	"-10 .. 10 V",
	"0 .. 5 mA",
};

const int OUTPUT_MODE_COUNT = sizeof(OutputModeStr) / sizeof(OutputModeStr[0]);

const char* const SensorTypeStr[] =
{
	"Not used",

	"Pt50 W=1.391",
	"Pt100 W=1.391",
	"Pt50 W=1.385",
	"Pt100 W=1.385",

	"Cu50 W=1.428",
	"Cu100 W=1.428",
	"Cu50 W=1.426",
	"Cu100 W=1.426",

	"Pt21",
	"Cu23",

	"K (TXA)",
	"L (TXK)",
	"N (THH)",
};

const int SENSOR_TYPE_COUNT = sizeof(SensorTypeStr) / sizeof(SensorTypeStr[0]);

const QString DATE_TIME_FORMAT_STR("yyyy-MM-ddTHH:mm:ss");

class Signal
{
	friend class DbWorker;
	friend class SignalSet;
	friend class SignalTests;

public:
	Signal();
	Signal(const Signal& s);
	Signal(const Hardware::DeviceSignal& deviceSignal);
	virtual ~Signal();

	// Signal identificators

	QString appSignalID() const { return m_appSignalID; }
	void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

	QString customAppSignalID() const { return m_customAppSignalID; }
	void setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

	QString caption() const { return m_caption; }
	void setCaption(const QString& caption) { m_caption = caption; }

	QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString busTypeID() const { return m_busTypeID; }
	void setBusTypeID(const QString& busTypeID) { m_busTypeID = busTypeID; }

	E::Channel channel() const { return m_channel; }
	int channelInt() const { return TO_INT(m_channel); }

	// Signal type

	E::SignalType signalType() const { return m_signalType; }
	int signalTypeInt() const { return TO_INT(m_signalType); }
	void setSignalType(E::SignalType type) { m_signalType = type; }

	bool isAnalog() const { return m_signalType == E::SignalType::Analog; }
	bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }
	bool isBus() const { return m_signalType == E::SignalType::Bus; }

	E::SignalInOutType inOutType() const { return m_inOutType; }
	int inOutTypeInt() const { return TO_INT(m_inOutType); }
	void setInOutType(E::SignalInOutType inOutType) { m_inOutType = inOutType; }

	bool isInput() const { return m_inOutType == E::SignalInOutType::Input; }
	bool isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
	bool isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }

	// Signal format

	int dataSize() const { return m_dataSize; }
	void setDataSize(int dataSize) { m_dataSize = dataSize; }
	void setDataSize(E::SignalType signalType, E::AnalogAppSignalFormat dataFormat);

	int sizeW() const { return (m_dataSize / SIZE_16BIT + (m_dataSize % SIZE_16BIT ? 1 : 0)); }

	E::ByteOrder byteOrder() const { return m_byteOrder; }
	int byteOrderInt() const { return TO_INT(m_byteOrder); }
	void setByteOrder(E::ByteOrder byteOrder) { m_byteOrder = byteOrder; }

	E::AnalogAppSignalFormat analogSignalFormat() const { return m_analogSignalFormat; }
	int analogSignalFormatInt() const { return TO_INT(m_analogSignalFormat); }
	void setAnalogSignalFormat(E::AnalogAppSignalFormat dataFormat) { m_analogSignalFormat = dataFormat; }

	E::DataFormat dataFormat() const;

	bool isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const;
	bool isCompatibleFormat(const SignalAddress16& sa16) const;

	// Analog signal properties

	int lowADC() const { return m_lowADC; }
	void setLowADC(int lowADC) { m_lowADC = lowADC; }

	int highADC() const { return m_highADC; }
	void setHighADC(int highADC) { m_highADC = highADC;}

	double lowEngeneeringUnits() const { return m_lowEngeneeringUnits; }
	void setLowEngeneeringUnits(double lowEngeneeringUnits) { m_lowEngeneeringUnits = lowEngeneeringUnits; }

	double highEngeneeringUnits() const { return m_highEngeneeringUnits; }
	void setHighEngeneeringUnits(double highEngeneeringUnits) { m_highEngeneeringUnits = highEngeneeringUnits; }

	double lowValidRange() const { return m_lowValidRange; }
	void setLowValidRange(double lowValidRange) { m_lowValidRange = lowValidRange; }

	double highValidRange() const { return m_highValidRange; }
	void setHighValidRange(double highValidRange) { m_highValidRange = highValidRange; }

	double filteringTime() const { return m_filteringTime; }
	void setFilteringTime(double filteringTime) { m_filteringTime = filteringTime; }

	double spreadTolerance() const { return m_spreadTolerance; }
	void setSpreadTolerance(double spreadTolerance) { m_spreadTolerance = spreadTolerance; }

	// Analog input/output signal properties

	double electricLowLimit() const { return m_electricLowLimit; }
	void setElectricLowLimit(double electricLowLimit) { m_electricLowLimit = electricLowLimit; }

	double electricHighLimit() const { return m_electricHighLimit; }
	void setElectricHighLimit(double electricHighLimit) { m_electricHighLimit = electricHighLimit; }

	E::ElectricUnit electricUnit() const { return m_electricUnit; }
	int electricUnitInt() const { return TO_INT(m_electricUnit); }
	void setElectricUnit(E::ElectricUnit electricUnit) { m_electricUnit = electricUnit; }

	E::SensorType sensorType() const { return m_sensorType; }
	int sensorTypeInt() const { return TO_INT(m_sensorType); }
	void setSensorType(E::SensorType sensorType) { m_sensorType = sensorType; }
	void setSensorTypeInt(int sensorType) { m_sensorType = IntToEnum<E::SensorType>(sensorType); }

	E::OutputMode outputMode() const { return m_outputMode; }
	int outputModeInt() const { return TO_INT(m_outputMode); }
	void setOutputMode(E::OutputMode outputMode) { m_outputMode = outputMode; }
	void setOutputModeInt(int outputMode) { m_outputMode = IntToEnum<E::OutputMode>(outputMode); }

	// Tuning signal properties

	bool enableTuning() const { return m_enableTuning; }
	void setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

	float tuningDefaultValue() const { return m_tuningDefaultValue; }
	void setTuningDefaultValue(float value) { m_tuningDefaultValue = value; }

	float tuningLowBound() const { return m_tuningLowBound; }
	void setTuningLowBound(float value) { m_tuningLowBound = value; }

	float tuningHighBound() const { return m_tuningHighBound; }
	void setTuningHighBound(float value) { m_tuningHighBound = value; }

	// Signal properties for MATS

	bool acquire() const { return m_acquire; }
	void setAcquire(bool acquire) { m_acquire = acquire; }

	bool isAcquired() const { return m_acquire; }

	int decimalPlaces() const { return m_decimalPlaces; }
	void setDecimalPlaces(int decimalPlaces) { m_decimalPlaces = decimalPlaces; }

	double coarseAperture() const { return m_coarseAperture; }
	void setCoarseAperture(double aperture) { m_coarseAperture = aperture; }

	double fineAperture() const { return m_fineAperture; }
	void setFineAperture(double aperture) { m_fineAperture = aperture; }

	bool adaptiveAperture() const { return m_adaptiveAperture; }
	void setAdaptiveAperture(bool adaptive) { m_adaptiveAperture = adaptive; }

	// Signal fields from database

	int ID() const { return m_ID; }
	int signalGroupID() const { return m_signalGroupID; }
	int signalInstanceID() const { return m_signalInstanceID; }
	int changesetID() const { return m_changesetID; }
	bool checkedOut() const { return m_checkedOut; }
	int userID() const { return m_userID; }
	QDateTime created() const { return m_created; }
	bool deleted() const { return m_deleted; }
	QDateTime instanceCreated() const { return m_instanceCreated; }
	VcsItemAction instanceAction() const { return m_instanceAction; }

	// Signal properties calculated in compile-time

	Hash hash() const { assert(m_hash !=0); return m_hash; }
	void setHash(Hash hash) { m_hash = hash; }

	QString unit() const { return m_unit; }
	void setUnit(const QString& unit) { m_unit = unit; }

	Address16 ioBufAddr() const { return m_ioBufAddr; }
	void setIoBufAddr(const Address16& addr) { m_ioBufAddr = addr; }

	Address16 tuningAddr() const { return m_tuningAddr; }
	void setTuningAddr(const Address16& addr) { m_tuningAddr = addr; }

	Address16 ualAddr() const { return m_ualAddr; }
	void setUalAddr(const Address16& addr) { m_ualAddr = addr; }

	Address16 regBufAddr() const { return m_regBufAddr; }
	void setRegBufAddr(const Address16& addr) { m_regBufAddr = addr; }

	Address16 regValueAddr() const { return m_regValueAddr; }
	void setRegValueAddr(const Address16& addr) { m_regValueAddr = addr; }

	Address16 regValidityAddr() const { return m_regValidityAddr; }
	void setRegValidityAddr(const Address16& addr) { m_regValidityAddr = addr; }

	void resetAddresses();

	QString regValueAddrStr() const;

	bool needConversion() const { return m_needConversion; }
	void setNeedConversion(bool need) { m_needConversion = need; }

	std::shared_ptr<Hardware::DeviceModule> lm() const { return m_lm; }
	void setLm(std::shared_ptr<Hardware::DeviceModule> lm) { m_lm = lm; }

	//

	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(bool));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(double));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const QString&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::OutputMode));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ElectricUnit));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SensorType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ByteOrder));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const Address16&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::AnalogAppSignalFormat));

	void serializeFields(const QXmlStreamAttributes& attr);

	void writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);

	void serializeTo(Proto::AppSignal* s) const;
	void serializeFrom(const Proto::AppSignal &s);

	//void serializeToProtoAppSignalParam(Proto::AppSignalParam* message) const;

private:
	// Private setters for fields, witch can't be changed outside DB engine
	// Should be used only by friends
	//
	void setID(int signalID) { m_ID = signalID; }
	void setSignalGroupID(int signalGroupID) { m_signalGroupID = signalGroupID; }
	void setSignalInstanceID(int signalInstanceID) { m_signalInstanceID = signalInstanceID; }
	void setChangesetID(int changesetID) { m_changesetID = changesetID; }
	void setCheckedOut(bool checkedOut) { m_checkedOut = checkedOut; }
	void setUserID(int userID) { m_userID = userID; }
	void setChannel(E::Channel channel) { m_channel = channel; }
	void setCreated(const QDateTime& created) { m_created = created; }
	void setCreated(const QString& createdStr) { m_created = QDateTime::fromString(createdStr, DATE_TIME_FORMAT_STR); }
	void setDeleted(bool deleted) { m_deleted = deleted; }
	void setInstanceCreated(const QDateTime& instanceCreated) { m_instanceCreated = instanceCreated; }
	void setInstanceCreated(const QString& instanceCreatedStr) { m_instanceCreated = QDateTime::fromString(instanceCreatedStr, DATE_TIME_FORMAT_STR); }
	void setInstanceAction(VcsItemAction action) { m_instanceAction = action; }

	//

	void initCalculatedProperties();

private:
	// Signal identificators
	//
	QString m_appSignalID;
	QString m_customAppSignalID;
	QString m_caption;
	QString m_equipmentID;
	QString m_busTypeID;											// only for: m_signalType == E::SignalType::Bus
	E::Channel m_channel = E::Channel::A;

	// Signal type
	//
	E::SignalType m_signalType = E::SignalType::Analog;
	E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;

	// Signal format
	//
	int m_dataSize = 32;											// signal data size in bits
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	// Analog signal properties
	//
	E::AnalogAppSignalFormat m_analogSignalFormat =					// only for m_signalType == E::SignalType::Analog
							E::AnalogAppSignalFormat::Float32;		// discrete signals is always treat as UnsignedInt and dataSize == 1

	QString m_unit;

	int m_lowADC = 0;
	int m_highADC = 0xFFFF;

	double m_lowEngeneeringUnits = 0;								// low physical value for input range
	double m_highEngeneeringUnits = 100;							// high physical value for input range

	double m_lowValidRange = 0;
	double m_highValidRange = 100;

	double m_filteringTime = 0.005;
	double m_spreadTolerance = 2;

	// Analog input/output signals properties
	//
	double m_electricLowLimit = 0;									// low electric value for input range
	double m_electricHighLimit = 0;									// high electric value for input range
	E::ElectricUnit m_electricUnit = E::ElectricUnit::NoUnit;		// electric unit for input range (mA, mV, Ohm, V ....)
	E::SensorType m_sensorType = E::SensorType::NoSensorType;		// electric sensor type for input range (was created for m_inputUnitID)
	E::OutputMode m_outputMode = E::OutputMode::Plus0_Plus5_V;		// output electric range (or mode ref. OutputModeStr[])

	// Tuning signal properties
	//
	bool m_enableTuning = false;
	float m_tuningDefaultValue = 0;
	float m_tuningLowBound = 0;
	float m_tuningHighBound = 100;

	// Signal properties for MATS
	//
	bool m_acquire = true;
	int m_decimalPlaces = 2;
	double m_coarseAperture = 1;
	double m_fineAperture = 0.5;
	bool m_adaptiveAperture = false;

	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	bool m_checkedOut = false;
	int m_userID = 0;
	QDateTime m_created;
	bool m_deleted = false;
	QDateTime m_instanceCreated;
	VcsItemAction m_instanceAction = VcsItemAction::Added;

	// Signal properties calculated in compile-time
	//
	Hash m_hash = 0;						// == calcHash(m_appSignalID)

	Address16 m_ioBufAddr;					// signal address in i/o modules buffers
											// only for signals of input/output modules (input and output signals)

	Address16 m_tuningAddr;					// signal address in tuning buffer
											// only for tuningable signals

	Address16 m_ualAddr;					// signal address is used in UAL
											// may be equal to m_ioBufAddr, m_tuningAddr, m_regValueAddr or not
											// this address should be used in all signals value read/write operations in UAL

	Address16 m_regBufAddr;					// absolute signal address in registration buffer (LM's memory address)

	Address16 m_regValueAddr;				// signal Value address in FSC data packet
	Address16 m_regValidityAddr;			// signal Validity address in FSC data packet

	//

	bool m_needConversion = false;

	std::shared_ptr<Hardware::DeviceModule> m_lm;		// valid in compile-time only
};


typedef PtrOrderedHash<int, Signal> SignalPtrOrderedHash;


class SignalSet : public SignalPtrOrderedHash
{
public:
	SignalSet();
	virtual ~SignalSet();

	virtual void clear() override;

	void reserve(int n);

	void buildID2IndexMap();
	void clearID2IndexMap() { m_strID2IndexMap.clear(); }
	bool ID2IndexMapIsEmpty();

	bool contains(const QString& appSignalID);
	Signal* getSignal(const QString& appSignalID);

	bool expandBusSignals();
	void initCalculatedSignalsProperties();

	virtual void append(const int& signalID, Signal* signal) override;
	virtual void remove(const int& signalID) override;
	virtual void removeAt(const int index) override;

	QVector<int> getChannelSignalsID(const Signal& signal) const;
	QVector<int> getChannelSignalsID(int signalGroupID) const;

	void resetAddresses();

private:
	QMultiHash<int, int> m_groupSignals;
	QHash<QString, int> m_strID2IndexMap;
};


void SerializeSignalsFromXml(const QString& filePath, SignalSet& signalSet);
