#ifndef SIGNAL_H
#define SIGNAL_H

#include <QString>
#include <QMultiHash>
#include "../include/Types.h"
#include "../include/DbStruct.h"
#include "../include/OrderedHash.h"
#include "../include/DeviceObject.h"


class QXmlStreamAttributes;


Q_DECLARE_METATYPE(SignalType);


enum SignalInOutType
{
	Input = 0,
	Output = 1,
	Internal = 2
};


enum InstanceAction
{
	Added = 1,
	Modified = 2,
	Deleted = 3
};


enum OutputRangeMode
{
	Plus0_Plus5_V = 0,
	Plus4_Plus20_mA = 1,
	Minus10_Plus10_V = 2,
	Plus0_Plus5_mA = 3,
};


const char* const InOutTypeStr[] =
{
	"Input",
	"Output",
	"Internal"
};

const int IN_OUT_TYPE_COUNT = sizeof(InOutTypeStr) / sizeof(InOutTypeStr[0]);


const char* const SensorTypeStr[] =
{
    "Not used",

    "TSP-50P-1.391",
    "TSP-100P-1.391",
    "TSP-50P-1.385",
    "TSP-100P-1.385",
    "TSM-50M-1.428",
	"TSM-100M-1.428",
    "TSM-50M-1.426",
    "TSM-100M-1.426",
    "TSP-21",
    "TSM-23",
    "Rheochord",

    "ТХА(K)",
    "TXK(L) 84",
    "TXK(L) 94",
    "TNN(N)",

    "BPS",
};

const int SENSOR_TYPE_COUNT = sizeof(SensorTypeStr) / sizeof(SensorTypeStr[0]);


const char* const OutputRangeModeStr[] =
{
	"0..5 V",
	"4..20 mA",
	"-10..10 V",
	"0..5 mA",
};

const int OUTPUT_RANGE_MODE_COUNT = sizeof(OutputRangeModeStr) / sizeof(OutputRangeModeStr[0]);


const int NO_UNIT_ID = 1;


struct Unit
{
	int ID;
	QString nameEn;
	QString nameRu;
};


struct DataFormatPair
{
	int ID;
	QString name;
};


// signal address struct
//
// offset	- signal offset in memory in 16-bit words
// bitNo	- discrete signal offset in memory in bits 0..15,
//			  for analog signals always 0
//
//
class Address16
{
private:
	int m_offset = -1;
	int m_bit = -1;

public:
	Address16() {};

	void set(int offset, int bit) { m_offset = offset; m_bit = bit; }
	void setOffset(int offset) { m_offset = offset; }
	void setBit(int bit) { m_bit = bit; }

	int offset() const { return m_offset; }
	int bit() const { return m_bit; }

	void reset() { 	m_offset = -1; m_bit = -1; }

	bool isValid() const { return m_offset != -1 && m_bit != -1; }

	QString toString() const { return QString("%1:%2").arg(m_offset).arg(m_bit); }
	void fromString(QString str);
};


typedef OrderedHash<int, QString> UnitList;


class DataFormatList : public OrderedHash<int, QString>
{
public:
	DataFormatList();
};


const QString DATE_TIME_FORMAT_STR("yyyy-MM-ddTHH:mm:ss");

class Signal : public QObject
{
	Q_OBJECT

private:
	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	bool m_checkedOut = false;
	int m_userID = 0;
	int m_channel = 1;
	SignalType m_type = SignalType::Analog;
	QDateTime m_created;
	bool m_deleted = false;
	QDateTime m_instanceCreated;
	InstanceAction m_instanceAction = InstanceAction::Added;

	QString m_strID;
	QString m_extStrID;
	QString m_name;
	DataFormat m_dataFormat = DataFormat::SignedInt;
	int m_dataSize = 16;
	int m_lowADC = 0;
	int m_highADC = 0xFFFF;
	double m_lowLimit = 0;
	double m_highLimit = 100;
	int m_unitID = NO_UNIT_ID;
	double m_adjustment = 0;
	double m_dropLimit = 0;
	double m_excessLimit = 0;
	double m_unbalanceLimit = 0;
	double m_inputLowLimit = 0;
	double m_inputHighLimit = 0;
	int m_inputUnitID = NO_UNIT_ID;
	int m_inputSensorID = 0;
	double m_outputLowLimit = 0;
	double m_outputHighLimit = 0;
	int m_outputUnitID = NO_UNIT_ID;
	OutputRangeMode m_outputRangeMode = OutputRangeMode::Plus4_Plus20_mA;
	int m_outputSensorID = 0;
	bool m_acquire = true;
	bool m_calculated = false;
	int m_normalState = 0;
	int m_decimalPlaces = 2;
	double m_aperture = 0;
	SignalInOutType m_inOutType = SignalInOutType::Internal;
	QString m_deviceStrID;
	double m_filteringTime = 0.05;
	double m_maxDifference = 0.5;
	ByteOrder m_byteOrder = ByteOrder::BigEndian;

	Address16 m_ramAddr;				// signal address in LM RAM
	Address16 m_regAddr;				// signal address in FSC data packet (registration address)

	// Private setters for fields, witch can't be changed outside DB engine
	// Should be used only by friends
	//
	void setID(int signalID) { m_ID = signalID; }
	void setSignalGroupID(int signalGroupID) { m_signalGroupID = signalGroupID; }
	void setSignalInstanceID(int signalInstanceID) { m_signalInstanceID = signalInstanceID; }
	void setChangesetID(int changesetID) { m_changesetID = changesetID; }
	void setCheckedOut(int checkedOut) { m_checkedOut = checkedOut; }
	void setUserID(int userID) { m_userID = userID; }
	void setChannel(int channel) { m_channel = channel; }
	void setCreated(const QDateTime& created) { m_created = created; }
	void setCreated(const QString& createdStr) { m_created = QDateTime::fromString(createdStr, DATE_TIME_FORMAT_STR); }
	void setDeleted(bool deleted) { m_deleted = deleted; }
	void setInstanceCreated(const QDateTime& instanceCreated) { m_instanceCreated = instanceCreated; }
	void setInstanceCreated(const QString& instanceCreatedStr) { m_instanceCreated = QDateTime::fromString(instanceCreatedStr, DATE_TIME_FORMAT_STR); }
	void setInstanceAction(InstanceAction action) { m_instanceAction = action; }

public:
	Signal();

	Signal(const Signal& signal) :
		QObject()
	{
		*this = signal;
	}

	Signal(const Hardware::DeviceSignal& deviceSignal);

	Signal& operator = (const Signal& signal);

	int ID() const { return m_ID; }
	int signalGroupID() const { return m_signalGroupID; }
	int signalInstanceID() const { return m_signalInstanceID; }
	int changesetID() const { return m_changesetID; }
	bool checkedOut() const { return m_checkedOut; }
	int userID() const { return m_userID; }
	int channel() const { return m_channel; }

	int typeInt() const { return TO_INT(m_type); }
	SignalType type() const { return m_type; }
	void setType(SignalType type) { m_type = type; }

	bool isAnalog() const { return m_type == SignalType::Analog; }
	bool isDiscrete() const { return m_type == SignalType::Discrete; }

	bool isInput() const { return m_inOutType == SignalInOutType::Input; }
	bool isOutput() const { return m_inOutType == SignalInOutType::Output; }
	bool isInternal() const { return m_inOutType == SignalInOutType::Internal; }

	bool isRegistered() const { return acquire(); }

	QDateTime created() const { return m_created; }
	bool deleted() const { return m_deleted; }
	QDateTime instanceCreated() const { return m_instanceCreated; }
	InstanceAction instanceAction() const { return m_instanceAction; }

	Address16& ramAddr() { return m_ramAddr; }
	Address16& regAddr() { return m_regAddr; }

	const Address16& ramAddr() const { return m_ramAddr; }
	const Address16& regAddr() const { return m_regAddr; }

	void resetAddresses() { m_ramAddr.reset(); m_regAddr.reset(); }

	void setRamAddr(const Address16& ramAddr) { m_ramAddr = ramAddr; }
	void setRegAddr(const Address16& regAddr) { m_regAddr = regAddr; }

	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(bool));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(double));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const QString&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(SignalType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(OutputRangeMode));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(SignalInOutType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(ByteOrder));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const Address16&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, DataFormatList& dataFormatInfo, void (Signal::*setter)(DataFormat));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(int));
	void serializeSensorField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));

	void serializeFields(const QXmlStreamAttributes& attr, DataFormatList& dataFormatInfo, UnitList& unitInfo);

    Q_INVOKABLE QString strID() const { return m_strID; }
	void setStrID(const QString& strID) { m_strID = strID; }

    Q_INVOKABLE QString extStrID() const { return m_extStrID; }
	void setExtStrID(const QString& extStrID) { m_extStrID = extStrID; }

    Q_INVOKABLE QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	Q_INVOKABLE DataFormat dataFormat() const { return m_dataFormat; }
	Q_INVOKABLE int dataFormatInt() const { return TO_INT(m_dataFormat); }
	void setDataFormat(DataFormat dataFormat) { m_dataFormat = dataFormat; }

    Q_INVOKABLE int dataSize() const { return m_dataSize; }
	void setDataSize(int dataSize) { m_dataSize = dataSize; }

    Q_INVOKABLE int lowADC() const { return m_lowADC; }
	void setLowADC(int lowADC) { m_lowADC = lowADC; }

    Q_INVOKABLE int highADC() const { return m_highADC; }
	void setHighADC(int highADC) { m_highADC = highADC;}

    Q_INVOKABLE double lowLimit() const { return m_lowLimit; }
	void setLowLimit(double lowLimit) { m_lowLimit = lowLimit; }

    Q_INVOKABLE double highLimit() const { return m_highLimit; }
	void setHighLimit(double highLimit) { m_highLimit = highLimit; }

    Q_INVOKABLE int unitID() const { return m_unitID; }
	void setUnitID(int unitID) { m_unitID = unitID; }

    Q_INVOKABLE double adjustment() const { return m_adjustment; }
	void setAdjustment(double adjustment) { m_adjustment = adjustment; }

    Q_INVOKABLE double dropLimit() const { return m_dropLimit; }
	void setDropLimit(double dropLimit) { m_dropLimit = dropLimit; }

    Q_INVOKABLE double excessLimit() const { return m_excessLimit; }
	void setExcessLimit(double excessLimit) { m_excessLimit = excessLimit; }

    Q_INVOKABLE double unbalanceLimit() const { return m_unbalanceLimit; }
	void setUnbalanceLimit(double unbalanceLimit) { m_unbalanceLimit = unbalanceLimit; }

    Q_INVOKABLE double inputLowLimit() const { return m_inputLowLimit; }
	void setInputLowLimit(double inputLowLimit) { m_inputLowLimit = inputLowLimit; }

    Q_INVOKABLE double inputHighLimit() const { return m_inputHighLimit; }
	void setInputHighLimit(double inputHighLimit) { m_inputHighLimit = inputHighLimit; }

    Q_INVOKABLE int inputUnitID() const { return m_inputUnitID; }
	void setInputUnitID(int inputUnitID) { m_inputUnitID = inputUnitID; }

    Q_INVOKABLE int inputSensorID() const { return m_inputSensorID; }
	void setInputSensorID(int inputSensorID) { m_inputSensorID = inputSensorID; }

    Q_INVOKABLE double outputLowLimit() const { return m_outputLowLimit; }
	void setOutputLowLimit(double outputLowLimit) { m_outputLowLimit = outputLowLimit; }

    Q_INVOKABLE double outputHighLimit() const { return m_outputHighLimit; }
	void setOutputHighLimit(double outputHighLimit) { m_outputHighLimit = outputHighLimit; }

    Q_INVOKABLE int outputUnitID() const { return m_outputUnitID; }
	void setOutputUnitID(int outputUnitID) { m_outputUnitID = outputUnitID; }

	OutputRangeMode outputRangeMode() const { return m_outputRangeMode; }
	Q_INVOKABLE int jsOutputRangeMode() const { return static_cast<int>(outputRangeMode());}
	void setOutputRangeMode(OutputRangeMode outputRangeMode) { m_outputRangeMode = outputRangeMode; }

    Q_INVOKABLE int outputSensorID() const { return m_outputSensorID; }
	void setOutputSensorID(int outputSensorID) { m_outputSensorID = outputSensorID; }

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

    Q_INVOKABLE SignalInOutType inOutType() const { return m_inOutType; }
	void setInOutType(SignalInOutType inOutType) { m_inOutType = inOutType; }

    Q_INVOKABLE QString deviceStrID() const { return m_deviceStrID; }
	void setDeviceStrID(const QString& deviceStrID) { m_deviceStrID = deviceStrID; }

	Q_INVOKABLE double filteringTime() const { return m_filteringTime; }
	void setFilteringTime(double filteringTime) { m_filteringTime = filteringTime; }

	Q_INVOKABLE double maxDifference() const { return m_maxDifference; }
	void setMaxDifference(double maxDifference) { m_maxDifference = maxDifference; }

	Q_INVOKABLE ByteOrder byteOrder() const { return m_byteOrder; }
	Q_INVOKABLE int byteOrderInt() const { return TO_INT(m_byteOrder); }
	void setByteOrder(ByteOrder byteOrder) { m_byteOrder = byteOrder; }


	friend class DbWorker;
};


typedef PtrOrderedHash<int, Signal> SignalPtrOrderedHash;


class SignalSet : public SignalPtrOrderedHash
{
private:
	QMultiHash<int, int> m_groupSignals;

public:
	SignalSet();
	virtual ~SignalSet();

	virtual void clear() override;

	virtual void append(const int& signalID, Signal* signal);
	virtual void remove(const int& signalID);
	virtual void removeAt(const int index);

	QVector<int> getChannelSignalsID(const Signal& signal);
	QVector<int> getChannelSignalsID(int signalGroupID);

	void reserve(int n);

	void resetAddresses();
};




#endif // SIGNAL_H
