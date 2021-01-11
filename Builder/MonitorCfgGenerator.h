#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{
	class Context;

	class MonitorCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		MonitorCfgGenerator(Context* context, Hardware::Software* software);
		~MonitorCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfiguration() override;

	protected:
		bool initSchemaTagsAndTuningSources();

		bool saveScriptProperties(QString scriptProperty, QString fileName);

		bool writeSchemasByTags();

		void writeErrorSection(QXmlStreamWriter& xmlWriter, QString error);

		template <typename TYPE>
		TYPE getObjectProperty(QString strId, QString property, bool* ok);

		template <typename TYPE>
		std::pair<TYPE, bool> getObjectProperty(QString strId, QString property);

		// Generate tuning signals file
		//
		bool writeTuningSignals();
		bool writeMonitorBehavior();
		bool writeMonitorLogo();

	private:
//		bool m_tuningEnabled = false;
		QStringList m_tuningSources;

		QStringList m_schemaTagList;		// Generated in writeMonitorSettings
	};


	template <typename TYPE>
	TYPE MonitorCfgGenerator::getObjectProperty(QString strId, QString property, bool* ok)
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
			m_log->errCFG3021(m_software->equipmentId(), property, strId);

			QString errorStr = tr("Object %1 is not found")
							   .arg(strId);

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

	template <typename TYPE>
	std::pair<TYPE, bool> MonitorCfgGenerator::getObjectProperty(QString strId, QString property)
	{
		bool ok = false;
		TYPE result = getObjectProperty<TYPE>(strId, property, &ok);

		return {result, ok};
	}

}

