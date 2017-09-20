#pragma once

#include "Signal.h"
#include "PropertyObject.h"


static const QString idCaption("ID");										// Optimization, to share one string among all Signal instances
static const QString signalGroupIDCaption("SignalGroupID");
static const QString signalInstanceIDCaption("SignalInstanceID");
static const QString changesetIDCaption("ChangesetID");
static const QString checkedOutCaption("CheckedOut");
static const QString userIdCaption("UserID");
static const QString channelCaption("Channel");
static const QString createdCaption("Created");
static const QString deletedCaption("Deleted");
static const QString instanceCreatedCaption("InstanceCreated");
//static const QString instanceActionCaption("InstanceAction");
static const QString typeCaption("Type");
static const QString inOutTypeCaption("InOutType");
static const QString cacheValidator("^[#]?[A-Za-z\\d_]*$");
static const QString appSignalIDCaption("AppSignalID");
static const QString customSignalIDCaption("CustomAppSignalID");
static const QString busTypeIDCaption("BusTypeID");
static const QString captionCaption("Caption");
static const QString captionValidator("^.+$");
static const QString analogDataFormatCaption("AnalogDataFormat");
static const QString dataSizeCaption("DataSize");
static const QString lowADCCaption("LowADC");
static const QString highADCCaption("HighADC");
static const QString lowDACCaption("LowDAC");
static const QString highDACCaption("HighDAC");
static const QString lowEngeneeringUnitsCaption("LowEngeneeringUnits");
static const QString highEngeneeringUnitsCaption("HighEngeneeringUnits");
static const QString unitCaption("Unit");
static const QString lowValidRangeCaption("LowValidRange");
static const QString highValidRangeCaption("HighValidRange");
static const QString unbalanceLimitCaption("UnbalanceLimit");
static const QString outputModeCaption("OutputMode");
static const QString acquireCaption("Acquire");
static const QString normalStateCaption("NormalState");
static const QString decimalPlacesCaption("DecimalPlaces");
static const QString coarseApertureCaption("CoarseAperture");
static const QString fineApertureCaption("FineAperture");
static const QString adaptiveApertureCaption("AdaptiveAperture");
static const QString filteringTimeCaption("FilteringTime");
static const QString spreadToleranceCaption("SpreadTolerance");
static const QString byteOrderCaption("ByteOrder");
static const QString equipmentIDCaption("EquipmentID");
static const QString enableTuningCaption("EnableTuning");
static const QString tuningDefaultValueCaption("TuningDefaultValue");
static const QString tuningLowBoundCaption("TuningLowBound");
static const QString tuningHighBoundCaption("TuningHighBound");
static const QString identificationCategory("1 Identification");
static const QString signalTypeCategory("2 Signal type");
static const QString dataFormatCategory("3 Data Format");
static const QString signalProcessingCategory("4 Signal processing");
static const QString onlineMonitoringSystemCategory("5 Online Monitoring System");
static const QString tuningCategory("6 Tuning");


class SignalProperties : public PropertyObject
{
	Q_OBJECT

private:
	Signal m_signal;

	std::vector<Property*> m_propertiesDependentOnPrecision;

	void initProperties();

public:
	explicit SignalProperties(Signal& signal);

	Signal& signal() { return m_signal; }

	const std::vector<Property*>& propertiesDependentOnPrecision() { return m_propertiesDependentOnPrecision; }

	Q_INVOKABLE QString appSignalID() const { return m_signal.appSignalID(); }
	Q_INVOKABLE QString customAppSignalID() const { return m_signal.customAppSignalID(); }
	Q_INVOKABLE QString caption() const { return m_signal.caption(); }
	Q_INVOKABLE int dataSize() const { return m_signal.dataSize(); }
	Q_INVOKABLE int lowADC() const { return m_signal.lowADC(); }
	Q_INVOKABLE int highADC() const { return m_signal.highADC(); }
	Q_INVOKABLE double lowEngeneeringUnits() const { return m_signal.lowEngeneeringUnits(); }
	Q_INVOKABLE double highEngeneeringUnits() const { return m_signal.highEngeneeringUnits(); }
	Q_INVOKABLE double lowValidRange() const { return m_signal.lowValidRange(); }
	Q_INVOKABLE double highValidRange() const { return m_signal.highValidRange(); }
	Q_INVOKABLE double inputLowLimit() const { return m_signal.electricLowLimit(); }
	Q_INVOKABLE double inputHighLimit() const { return m_signal.electricHighLimit(); }
	Q_INVOKABLE int jsInputUnitID() const { return static_cast<int>(m_signal.electricUnit());}
	Q_INVOKABLE int jsInputSensorType() const { return static_cast<int>(m_signal.sensorType());}
	Q_INVOKABLE int jsOutputMode() const { return static_cast<int>(m_signal.outputMode());}
	Q_INVOKABLE bool acquire() const { return m_signal.acquire(); }
	Q_INVOKABLE int decimalPlaces() const { return m_signal.decimalPlaces(); }
	Q_INVOKABLE double aperture() const { return m_signal.coarseAperture(); }
	Q_INVOKABLE E::SignalInOutType inOutType() const { return m_signal.inOutType(); }
	Q_INVOKABLE QString equipmentID() const { return m_signal.equipmentID(); }
	Q_INVOKABLE double filteringTime() const { return m_signal.filteringTime(); }
	Q_INVOKABLE double spreadTolerance() const { return m_signal.spreadTolerance(); }
	Q_INVOKABLE E::ByteOrder byteOrder() const { return m_signal.byteOrder(); }
	Q_INVOKABLE int byteOrderInt() const { return TO_INT(m_signal.byteOrder()); }
	Q_INVOKABLE bool enableTuning() const { return m_signal.enableTuning(); }
	Q_INVOKABLE float tuningDefaultValue() const { return m_signal.tuningDefaultValue(); }
	Q_INVOKABLE float tuningLowBound() const { return m_signal.tuningLowBound(); }
	Q_INVOKABLE float tuningHighBound() const { return m_signal.tuningHighBound(); }
};

