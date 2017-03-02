#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

namespace Builder
{
	class SignalParam;
	class SignalLocation;

	class MetrologyCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool writeSettings();
		bool writeMetrologySignalsXml();


	public:
		MetrologyCfgGenerator(	DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);
		~MetrologyCfgGenerator();

		virtual bool generateConfiguration() override;

		template <typename TYPE>
		TYPE getObjectProperty(QString strId, QString property, bool* ok);
		void writeErrorSection(QXmlStreamWriter& xmlWriter, QString error);
	};

	template <typename TYPE>
	TYPE MetrologyCfgGenerator::getObjectProperty(QString strId, QString property, bool* ok)
	{
		if (ok == nullptr)
		{
			assert(false);
			return TYPE();
		}

		*ok = true;

		Hardware::DeviceObject* object = m_equipment->deviceObject(strId);
		if (object == nullptr)
		{
			QString errorStr = tr("Object %1 is not found")
							   .arg(strId);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

			*ok = false;
			return TYPE();
		}

		bool exists = object->propertyExists(property);
		if (exists == false)
		{
			QString errorStr = tr("Object %1 does not have property %2").arg(strId).arg(property);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

			*ok = false;
			return TYPE();
		}

		QVariant v = object->propertyValue(property);
		if (v.isValid() == false)
		{
			QString errorStr = tr("Object %1, property %2 is invalid").arg(strId).arg(property);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

			*ok = false;
			return TYPE();
		}

		if (v.canConvert<TYPE>() == false)
		{
			QString errorStr = tr("Object %1, property %2 has wrong type").arg(strId).arg(property);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

			*ok = false;
			return TYPE();
		}

		TYPE t = v.value<TYPE>();

		return t;
	}

	class SignalLocation
	{
		public:
			SignalLocation() {}
			SignalLocation(Hardware::DeviceObject* pDeviceObject);

		private:
			QString     m_equipmentID;

			QString     m_rackCaption;
			int         m_chassis = -1;
			int         m_module = -1;
			int         m_place = -1;

			QString     m_contact;

			void        getParentObject(Hardware::DeviceObject* pDeviceObject);

		public:

			QString equipmentID() const { return m_equipmentID; }
			void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

			QString rackCaption() const { return m_rackCaption; }
			void setRackCaption(const QString& caption) { m_rackCaption = caption; }

			int chassis() const { return m_chassis; }
			void setChassis(int chassis) { m_chassis = chassis; }

			int module() const { return m_module; }
			void setModule(int module) { m_module = module; }

			int place() const { return m_place; }
			void setPlace(int place) { m_place = place; }

			QString contact() const { return m_contact; }
			void setContact(const QString& contact) { m_contact = contact; }
	};

	class SignalParam
	{
	public:

								SignalParam() {}
								SignalParam(const Signal& signal, const SignalLocation& location) { setParam(signal, location); }
								~SignalParam() {}
	private:

		QString                 m_appSignalID;
		QString                 m_customAppSignalID;
		QString                 m_caption;

		E::SignalType           m_signalType = E::SignalType::Analog;
		E::SignalInOutType      m_inOutType = E::SignalInOutType::Internal;

		SignalLocation          m_location;

		int                     m_lowADC = 0;
		int                     m_highADC = 0;

		double                  m_inputElectricLowLimit = 0;
		double                  m_inputElectricHighLimit = 0;
		E::InputUnit            m_inputElectricUnitID = E::InputUnit::NoInputUnit;
		E::SensorType           m_inputElectricSensorType = E::SensorType::NoSensorType;
		int                     m_inputElectricPrecision = 3;

		double                  m_inputPhysicalLowLimit = 0;
		double                  m_inputPhysicalHighLimit = 0;
		int                     m_inputPhysicalUnitID = NO_UNIT_ID;
		int                     m_inputPhysicalPrecision = 2;

		double                  m_outputElectricLowLimit = 0;
		double                  m_outputElectricHighLimit = 0;
		E::InputUnit            m_outputElectricUnitID  = E::InputUnit::NoInputUnit;
		E::SensorType           m_outputElectricSensorType = E::SensorType::NoSensorType;
		int                     m_outputElectricPrecision = 3;

		double                  m_outputPhysicalLowLimit = 0;
		double                  m_outputPhysicalHighLimit = 0;
		int                     m_outputPhysicalUnitID = NO_UNIT_ID;
		int                     m_outputPhysicalPrecision = 2;

		bool                    m_enableTuning = false;
		double                  m_tuningDefaultValue = 0;

	public:

		void                    setParam(const Signal& signal, const SignalLocation& location);

		QString                 appSignalID() const { return m_appSignalID; }
		void                    setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

		QString                 customAppSignalID() const { return m_customAppSignalID; }
		void                    setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

		QString                 caption() const { return m_caption; }
		void                    setCaption(const QString& caption) { m_caption = caption; }

		E::SignalType           signalType() const { return m_signalType; }
		void                    setSignalType(const E::SignalType& type) { m_signalType = type; }

		E::SignalInOutType      inOutType() const { return m_inOutType; }
		void                    setInOutType(const E::SignalInOutType& inOutType) { m_inOutType = inOutType; }

		SignalLocation          location() const { return m_location; }
		void                    setLocation(const SignalLocation& location) { m_location = location; }

		int                     lowADC() const { return m_lowADC; }
		void                    setLowADC(int lowADC) { m_lowADC = lowADC; }

		int                     highADC() const { return m_highADC; }
		void                    setHighADC(int highADC) { m_highADC = highADC;}

		double                  inputElectricLowLimit() const { return m_inputElectricLowLimit; }
		void                    setInputElectricLowLimit(double lowLimit) { m_inputElectricLowLimit = lowLimit; }

		double                  inputElectricHighLimit() const { return m_inputElectricHighLimit; }
		void                    setInputElectricHighLimit(double highLimit) { m_inputElectricHighLimit = highLimit; }

		E::InputUnit            inputElectricUnitID() const { return m_inputElectricUnitID; }
		void                    setInputElectricUnitID(const E::InputUnit unit) { m_inputElectricUnitID = unit; }

		E::SensorType           inputElectricSensorType() const { return m_inputElectricSensorType; }
		void                    setInputElectricSensorType(const E::SensorType sensorType) { m_inputElectricSensorType = sensorType; }

		int                     inputElectricPrecision() const { return m_inputElectricPrecision; }
		void                    setInputElectricPrecision(int precision) { m_inputElectricPrecision = precision; }

		double                  inputPhysicalLowLimit() const { return m_inputPhysicalLowLimit; }
		void                    setInputPhysicalLowLimit(double lowLimit) { m_inputPhysicalLowLimit = lowLimit; }

		double                  inputPhysicalHighLimit() const { return m_inputPhysicalHighLimit; }
		void                    setInputPhysicalHighLimit(double highLimit) { m_inputPhysicalHighLimit = highLimit; }

		int                     inputPhysicalUnitID() const { return m_inputPhysicalUnitID; }
		void                    setInputPhysicalUnitID(int unit) { m_inputPhysicalUnitID = unit; }

		int                     inputPhysicalPrecision() const { return m_inputPhysicalPrecision; }
		void                    setInputPhysicalPrecision(int precision) { m_inputPhysicalPrecision = precision; }

		double                  outputElectricLowLimit() const { return m_outputElectricLowLimit; }
		void                    setOutputElectricLowLimit(double lowLimit) { m_outputElectricLowLimit = lowLimit; }

		double                  outputElectricHighLimit() const { return m_outputElectricHighLimit; }
		void                    setOutputElectricHighLimit(double highLimit) { m_outputElectricHighLimit = highLimit; }

		E::InputUnit            outputElectricUnitID() const { return m_outputElectricUnitID; }
		void                    setOutputElectricUnitID(const E::InputUnit unit) { m_outputElectricUnitID = unit; }

		E::SensorType           outputElectricSensorType() const { return m_outputElectricSensorType; }
		void                    setOutputElectricSensorType(const E::SensorType sensorType) { m_outputElectricSensorType = sensorType; }

		int                     outputElectricPrecision() const { return m_outputElectricPrecision; }
		void                    setOutputElectricPrecision(int precision) { m_outputElectricPrecision = precision; }

		double                  outputPhysicalLowLimit() const { return m_outputPhysicalLowLimit; }
		void                    setOutputPhysicalLowLimit(double lowLimit) { m_outputPhysicalLowLimit = lowLimit; }

		double                  outputPhysicalHighLimit() const { return m_outputPhysicalHighLimit; }
		void                    setOutputPhysicalHighLimit(double highLimit) { m_outputPhysicalHighLimit = highLimit; }

		int                     outputPhysicalUnitID() const { return m_outputPhysicalUnitID; }
		void                    setOutputPhysicalUnitID(int unit) { m_outputPhysicalUnitID = unit; }

		int                     outputPhysicalPrecision() const { return m_outputPhysicalPrecision; }
		void                    setOutputPhysicalPrecision(int precision) { m_outputPhysicalPrecision = precision; }

		bool                    enableTuning() const { return m_enableTuning; }
		void                    setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

		double                  tuningDefaultValue() const { return m_tuningDefaultValue; }
		void                    setTuningDefaultValue(double value) { m_tuningDefaultValue = value; }

		void                    writeToXml(XmlWriteHelper& xml);
	};
}
