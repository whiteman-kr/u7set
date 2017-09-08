#pragma once

#include "Signal.h"
#include "PropertyObject.h"


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
	Q_INVOKABLE int unitID() const { return m_signal.unitID(); }
	Q_INVOKABLE double lowValidRange() const { return m_signal.lowValidRange(); }
	Q_INVOKABLE double highValidRange() const { return m_signal.highValidRange(); }
//	Q_INVOKABLE double unbalanceLimit() const { return m_signal.unbalanceLimit(); }
	Q_INVOKABLE double inputLowLimit() const { return m_signal.inputLowLimit(); }
	Q_INVOKABLE double inputHighLimit() const { return m_signal.inputHighLimit(); }
	Q_INVOKABLE int jsInputUnitID() const { return static_cast<int>(m_signal.inputUnitID());}
	Q_INVOKABLE int jsInputSensorType() const { return static_cast<int>(m_signal.inputSensorType());}
	Q_INVOKABLE double outputLowLimit() const { return m_signal.outputLowLimit(); }
	Q_INVOKABLE double outputHighLimit() const { return m_signal.outputHighLimit(); }
	Q_INVOKABLE int outputUnitID() const { return m_signal.outputUnitID(); }
	Q_INVOKABLE int jsOutputMode() const { return static_cast<int>(m_signal.outputMode());}
	Q_INVOKABLE int jsOutputSensorType() const { return static_cast<int>(m_signal.outputSensorType());}
	Q_INVOKABLE bool acquire() const { return m_signal.acquire(); }
	Q_INVOKABLE bool calculated() const { return m_signal.calculated(); }
	Q_INVOKABLE int normalState() const { return m_signal.normalState(); }
	Q_INVOKABLE int decimalPlaces() const { return m_signal.decimalPlaces(); }
	Q_INVOKABLE double aperture() const { return m_signal.roughAperture(); }
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

