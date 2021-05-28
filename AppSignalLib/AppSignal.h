#pragma once

#include <QString>
#include <QMultiHash>
#include <QFile>
#include <utility>
#include <set>

#include "../UtilsLib/Address16.h"
#include "../CommonLib/Hash.h"
#include "../UtilsLib/WUtils.h"
#include "../CommonLib/OrderedHash.h"
#include "../lib/ConstStrings.h"
#include "../CommonLib/Types.h"
#include "../CommonLib/PropertyObject.h"

#include "TuningValue.h"

class XmlWriteHelper;
class XmlReadHelper;

struct ID_AppSignalID
{
	int ID = -1;
	QString appSignalID;
};

Q_DECLARE_METATYPE(ID_AppSignalID)

#pragma pack(push, 1)

union AppSignalStateFlags
{
	struct
	{
		// signal state flags
		//
		quint32	valid : 1;					//	0	this flag is set according to validity signal of signal
											//		if validity signal isn't exists - validity flag is equal to stateAvailable flag

		quint32 stateAvailable : 1;			//	1	sets to 1 if application data received from LM
											//		if no data available from LM this flag sets to 0

		quint32 simulated : 1;				//	2	sets according to simulation signal of signal (see AFB sim_lock)
		quint32 blocked : 1;				//	3	sets according to blocking signal of signal (see AFB sim_lock)
		quint32 mismatch : 1;				//	4	sets according to mismatch signal of signal (see AFB mismatch)

		quint32 aboveHighLimit: 1;			//	5	sets to 1 if value of signal is greate than HighEngineeringUnits limit
		quint32 belowLowLimit: 1;			//	6	sets to 1 if value of signal is less than LowEngineeringUnits limit

		quint32 swSimulated : 1;			//  7	state of signal is acquire from software simulated packet

		// bits reserved for state flags
		//
		quint32 _bit8: 1;					//	8
		quint32 _bit9: 1;					//	9
		quint32 _bit10: 1;					//	10
		quint32 _bit11: 1;					//	11
		quint32 _bit12: 1;					//	12
		quint32 _bit13: 1;					//	13
		quint32 _bit14: 1;					//	14
		quint32 _bit15: 1;					//	15

		// archiving reasons flags
		//
		quint32 validityChange : 1;			//	16	any changes of valid or stateAvailable flags
		quint32 simBlockMismatchChange : 1;	//	17	any changes of simulated, blocked, mismatch flags
		quint32 limitFlagsChange : 1;		//	18	any changes of aboveHighLimit or belowLowLimit flags
		quint32 autoPoint : 1;				//	19
		quint32 fineAperture : 1;			//	20
		quint32 coarseAperture : 1;			//	21

		// bits reserved for archiving reasons flags
		//
		quint32 _bit22 : 1;					//	22
		quint32 _bit23 : 1;					//	23
		quint32 _bit24 : 1;					//	24
		quint32 _bit25 : 1;					//	25
		quint32 _bit26 : 1;					//	26
		quint32 _bit27 : 1;					//	27
		quint32 _bit28 : 1;					//	28
		quint32 _bit29 : 1;					//	29
		quint32 _bit30 : 1;					//	30

		quint32 realtimePoint: 1;			//	31	special flag for real time trends displaying
	};

	quint32 all = 0;

	void setFlag(E::AppSignalStateFlagType flagType, quint32 value);

	void clear();
	void clearReasonsFlags();

	bool hasArchivingReason() const;
	bool hasShortTermArchivingReasonOnly() const;
	bool hasAutoPointReasonOnly() const;

	void updateArchivingReasonFlags(const AppSignalStateFlags& prevFlags);

	QString print();

	static const quint32 MASK_VALIDITY_AND_AVAILABLE_FLAGS = 0x00000003;
	static const quint32 MASK_SIM_BLOCK_UNBL_FLAGS = 0x0000001C;
	static const quint32 MASK_LIMITS_FLAGS = 0x00000060;
	static const quint32 MASK_ALL_ARCHIVING_REASONS = 0x003F0000;
	static const quint32 MASK_SHORT_TERM_ARCHIVING_REASONE = 0x00100000;		// for now this is fineAperture flag only
	static const quint32 MASK_AUTO_POINT_REASONE = 0x00080000;
};

#pragma pack(pop)

typedef QHash<E::AppSignalStateFlagType, QString> AppSignalStateFlagsMap;

class AppSignalSpecPropValue
{
public:
	AppSignalSpecPropValue();

	bool create(std::shared_ptr<Property> prop);
	bool create(const QString& name, const QVariant& value, bool isEnum);

	bool setValue(const QString& name, const QVariant& value, bool isEnum);
	bool setAnyValue(const QString& name, const QVariant& value);

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	QVariant::Type type() const { return m_value.type(); }
	QVariant value() const { return m_value; }
	bool isEnum() const {return m_isEnum; }

	bool save(Proto::SignalSpecPropValue* protoValue) const;
	bool load(const Proto::SignalSpecPropValue& protoValue);

private:
	QString m_name;
	QVariant m_value;
	bool m_isEnum = false;
};

class AppSignal;

class AppSignalSpecPropValues
{
public:
	AppSignalSpecPropValues();

	bool create(const AppSignal& s);

	bool createFromSpecPropStruct(const QString& specPropStruct, bool buildNamesMap = true);
	bool updateFromSpecPropStruct(const QString& specPropStruct);

	bool isExists(const QString& name) const { return m_propNamesMap.contains(name); }

	bool setValue(const QString& name, const QVariant& value);

	bool setAnyValue(const QString& name, const QVariant& value);		// setter without isEnum checking

	template<typename ENUM_TYPE>
	bool setEnumValue(const QString& name, ENUM_TYPE enumItemValue);
	bool setEnumValue(const QString& name, int enumItemValue);

	bool setValue(const AppSignalSpecPropValue& propValue);

	bool getValue(const QString& name, QVariant* qv) const;
	bool getValue(const QString& name, QVariant* qv, bool* isEnum) const;

	bool serializeValuesToArray(QByteArray* protoData) const;
	bool parseValuesFromArray(const QByteArray& protoData);

	//bool save(Proto::SignalSpecPropValues* protoValues) const;

	const QVector<AppSignalSpecPropValue>& values() const { return m_specPropValues; }

	void append(const AppSignalSpecPropValue& value);

	bool replaceName(const QString& oldName, const QString& newName);			// returns true if replacing is occured

private:
	void buildPropNamesMap();

	bool setValue(const QString& name, const QVariant& value, bool isEnum);

	int getPropertyIndex(const QString& name) const;

private:
	QVector<AppSignalSpecPropValue> m_specPropValues;
	QHash<QString, int> m_propNamesMap;									// prop name => index in m_propSpecValues
};

template<typename ENUM_TYPE>
bool AppSignalSpecPropValues::setEnumValue(const QString& name, ENUM_TYPE enumItemValue)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);
	return setValue(name, static_cast<int>(enumItemValue), true);
}

class AppSignal
{
	friend class DbWorker;
	friend class AppSignalSet;
	friend class SignalTests;
	friend class DbControllerSignalTests;

public:
	static const QString CAPTION_VALIDATOR;
	static const QString IDENTIFICATORS_VALIDATOR;

public:
	AppSignal();
	AppSignal(const AppSignal& s);
	AppSignal(const ID_AppSignalID& ids);
	virtual ~AppSignal();

	QString initFromDeviceSignal(const QString& deviceSignalEquipmentID,
								E::SignalType deviceSignalType,
								E::SignalFunction deviceSignalFunction,
								const QString& appSignalID,
								const QString& customAppSignalID,
								const QString& appSignalCaption,
								const QString& appSignalBusTypeID,
								E::AnalogAppSignalFormat analogAppSignalFormat,
								const QString& appSignalSpecPropsStruct,
								bool enableTuning,
								const QVariant& tuningLowBound,
								const QVariant& tuningHighBound,
								const QVariant& tuningDefaultValue);

	void clear();

	void initSpecificProperties();

	bool isLoaded() const { return m_isLoaded; }
	void setIsLoaded(bool isLoaded) { m_isLoaded = isLoaded; }

	// Signal identificators

	const QString& appSignalID() const { return m_appSignalID; }
	void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

	QString customAppSignalID() const { return m_customAppSignalID; }
	void setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }
	bool customAppSignalIDContainsMacro() const { return m_customAppSignalID.contains(TemplateMacro::START_TOKEN); }

	QString caption() const { return m_caption; }
	void setCaption(const QString& caption) { m_caption = caption; }
	bool captionContainsMacro() const { return m_caption.contains(TemplateMacro::START_TOKEN); }

	QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString lmEquipmentID() const { return m_lmEquipmentID; }
	void setLmEquipmentID(const QString& lmEquipmentID) { m_lmEquipmentID = lmEquipmentID; }

	QString busTypeID() const { return m_busTypeID; }
	void setBusTypeID(const QString& busTypeID) { m_busTypeID = busTypeID; }

	E::Channel channel() const { return m_channel; }
	int channelInt() const { return TO_INT(m_channel); }

	bool excludeFromBuild() const { return m_excludeFromBuild; }
	void setExcludeFromBuild(bool excludeFromBuild) { m_excludeFromBuild = excludeFromBuild; }

	bool isAutoSignal() const { return m_isAutoSignal; }
	void setAutoSignal(bool autoSignal) { m_isAutoSignal = autoSignal; }

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

	int sizeW() const { return (m_dataSize / SIZE_16BIT + ((m_dataSize % SIZE_16BIT) ? 1 : 0)); }
	int sizeBit() const { return m_dataSize; }

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
	bool isCompatibleFormat(const AppSignal& s) const;
	bool isCompatibleFormat(E::SignalType signalType, const QString& busTypeID) const;

	// Analog signal properties

	int lowADC(QString* err = nullptr) const;
	void setLowADC(int lowADC);

	int highADC(QString* err = nullptr) const;
	void setHighADC(int highADC);

	int lowDAC(QString* err = nullptr) const;
	void setLowDAC(int lowDAC);

	int highDAC(QString* err = nullptr) const;
	void setHighDAC(int highDAC);

	double lowEngineeringUnits(QString* err = nullptr) const;
	void setLowEngineeringUnits(double lowEngineeringUnits);

	double highEngineeringUnits(QString* err = nullptr) const;
	void setHighEngineeringUnits(double highEngineeringUnits);

	double lowValidRange(QString* err = nullptr) const;
	void setLowValidRange(double lowValidRange);

	double highValidRange(QString* err = nullptr) const;
	void setHighValidRange(double highValidRange);

	double filteringTime(QString* err = nullptr) const;
	void setFilteringTime(double filteringTime);

	double spreadTolerance(QString* err = nullptr) const;
	void setSpreadTolerance(double spreadTolerance);

	// Analog input/output signal properties

	double electricLowLimit(QString* err = nullptr) const;
	void setElectricLowLimit(double electricLowLimit);

	double electricHighLimit(QString* err = nullptr) const;
	void setElectricHighLimit(double electricHighLimit);

	E::ElectricUnit electricUnit(QString* err = nullptr) const;
	void setElectricUnit(E::ElectricUnit electricUnit);

	double rload_Ohm(QString* err = nullptr) const;
	void setRload_Ohm(double rload_Ohm);

	E::SensorType sensorType(QString* err = nullptr) const;
	void setSensorType(E::SensorType sensorType);

	E::OutputMode outputMode(QString* err = nullptr) const;
	void setOutputMode(E::OutputMode outputMode);

	double r0_Ohm(QString* err = nullptr) const;
	void setR0_Ohm(double r0_Ohm);

	// Tuning signal properties

	bool enableTuning() const { return m_enableTuning; }
	bool isTunable() const { return m_enableTuning; }
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

	double getSpecPropDouble(const QString& name, QString* err) const;
	int getSpecPropInt(const QString& name, QString* err) const;
	unsigned int getSpecPropUInt(const QString& name, QString* err) const;
	int getSpecPropEnum(const QString& name, QString* err) const;
	bool getSpecPropValue(const QString& name, QVariant* qv, bool* isEnum, QString* err) const;
	bool isSpecPropExists(const QString& name) const;

	bool setSpecPropDouble(const QString& name, double value);
	bool setSpecPropInt(const QString& name, int value);
	bool setSpecPropUInt(const QString& name, unsigned int value);
	bool setSpecPropEnum(const QString& name, int enumValue);
	bool setSpecPropValue(const QString& name, const QVariant& qv, bool isEnum);

	//

	QStringList tags() const;
	const std::set<QString>& tagsSet() const { return m_tags; }
	std::set<QString>& tagsSet() { return m_tags; }
	QString tagsStr() const { return tags().join(QChar::LineFeed); }

	void setTags(const QStringList& tags);
	void setTags(const std::set<QString>& tags) { m_tags = tags; }
	void setTagsStr(const QString& tagsStr) { setTags(tagsStr.split(QRegExp("\\W+"), Qt::SkipEmptyParts)); }

	bool hasTags() const { return m_tags.size() > 0; }
	bool hasTag(const QString& tag) const { return m_tags.find(tag.toLower().trimmed()) != m_tags.end(); }
	int tagsCount() const { return static_cast<int>(m_tags.size()); }

	void appendTag(const QString& tag);
	void appendTags(const QStringList& tags);
	void appendTags(const std::set<QString>& tags);

	void removeTag(const QString& tag);
	void removeTags(const QStringList& tags);
	void removeTags(const std::set<QString>& tags);

	void clearTags();

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
	E::VcsItemAction instanceAction() const { return m_instanceAction; }

	// Signal properties calculated in compile-time

	Hash hash() const { assert(m_hash !=0); return m_hash; }
	void setHash(Hash hash) { m_hash = hash; }

	QString unit() const { return m_unit; }
	void setUnit(const QString& unit) { m_unit = unit; }

	Address16 ioBufAddr() const;
	void setIoBufAddr(const Address16& addr);

	Address16 tuningAddr() const { return m_tuningAddr; }
	void setTuningAddr(const Address16& addr) { m_tuningAddr = addr; }

	Address16 tuningAbsAddr() const { return m_tuningAbsAddr; }
	void setTuningAbsAddr(const Address16& addr) { m_tuningAbsAddr = addr; }

	Address16 ualAddr() const { return m_ualAddr; }
	void setUalAddr(const Address16& addr) { m_ualAddr = addr; }
	bool ualAddrIsValid() const { return m_ualAddr.isValid(); }

	Address16 regBufAddr() const { return m_regBufAddr; }
	void setRegBufAddr(const Address16& addr) { m_regBufAddr = addr; }

	Address16 regValueAddr() const { return m_regValueAddr; }
	void setRegValueAddr(const Address16& addr) { m_regValueAddr = addr; }

	Address16 regValidityAddr() const { return m_regValidityAddr; }
	void setRegValidityAddr(const Address16& addr) { m_regValidityAddr = addr; }

	Address16 actualAddr(E::LogicModuleRamAccess* lmRamAccess = nullptr) const;

	void resetAddresses();

	E::LogicModuleRamAccess lmRamAccess() const { return m_lmRamAccess; }
	void setLmRamAccess(E::LogicModuleRamAccess access) { m_lmRamAccess = access; }

	QString regValueAddrStr() const;

	bool needConversion() const { return m_needConversion; }
	void setNeedConversion(bool need) { m_needConversion = need; }

	bool isConst() const { return m_isConst; }
	void setIsConst(bool isConst) { m_isConst = isConst; }

	double constValue() const { return m_constValue; }
	void setConstValue(double constValue) { m_constValue = constValue; }

	bool isEndpoint() const { return m_isEndpoint; }
	void setEndpoint(bool ep) { m_isEndpoint = ep; }

	//

	void writeToXml(XmlWriteHelper& xml);
	void writeDoubleSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName = QString());
	void writeIntSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName = QString());
	void writeTuningValuesToXml(XmlWriteHelper& xml);

	bool readFromXml(XmlReadHelper& xml);
	bool readTuningValuesFromXml(XmlReadHelper& xml);

	void serializeTo(Proto::AppSignal* s) const;
	void serializeFrom(const Proto::AppSignal &s);

	void initCalculatedProperties();

	bool addFlagSignalID(E::AppSignalStateFlagType flagType, const QString& appSignalID);
	QString getFlagSignalID(E::AppSignalStateFlagType flagType) const { return  m_stateFlagsSignals.value(flagType, QString()); }
	QStringList getFlagSignalsIDs() const { return m_stateFlagsSignals.values(); }
	bool hasFlagsSignals() const { return m_stateFlagsSignals.count(); }

	const AppSignalStateFlagsMap& stateFlagsSignals() const { return m_stateFlagsSignals; }

	void initTuningValues();

	static AppSignal* createDiscreteSignal(E::SignalInOutType inOutType,
										  const QString& appSignalID,
										  const QString& customAppSignalID,
										  const QString& caption,
										  const QString& equipmentID);

	static QString removeNumberSign(const QString& appSignalID);

private:
	// Private setters for fields, wich can't be changed outside DB engine
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
	void setCreated(const QString& createdStr) { m_created = QDateTime::fromString(createdStr, FormatStr::POSTGRES_DATE_TIME); }
	void setDeleted(bool deleted) { m_deleted = deleted; }
	void setInstanceCreated(const QDateTime& instanceCreated) { m_instanceCreated = instanceCreated; }
	void setInstanceCreated(const QString& instanceCreatedStr) { m_instanceCreated = QDateTime::fromString(instanceCreatedStr, FormatStr::POSTGRES_DATE_TIME); }
	void setInstanceAction(E::VcsItemAction action) { m_instanceAction = action; }
	void initCreatedDates();

	bool isCompatibleFormatPrivate(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder, const QString& busTypeID) const;

	void updateTuningValuesType();

	QString specPropNotExistErr(const QString &propName) const;

private:
	bool m_isLoaded = false;										// == false - only m_ID and m_appSignalID fields is initialized from database
																	// == true - all Signal fields is initialized from database

	// Signal identificators
	//
	QString m_appSignalID;
	QString m_customAppSignalID;
	QString m_caption;
	QString m_equipmentID;											// should be transformed to portEquipmentID
	QString m_lmEquipmentID;										// now fills in compile time only
	QString m_busTypeID;											// only for: m_signalType == E::SignalType::Bus
	E::Channel m_channel = E::Channel::A;
	bool m_excludeFromBuild = false;

	bool m_isAutoSignal = false;

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

	std::shared_ptr<AppSignalSpecPropValues> m_cachedSpecPropValues;

	std::set<QString> m_tags;

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
	E::VcsItemAction m_instanceAction = E::VcsItemAction::Added;

	// Signal properties calculated in compile-time
	//
	Hash m_hash = 0;						// == calcHash(m_appSignalID)

	Address16 m_ioBufAddr;					// signal address in i/o modules buffers for signals of input/output modules (input and output signals)
											// or

	Address16 m_tuningAddr;					// address of tunable signal  from beginning of tuning buffer

	Address16 m_tuningAbsAddr;				// absolute address of tunable signal
											// For analogs m_tuningAddr and m_tuningAbsAddr are EQUAL!
											// For discretes m_tuningAddr and m_tuningAbsAddr are different, due to 32-bits packing of discretes!

	Address16 m_ualAddr;					// signal address is used in UAL
											// may be equal to m_ioBufAddr, m_tuningAddr, m_regValueAddr or not
											// this address should be used in all signals value read/write operations in UAL

	Address16 m_regBufAddr;					// absolute signal address in registration buffer (LM's memory address)

	Address16 m_regValueAddr;				// signal Value address in FSC data packet
	Address16 m_regValidityAddr;			// signal Validity address in FSC data packet

	E::LogicModuleRamAccess m_lmRamAccess = E::LogicModuleRamAccess::Undefined;

	bool m_isConst = false;
	double m_constValue = 0;

	bool m_isEndpoint = false;

	AppSignalStateFlagsMap m_stateFlagsSignals;

	//

	bool m_needConversion = false;
};

typedef std::shared_ptr<AppSignal> AppSignalShared;

typedef PtrOrderedHash<int, AppSignal> SignalPtrOrderedHash;

class AppSignalSet : public SignalPtrOrderedHash
{
public:
	AppSignalSet();
	virtual ~AppSignalSet();

	virtual void clear() override;

	void reserve(int n);

	void buildID2IndexMap();
	void updateID2IndexInMap(const QString& appSignalId, int index);
	void updateID2IndexInMap(const AppSignal* appSignal);
	void clearID2IndexMap() { m_strID2IndexMap.clear(); }
	bool ID2IndexMapIsEmpty();

	bool contains(const QString& appSignalID) const;

	AppSignal* getSignal(const QString& appSignalID);
	const AppSignal* getSignal(const QString& appSignalID) const;

	virtual void append(const int& signalID, AppSignal* signal) override;
	void append(AppSignal* signal);

	virtual void remove(const int& signalID) override;
	virtual void removeAt(const int index) override;

	QVector<int> getChannelSignalsID(const AppSignal& signal) const;
	QVector<int> getChannelSignalsID(int signalGroupID) const;

	void resetAddresses();

	bool serializeFromProtoFile(const QString& filePath);

	int getMaxID();
	QStringList appSignalIdsList(bool removeNumberSign, bool sort) const;

	void replaceOrAppendIfNotExists(int signalID, const AppSignal& s);

private:
	QMultiHash<int, int> m_groupSignals;
	QHash<QString, int> m_strID2IndexMap;

	int m_maxID = -1;
};

