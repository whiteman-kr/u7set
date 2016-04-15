#pragma once

#include "../include/DbController.h"
#include "../include/Signal.h"
#include "../include/DeviceObject.h"
#include "../Builder/BuildResultWriter.h"
#include "IssueLogger.h"

namespace Builder
{
	class SoftwareCfgGenerator : public QObject
	{
		Q_OBJECT

	private:
		DbController* m_dbController = nullptr;
		Hardware::Software* m_software = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::EquipmentSet* m_equipment = nullptr;
		BuildResultWriter* m_buildResultWriter = nullptr;
		IssueLogger* m_log = nullptr;
		ConfigurationXmlFile * m_cfgXml = nullptr;
		QString m_subDir;

		static QList<Hardware::DeviceModule*> m_lmList;

		Hardware::DeviceRoot* m_deviceRoot = nullptr;

		bool generateMonitorCfg();
		bool writeMonitorSettings();

		template <typename TYPE>
		TYPE getObjectProperty(QString strId, QString property, bool* ok);

		void writeErrorSection(QXmlStreamWriter& xmlWriter, QString error);

		static bool buildLmList(Hardware::EquipmentSet *equipment, IssueLogger* log);

		bool writeAppSignalsXml();
		bool writeEquipmentXml();
		bool writeDataSourcesXml();

		bool generateDataAcqisitionServiceCfg();

		static const int LM_ETHERNET_ADAPTER1 = 1;
		static const int LM_ETHERNET_ADAPTER2 = 2;
		static const int LM_ETHERNET_ADAPTER3 = 3;

		struct LmEthernetAdapterNetworkProperties
		{
			int adapterNo;		// LM_ETHERNET_ADAPTER* values
			QString adapterStrID;

			// only for adapterNo == LM_ETHERNET_ADAPTER1
			//
			bool tuningEnable = true;
			QString tuningIP;
			int tuningPort = 0;
			QString tuningServiceStrID;

			// only for adapterNo == LM_ETHERNET_ADAPTER2 or adapterNo == LM_ETHERNET_ADAPTER3
			//
			bool appDataEnable = true;
			QString appDataIP;
			int appDataPort = 0;
			QString appDataServiceStrID;

			bool diagDataEnable = true;
			QString diagDataIP;
			int diagDataPort = 0;
			QString diagDataServiceStrID;

			bool getLmEthernetAdapterNetworkProperties(Hardware::DeviceModule* lm, int adapterNo, OutputLog *log);
		};

	public:
		SoftwareCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);

		bool run();

		static bool generalSoftwareCfgGeneration(DbController* db, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		static bool writeSchemas(DbController* db, BuildResultWriter* buildResultWriter, IssueLogger* log);
		static bool writeSchemasList(DbController* db,
									 BuildResultWriter* buildResultWriter,
									 int parentFileId,
									 QString fileExtension,
									 QString subDir,
									 QString group,
									 IssueLogger* log);
	};


	template <typename TYPE>
	TYPE SoftwareCfgGenerator::getObjectProperty(QString strId, QString property, bool* ok)
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
}
