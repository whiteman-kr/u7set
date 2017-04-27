#pragma once

#include <QString>
#include <QMultiHash>
#include "../lib/Types.h"
#include "../lib/DbStruct.h"
#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/Address16.h"
#include "../lib/DataSource.h"
#include "../VFrame30/Afb.h"
#include "../lib/ProtobufHelper.h"
#include "../lib/Hash.h"


class QXmlStreamAttributes;


const char* const InOutTypeStr[] =
{
	"Input",
	"Output",
	"Internal"
};

const int IN_OUT_TYPE_COUNT = sizeof(InOutTypeStr) / sizeof(InOutTypeStr[0]);


const char* const OutputModeStr[] =
{
    "0 .. 5 V",
    "4 .. 20 mA",
    "-10 .. 10 V",
    "0 .. 5 mA",
};

const int OUTPUT_MODE_COUNT = sizeof(OutputModeStr) / sizeof(OutputModeStr[0]);


const int   NO_UNIT_ID = 1;

struct Unit
{
    int ID;
    QString nameEn;
    QString nameRu;
};

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

const int   SENSOR_TYPE_COUNT = sizeof(SensorTypeStr) / sizeof(SensorTypeStr[0]);


struct UnitSensorTypePair
{
    int unitID;
    int sensorType;
};

const UnitSensorTypePair SensorTypeByUnit[] =
{
    // types of thermistors
    //
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1391 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1391 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Pt50_W1385 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Pt100_W1385 },

    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Cu_50_W1428 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Cu_100_W1428 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Cu_50_W1426 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Cu_100_W1426 },

    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Pt21 },
    { E::InputUnit::Ohm, 	E::SensorType::Ohm_Cu23 },

    // types of thermocouple
    //
    { E::InputUnit::mV, 	E::SensorType::mV_K_TXA },
    { E::InputUnit::mV, 	E::SensorType::mV_L_TXK },
    { E::InputUnit::mV, 	E::SensorType::mV_N_THH },
};

const int	SENSOR_TYPE_BY_UNIT_COUNT = sizeof(SensorTypeByUnit) / sizeof(SensorTypeByUnit[0]);


struct DataFormatPair
{
	int ID;
	QString name;
};


typedef OrderedHash<int, QString> UnitList;


class DataFormatList : public OrderedHash<int, QString>
{
public:
	DataFormatList();
};


const QString DATE_TIME_FORMAT_STR("yyyy-MM-ddTHH:mm:ss");

class Signal : public PropertyObject
{
	Q_OBJECT

private:
	void InitProperties();

	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	bool m_checkedOut = false;
	int m_userID = 0;
	E::Channel m_channel = E::Channel::A;
	E::SignalType m_signalType = E::SignalType::Analog;
	QDateTime m_created;
	bool m_deleted = false;
	QDateTime m_instanceCreated;
	VcsItemAction m_instanceAction = VcsItemAction::Added;

	QString m_appSignalID;
	QString m_customAppSignalID;
	QString m_caption;

	// 04.10.2016
	// Changed from E:DataFormat m_dataFormat => E::AppSignalDataFormat m_analogSignalFormat
	// m_analogSignalFormat matters only for analog signals
	// for all discretes assume data format UnsignedInt
	//
	E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;

	int m_dataSize = 32;											// signal data size in bits
	int m_lowADC = 0;
	int m_highADC = 0xFFFF;
    double m_lowEngeneeringUnits = 0;                               // low physical value for input range
    double m_highEngeneeringUnits = 100;                            // high physical value for input range
    int m_unitID = NO_UNIT_ID;                                      // physical unit for input range (kg, mm, Pa ...)
	double m_lowValidRange = 0;
	double m_highValidRange = 100;
	double m_unbalanceLimit = 0;
    double m_inputLowLimit = 0;                                     // low electric value for input range
    double m_inputHighLimit = 0;                                    // high electric value for input range
    E::InputUnit m_inputUnitID = E::InputUnit::NoInputUnit;         // electric unit for input range (mA, mV, Ohm, V ....)
    E::SensorType m_inputSensorType = E::SensorType::NoSensorType;  // electric sensor type for input range (was created for m_inputUnitID)
    double m_outputLowLimit = 0;                                    // low physical value for output range
    double m_outputHighLimit = 0;                                   // high physical value for output range
    int m_outputUnitID = NO_UNIT_ID;                                // physical unit for output range (kg, mm, Pa ...)
    E::OutputMode m_outputMode = E::OutputMode::Plus0_Plus5_V;      // output electric range (or mode ref. OutputModeStr[])
    E::SensorType m_outputSensorType = E::SensorType::NoSensorType; // electric sensor type for output range (was created for m_outputMode)
	bool m_acquire = true;
	bool m_calculated = false;
	int m_normalState = 0;
	int m_decimalPlaces = 2;
	double m_aperture = 1;
	E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;
	QString m_equipmentID;
	double m_filteringTime = 0.005;
	double m_spreadTolerance = 2;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;
	bool m_enableTuning = false;
	double m_tuningDefaultValue = 0;

	Hash m_hash = 0;					// hash of AppSignalID

	Address16 m_ioBufferAddr;			// only for modules input/output signals
										// signal address in i/o modules buffers

	Address16 m_ramAddr;				// signal address in LM RAM
	Address16 m_regValueAddr;			// signal Value address in FSC data packet (registration address)
	Address16 m_regValidityAddr;		// signal Validity address in FSC data packet (registration address)

	Address16 m_tuningAddr;

	std::shared_ptr<Hardware::DeviceModule> m_lm;		// valid in compile-time only

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

public:
	Signal();
	Signal(const Hardware::DeviceSignal& deviceSignal);

	virtual ~Signal();

	Signal& operator = (const Signal& signal);

	static std::shared_ptr<UnitList> unitList;

	int ID() const { return m_ID; }
	int signalGroupID() const { return m_signalGroupID; }
	int signalInstanceID() const { return m_signalInstanceID; }
	int changesetID() const { return m_changesetID; }
	bool checkedOut() const { return m_checkedOut; }
	int userID() const { return m_userID; }

	E::Channel channel() const { return m_channel; }
	int channelInt() const { return TO_INT(m_channel); }

	int signalTypeInt() const { return TO_INT(m_signalType); }
	E::SignalType signalType() const { return m_signalType; }
	void setSignalType(E::SignalType type) { m_signalType = type; }

	bool isAnalog() const { return m_signalType == E::SignalType::Analog; }
	bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }

	bool isInput() const { return m_inOutType == E::SignalInOutType::Input; }
	bool isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
	bool isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }

	bool isRegistered() const { return acquire(); }

	int sizeW() const { return (m_dataSize / 16 + (m_dataSize % 16 ? 1 : 0)); }

	QDateTime created() const { return m_created; }
	bool deleted() const { return m_deleted; }
	QDateTime instanceCreated() const { return m_instanceCreated; }
	VcsItemAction instanceAction() const { return m_instanceAction; }

	Address16& iobufferAddr() { return m_ioBufferAddr; }
	Address16& ramAddr() { return m_ramAddr; }
	Address16& regValueAddr() { return m_regValueAddr; }
	Address16& regValidityAddr() { return m_regValidityAddr; }

	const Address16& iobufferAddr() const { return m_ioBufferAddr; }
	const Address16& ramAddr() const { return m_ramAddr; }

	const Address16& regValueAddr() const { return m_regValueAddr; }
	const Address16& regValidityAddr() const { return m_regValidityAddr; }

	void resetAddresses() { m_ramAddr.reset(); m_regValueAddr.reset(); m_regValidityAddr.reset(); }

	void setRamAddr(const Address16& ramAddr) { m_ramAddr = ramAddr; }
	void setRegValueAddr(const Address16& regValueAddr) { m_regValueAddr = regValueAddr; }
	void setRegValidityAddr(const Address16& regValidityAddr) { m_regValidityAddr = regValidityAddr; }

	void setTuningAddr(const Address16& tuningAddr) { m_tuningAddr = tuningAddr; }
	const Address16& tuningAddr() const { return m_tuningAddr; }

	Hash hash() const { return m_hash; }

	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(bool));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(double));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const QString&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::OutputMode));
    void serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(E::InputUnit));
    void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SensorType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ByteOrder));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const Address16&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, DataFormatList& dataFormatInfo, void (Signal::*setter)(E::AnalogAppSignalFormat));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(int));
	void serializeSensorField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));

	void serializeFields(const QXmlStreamAttributes& attr, DataFormatList& dataFormatInfo, UnitList& unitInfo);

	Q_INVOKABLE QString appSignalID() const { return m_appSignalID; }
	void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

	Q_INVOKABLE QString customAppSignalID() const { return m_customAppSignalID; }
	void setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

	Q_INVOKABLE QString caption() const { return m_caption; }
	void setCaption(const QString& caption) { m_caption = caption; }

	/*Q_INVOKABLE E::DataFormat dataFormat() const { return m_appSignalDataFormat; }
	Q_INVOKABLE int dataFormatInt() const { return TO_INT(m_appSignalDataFormat); }*/

	E::AnalogAppSignalFormat analogSignalFormat() const { return m_analogSignalFormat; }
	int analogSignalFormatInt() const { return TO_INT(m_analogSignalFormat); }

	void setAnalogSignalFormat(E::AnalogAppSignalFormat dataFormat) { m_analogSignalFormat = dataFormat; }

	Q_INVOKABLE int dataSize() const { return m_dataSize; }
	void setDataSize(int dataSize) { m_dataSize = dataSize; }
	void setDataSize(E::SignalType signalType, E::AnalogAppSignalFormat dataFormat);

	Q_INVOKABLE int lowADC() const { return m_lowADC; }
	void setLowADC(int lowADC) { m_lowADC = lowADC; }

	Q_INVOKABLE int highADC() const { return m_highADC; }
	void setHighADC(int highADC) { m_highADC = highADC;}

	Q_INVOKABLE double lowEngeneeringUnits() const { return m_lowEngeneeringUnits; }
	void setLowEngeneeringUnits(double lowEngeneeringUnits) { m_lowEngeneeringUnits = lowEngeneeringUnits; }

	Q_INVOKABLE double highEngeneeringUnits() const { return m_highEngeneeringUnits; }
	void setHighEngeneeringUnits(double highEngeneeringUnits) { m_highEngeneeringUnits = highEngeneeringUnits; }

	Q_INVOKABLE int unitID() const { return m_unitID; }
	void setUnitID(int unitID) { m_unitID = unitID; }

	Q_INVOKABLE double lowValidRange() const { return m_lowValidRange; }
	void setLowValidRange(double lowValidRange) { m_lowValidRange = lowValidRange; }

	Q_INVOKABLE double highValidRange() const { return m_highValidRange; }
	void setHighValidRange(double highValidRange) { m_highValidRange = highValidRange; }

	Q_INVOKABLE double unbalanceLimit() const { return m_unbalanceLimit; }
	void setUnbalanceLimit(double unbalanceLimit) { m_unbalanceLimit = unbalanceLimit; }

	Q_INVOKABLE double inputLowLimit() const { return m_inputLowLimit; }
	void setInputLowLimit(double inputLowLimit) { m_inputLowLimit = inputLowLimit; }

	Q_INVOKABLE double inputHighLimit() const { return m_inputHighLimit; }
	void setInputHighLimit(double inputHighLimit) { m_inputHighLimit = inputHighLimit; }

    int inputUnitIDInt() const { return TO_INT(m_inputUnitID); }
    E::InputUnit inputUnitID() const { return m_inputUnitID; }
    Q_INVOKABLE int jsInputUnitID() const { return static_cast<int>(inputUnitID());}
    void setInputUnitID(E::InputUnit inputUnitID) { m_inputUnitID = inputUnitID; }

    int inputSensorTypeInt() const { return TO_INT(m_inputSensorType); }
    E::SensorType inputSensorType() const { return m_inputSensorType; }
    Q_INVOKABLE int jsInputSensorType() const { return static_cast<int>(inputSensorType());}
    void setInputSensorType(E::SensorType inputSensorType) { m_inputSensorType = inputSensorType; }

	Q_INVOKABLE double outputLowLimit() const { return m_outputLowLimit; }
	void setOutputLowLimit(double outputLowLimit) { m_outputLowLimit = outputLowLimit; }

	Q_INVOKABLE double outputHighLimit() const { return m_outputHighLimit; }
	void setOutputHighLimit(double outputHighLimit) { m_outputHighLimit = outputHighLimit; }

	Q_INVOKABLE int outputUnitID() const { return m_outputUnitID; }
	void setOutputUnitID(int outputUnitID) { m_outputUnitID = outputUnitID; }

	int outputModeInt() const { return TO_INT(m_outputMode); }
	E::OutputMode outputMode() const { return m_outputMode; }
	Q_INVOKABLE int jsOutputMode() const { return static_cast<int>(outputMode());}
	void setOutputMode(E::OutputMode outputMode) { m_outputMode = outputMode; }

    int outputSensorTypeInt() const { return TO_INT(m_outputSensorType); }
    E::SensorType outputSensorType() const { return m_outputSensorType; }
    Q_INVOKABLE int jsOutputSensorType() const { return static_cast<int>(outputSensorType());}
    void setOutputSensorType(E::SensorType outputSensorType) { m_outputSensorType = outputSensorType; }

	Q_INVOKABLE bool acquire() const { return m_acquire; }
	void setAcquire(bool acquire) { m_acquire = acquire; }

	Q_INVOKABLE bool calculated() const { return m_calculated; }
	void setCalculated(bool calculated) { m_calculated = calculated; }

	Q_INVOKABLE int normalState() const { return m_normalState; }
	void setNormalState	(int normalState) { m_normalState = normalState; }

	Q_INVOKABLE int decimalPlaces() const { return m_decimalPlaces; }
	void setDecimalPlaces(int decimalPlaces) { m_decimalPlaces = decimalPlaces; }

	Q_INVOKABLE double aperture() const { return m_aperture; }
	void setAperture(double aperture) { m_aperture = aperture; }

	int inOutTypeInt() const { return TO_INT(m_inOutType); }
	Q_INVOKABLE E::SignalInOutType inOutType() const { return m_inOutType; }
	void setInOutType(E::SignalInOutType inOutType) { m_inOutType = inOutType; }

	Q_INVOKABLE QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	Q_INVOKABLE double filteringTime() const { return m_filteringTime; }
	void setFilteringTime(double filteringTime) { m_filteringTime = filteringTime; }

	Q_INVOKABLE double spreadTolerance() const { return m_spreadTolerance; }
	void setSpreadTolerance(double spreadTolerance) { m_spreadTolerance = spreadTolerance; }

	Q_INVOKABLE E::ByteOrder byteOrder() const { return m_byteOrder; }
	Q_INVOKABLE int byteOrderInt() const { return TO_INT(m_byteOrder); }
	void setByteOrder(E::ByteOrder byteOrder) { m_byteOrder = byteOrder; }

	Q_INVOKABLE bool enableTuning() const { return m_enableTuning; }
	void setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

	Q_INVOKABLE double tuningDefaultValue() const { return m_tuningDefaultValue; }
	void setTuningDefaultValue(double value) { m_tuningDefaultValue = value; }

	void writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);

	void serializeToProtoAppSignal(Proto::AppSignal* s) const;
	void serializeFromProtoAppSignal(const Proto::AppSignal* s);

	void setLm(std::shared_ptr<Hardware::DeviceModule> lm) { m_lm = lm; }
	std::shared_ptr<Hardware::DeviceModule> lm() const { return m_lm; }

	bool isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const;
	bool isCompatibleFormat(const SignalAddress16& sa16) const;

	QString regValueAddrStr() const;


	friend class DbWorker;
};


typedef PtrOrderedHash<int, Signal> SignalPtrOrderedHash;


class SignalSet : public SignalPtrOrderedHash
{
private:
	QMultiHash<int, int> m_groupSignals;
	QHash<QString, int> m_strID2IndexMap;

public:
	SignalSet();
	virtual ~SignalSet();

	virtual void clear() override;

	virtual void append(const int& signalID, Signal* signal);
	virtual void remove(const int& signalID);
	virtual void removeAt(const int index);

	QVector<int> getChannelSignalsID(const Signal& signal);
	QVector<int> getChannelSignalsID(int signalGroupID);

	void buildID2IndexMap();
	void clearID2IndexMap() { m_strID2IndexMap.clear(); }
	bool ID2IndexMapIsEmpty();
	bool contains(const QString& appSignalID);

	Signal* getSignal(const QString& appSignalID);

	void reserve(int n);

	void resetAddresses();
};


void SerializeSignalsFromXml(const QString& filePath, UnitList& unitInfo, SignalSet& signalSet);
