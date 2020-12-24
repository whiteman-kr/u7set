#ifndef TUNINGCLIENTCFGGENERATOR_H
#define TUNINGCLIENTCFGGENERATOR_H

#include "SoftwareCfgGenerator.h"

#include "../lib/Tuning/TuningFilter.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{

	class TuningClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningClientCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool generateConfiguration() override;

		static bool createTuningSignals(const QStringList& equipmentList, const SignalSet* signalSet, ::Proto::AppSignalSet* tuningSet);

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool createEquipmentList(QStringList* equipmentList);
		bool createObjectFilters(const QStringList& equipmentList);

		bool writeTuningSignals();
		bool writeObjectFilters();
		bool writeTuningSchemas();
		bool writeGlobalScript();
		bool writeTuningClientBehavior();

		void writeErrorSection(QXmlStreamWriter& xmlWriter, QString error);

		bool createAutomaticFilters(const QStringList& equipmentList,
									const TuningSignalManager& tuningSignalManager);

		template <typename TYPE>
		TYPE getObjectProperty(QString strId, QString property, bool* ok);

	private:
		::Proto::AppSignalSet m_tuningSet;

		TuningFilterStorage m_tuningFilterStorage;
	};

	template <typename TYPE>
	TYPE TuningClientCfgGenerator::getObjectProperty(QString strId, QString property, bool* ok)
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
			m_log->errCFG3020(strId, property);

			QString errorStr = tr("Object %1 does not have property %2").arg(strId).arg(property);

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

}

#endif // TUNINGCLIENTCFGGENERATOR_H
