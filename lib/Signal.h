#pragma once

#include <QString>
#include <QMultiHash>

#include "Types.h"
#include "DbStruct.h"
#include "OrderedHash.h"
#include "DeviceObject.h"
#include "Address16.h"
#include "../VFrame30/Afb.h"
#include "Hash.h"
#include "TuningValue.h"

class QXmlStreamAttributes;
class XmlWriteHelper;
class XmlReadHelper;

const QString DATE_TIME_FORMAT_STR("yyyy-MM-ddTHH:mm:ss");

class SignalSpecPropValues;


class Signal
{
	friend class DbWorker;
	friend class SignalSet;
	friend class SignalTests;

public:
	static QString BUS_SIGNAL_ID_SEPARATOR;

	static QString BUS_SIGNAL_MACRO_BUSTYPEID;
	static QString BUS_SIGNAL_MACRO_BUSID;
	static QString BUS_SIGNAL_MACRO_BUSSIGNALID;
	static QString BUS_SIGNAL_MACRO_BUSSIGNALCAPTION;

public:
	Signal();
	Signal(const Signal& s);
	Signal(const Hardware::DeviceSignal& deviceSignal);
	virtual ~Signal();

	void initSpecificProperties();

	// Signal identificators

	QString appSignalID() const { return m_appSignalID; }
	void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

	QString customAppSignalID() const { return m_customAppSignalID; }
	void setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

	QString caption() const { return m_caption; }
	void setCaption(const QString& caption) { m_caption = caption; }

	QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString lmEquipmentID() const { return m_lmEquipmentID; }
	void setLmEquipmentID(const QString& lmEquipmentID) { m_lmEquipmentID = lmEquipmentID; }

	QString busTypeID() const { return m_busTypeID; }
	void setBusTypeID(const QString& busTypeID) { m_busTypeID = busTypeID; }

	E::Channel channel() const { return m_channel; }
	int channelInt() const { return TO_INT(m_channel); }

	// Signal type

	E::SignalType signalType() const { return m_signalType; }
	int signalTypeInt() const { return TO_INT(m_signalType); }
	void setSignalType(E::SignalType type);

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
	void setDataSizeW(int sizeW);

	int sizeW() const { return (m_dataSize / SIZE_16BIT + (m_dataSize % SIZE_16BIT ? 1 : 0)); }

	E::ByteOrder byteOrder() const { return m_byteOrder; }
	int byteOrderInt() const { return TO_INT(m_byteOrder); }
	void setByteOrder(E::ByteOrder byteOrder) { m_byteOrder = byteOrder; }

	E::AnalogAppSignalFormat analogSignalFormat() const { return m_analogSignalFormat; }
	int analogSignalFormatInt() const { return TO_INT(m_analogSignalFormat); }
	void setAnalogSignalFormat(E::AnalogAppSignalFormat dataFormat);

	E::DataFormat dataFormat() const;

	bool isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const;
	bool isCompatibleFormat(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, E::ByteOrder byteOrder) const;
	bool isCompatibleFormat(const SignalAddress16& sa16) const;
	bool isCompatibleFormat(const Signal& s) const;
	bool isCompatibleFormat(E::SignalType signalType, const QString& busTypeID) const;

	// Analog signal properties

	int lowADC() const;
	void setLowADC(int lowADC);

	int highADC() const;
	void setHighADC(int highADC);

	int lowDAC() const;
	void setLowDAC(int lowDAC);

	int highDAC() const;
	void setHighDAC(int highDAC);

	double lowEngeneeringUnits() const;
	void setLowEngeneeringUnits(double lowEngeneeringUnits);

	double highEngeneeringUnits() const;
	void setHighEngeneeringUnits(double highEngeneeringUnits);

	double lowValidRange() const;
	void setLowValidRange(double lowValidRange);

	double highValidRange() const;
	void setHighValidRange(double highValidRange);

	double filteringTime() const;
	void setFilteringTime(double filteringTime);

	double spreadTolerance() const;
	void setSpreadTolerance(double spreadTolerance);

	// Analog input/output signal properties

	double electricLowLimit() const;
	void setElectricLowLimit(double electricLowLimit);

	double electricHighLimit() const;
	void setElectricHighLimit(double electricHighLimit);

	E::ElectricUnit electricUnit() const;
	void setElectricUnit(E::ElectricUnit electricUnit);

	E::SensorType sensorType() const;
	void setSensorType(E::SensorType sensorType);

	E::OutputMode outputMode() const;
	void setOutputMode(E::OutputMode outputMode);

	// Tuning signal properties

	bool enableTuning() const { return m_enableTuning; }
	void setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

	TuningValue tuningDefaultValue() const { return m_tuningDefaultValue; }
	void setTuningDefaultValue(const TuningValue& value) { m_tuningDefaultValue = value; }

	TuningValue tuningLowBound() const { return m_tuningLowBound; }
	void setTuningLowBound(const TuningValue& value) { m_tuningLowBound = value; }

	TuningValue tuningHighBound() const { return m_tuningHighBound; }
	void setTuningHighBound(const TuningValue& value) { m_tuningHighBound = value; }

	// Signal properties for MATS

	bool acquire() const { return m_acquire; }
	void setAcquire(bool acquire) { m_acquire = acquire; }

	bool isAcquired() const { return m_acquire; }

	bool archive() const { return m_archive; }
	void setArchive(bool archive) { m_archive = archive; }

	bool isArchived() const { return m_archive; }

	int decimalPlaces() const { return m_decimalPlaces; }
	void setDecimalPlaces(int decimalPlaces) { m_decimalPlaces = decimalPlaces; }

	double coarseAperture() const { return m_coarseAperture; }
	void setCoarseAperture(double aperture) { m_coarseAperture = aperture; }

	double fineAperture() const { return m_fineAperture; }
	void setFineAperture(double aperture) { m_fineAperture = aperture; }

	bool adaptiveAperture() const { return m_adaptiveAperture; }
	void setAdaptiveAperture(bool adaptive) { m_adaptiveAperture = adaptive; }

	// Specific properties

	QString specPropStruct() const { return m_specPropStruct; }
	void setSpecPropStruct(const QString& specPropsStruct) { m_specPropStruct = specPropsStruct; }

	bool createSpecPropValues();

	void setProtoSpecPropValues(const QByteArray& protoSpecPropValues) { m_protoSpecPropValues = protoSpecPropValues; }
	const QByteArray& protoSpecPropValues() const { return m_protoSpecPropValues; }

	void cacheSpecPropValues();

	double getSpecPropDouble(const QString& name) const;
	int getSpecPropInt(const QString& name) const;
	unsigned int getSpecPropUInt(const QString& name) const;
	int getSpecPropEnum(const QString& name) const;
	bool getSpecPropValue(const QString& name, QVariant* qv, bool* isEnum) const;

	bool setSpecPropDouble(const QString& name, double value);
	bool setSpecPropInt(const QString& name, int value);
	bool setSpecPropUInt(const QString& name, unsigned int value);
	bool setSpecPropEnum(const QString& name, int enumValue);
	bool setSpecPropValue(const QString& name, const QVariant& qv, bool isEnum);

	//

	void saveProtoData(QByteArray* protoDataArray) const;
	void saveProtoData(Proto::ProtoAppSignalData* protoData) const;

	void loadProtoData(const QByteArray& protoDataArray);
	void loadProtoData(const Proto::ProtoAppSignalData& protoData);

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

	E::LogicModuleRamAccess lmRamAccess() const { return m_lmRamAccess; }
	void setLmRamAccess(E::LogicModuleRamAccess access) { m_lmRamAccess = access; }

	QString regValueAddrStr() const;

	bool needConversion() const { return m_needConversion; }
	void setNeedConversion(bool need) { m_needConversion = need; }

	std::shared_ptr<Hardware::DeviceModule> lm() const { return m_lm; }
	void setLm(std::shared_ptr<Hardware::DeviceModule> lm);

	bool isConst() const { return m_isConst; }
	void setIsConst(bool isConst) { m_isConst = isConst; }

	double constValue() const { return m_constValue; }
	void setConstValue(double constValue) { m_constValue = constValue; }

	//

	void writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);

	void serializeTo(Proto::AppSignal* s) const;
	void serializeFrom(const Proto::AppSignal &s);

	void initCalculatedProperties();

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

	bool isCompatibleFormatPrivate(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder, const QString& busTypeID) const;

	void updateTuningValuesType();

private:
	// Signal identificators
	//
	QString m_appSignalID;
	QString m_customAppSignalID;
	QString m_caption;
	QString m_equipmentID;											// should be transformed to portEquipmentID
	QString m_lmEquipmentID;										// now fills in compile time only
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

	// Tuning signal properties
	//
	bool m_enableTuning = false;
	TuningValue m_tuningDefaultValue;
	TuningValue m_tuningLowBound;
	TuningValue m_tuningHighBound;

	// Signal properties for MATS
	//
	bool m_acquire = true;
	bool m_archive = true;
	int m_decimalPlaces = 2;
	double m_coarseAperture = 1;
	double m_fineAperture = 0.5;
	bool m_adaptiveAperture = false;

	// Signal specific properties
	//

	QString m_specPropStruct;
	QByteArray m_protoSpecPropValues;					// serialized protobuf message Proto::PropertyValues

	SignalSpecPropValues* m_cachedSpecPropValues = nullptr;

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

	Address16 m_ioBufAddr;					// signal address in i/o modules buffers for signals of input/output modules (input and output signals)
											// or

	Address16 m_tuningAddr;					// signal address in tuning buffer
											// only for tuningable signals

	Address16 m_ualAddr;					// signal address is used in UAL
											// may be equal to m_ioBufAddr, m_tuningAddr, m_regValueAddr or not
											// this address should be used in all signals value read/write operations in UAL

	Address16 m_regBufAddr;					// absolute signal address in registration buffer (LM's memory address)

	Address16 m_regValueAddr;				// signal Value address in FSC data packet
	Address16 m_regValidityAddr;			// signal Validity address in FSC data packet

	E::LogicModuleRamAccess m_lmRamAccess = E::LogicModuleRamAccess::Undefined;

	bool m_isConst = false;
	double m_constValue = 0;

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

	bool contains(const QString& appSignalID) const;
	Signal* getSignal(const QString& appSignalID);

	virtual void append(const int& signalID, Signal* signal) override;
	virtual void remove(const int& signalID) override;
	virtual void removeAt(const int index) override;

	QVector<int> getChannelSignalsID(const Signal& signal) const;
	QVector<int> getChannelSignalsID(int signalGroupID) const;

	void resetAddresses();

	bool serializeFromProtoFile(const QString& filePath);

private:
	QMultiHash<int, int> m_groupSignals;
	QHash<QString, int> m_strID2IndexMap;
};
