#include "TuningClientCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../VFrame30/Schema.h"
#include "../lib/AppSignal.h"

namespace Builder
{

	TuningClientCfgGenerator::TuningClientCfgGenerator(DbController* db,
													   Hardware::SubsystemStorage* subsystems,
													   Hardware::Software* software,
													   SignalSet* signalSet,
													   Hardware::EquipmentSet* equipment,
													   BuildResultWriter* buildResultWriter)	:
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_subsystems(subsystems)
	{
	}

	bool TuningClientCfgGenerator::generateConfiguration()
	{
		if (m_software == nullptr ||
				m_software->type() != E::SoftwareType::TuningClient ||
				m_equipment == nullptr ||
				m_cfgXml == nullptr ||
				m_buildResultWriter == nullptr ||
				m_subsystems == nullptr)
		{
			assert(m_software);
			assert(m_software->type() == E::SoftwareType::Monitor);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			assert(m_subsystems);
			return false;
		}

		IssueLogger* log = m_buildResultWriter->log();
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		QStringList equipmentList;

		bool filterByEquipment = false;
		bool filterBySchema = false;
		bool showDiscreteCounters = false;

		result &= createEquipmentList(&equipmentList);
		if (result == false)
		{
			return result;
		}

		result &= createSettings(&filterByEquipment, &filterBySchema, &showDiscreteCounters);
		if (result == false)
		{
			return result;
		}


		result &= createTuningSignals(equipmentList);
		if (result == false)
		{
			return result;
		}

		result &= createObjectFilters(equipmentList, filterByEquipment, filterBySchema, showDiscreteCounters);
		if (result == false)
		{
			return result;
		}

		result &= writeSettings(filterByEquipment, filterBySchema, showDiscreteCounters);
		if (result == false)
		{
			return result;
		}

		result &= writeTuningSignals();
		if (result == false)
		{
			return result;
		}

		result &= writeObjectFilters();
		if (result == false)
		{
			return result;
		}

		result &= writeTuningSchemas();
		if (result == false)
		{
			return result;
		}

		result &= writeGlobalScript();

		return result;
	}

	bool TuningClientCfgGenerator::createEquipmentList(QStringList* equipmentList)
	{
		QString equipmentString;

		if (equipmentList == nullptr)
		{
			assert(equipmentList);
			return false;
		}

		//
		// equipmentList
		//
		bool ok = false;

		equipmentString = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		// Parse equipmentList
		//
		if (equipmentString.isEmpty() == false)
		{
			equipmentString.replace(' ', ';');
			equipmentString.replace('\n', ';');
			equipmentString.remove('\r');
			*equipmentList = equipmentString.split(';');
		}

		// Check for valid EquipmentIds
		//

		for (const QString& tuningEquipmentID : *equipmentList)
		{
			if (m_equipment->deviceObject(tuningEquipmentID) == nullptr)
			{
				m_log->errEQP6109(tuningEquipmentID, m_software->equipmentIdTemplate());
				return false;
			}
		}

		return true;
	}

	bool TuningClientCfgGenerator::createSettings(bool* filterByEquipment, bool* filterBySchema, bool* showDiscreteCounters)
	{
		if (filterByEquipment == nullptr || filterBySchema == nullptr || showDiscreteCounters == nullptr)
		{
			assert(filterByEquipment);
			assert(filterBySchema);
			assert(showDiscreteCounters);
			return false;
		}

		bool ok = false;

		//
		// filterByEquipment
		//
		*filterByEquipment = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "FilterByEquipment", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// filterBySchema
		//
		*filterBySchema = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "FilterBySchema", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// showDiscreteCounters
		//
		*showDiscreteCounters = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowDiscreteCounters", &ok);
		if (ok == false)
		{
			return false;
		}

		return true;
	}

	bool TuningClientCfgGenerator::writeSettings(bool filterByEquipment, bool filterBySchema, bool showDiscreteCounters)
	{

		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		{
			xmlWriter.writeStartElement("Settings");
			std::shared_ptr<int*> writeEndSettings(nullptr, [&xmlWriter](void*)
			{
				xmlWriter.writeEndElement();
			});

			// --
			//
			bool ok = true;

			//
			// ConfigurationServiceID1,2
			//

			QStringList configServiceIdProperties;
			configServiceIdProperties << "ConfigurationServiceID1";
			configServiceIdProperties << "ConfigurationServiceID2";

			for (const QString& cfgsProperty : configServiceIdProperties)
			{
				QString configurationServiceId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), cfgsProperty, &ok).trimmed();
				if (ok == false)
				{
					return false;
				}

				if (configurationServiceId.isEmpty() == true)
				{
					m_log->errCFG3022(m_software->equipmentId(), cfgsProperty);

					QString errorStr = tr("TuningClient configuration error %1, property %2 is invalid")
							.arg(m_software->equipmentIdTemplate()).arg(cfgsProperty);

					writeErrorSection(xmlWriter, errorStr);
					return false;
				}

				Hardware::Software* cfgs = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(configurationServiceId));

				if (cfgs == nullptr)
				{
					m_log->errCFG3021(m_software->equipmentIdTemplate(), cfgsProperty, configurationServiceId);

					QString errorStr = tr("Object %1 is not found").arg(configurationServiceId);

					writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
					return false;
				}

				if (cfgs->type() != E::SoftwareType::ConfigurationService)
				{
					m_log->errCFG3017(m_software->equipmentIdTemplate(), cfgsProperty, configurationServiceId);

					QString errorStr = tr("Property '%1.%2' is linked to not compatible software '%3'.")
						.arg(m_software->equipmentIdTemplate())
						.arg(cfgsProperty)
						.arg(configurationServiceId);


					writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
					return false;
				}
			}

			//
			// TuningServiceID
			//

			QString tunsProperty = "TuningServiceID";

			QString tuningServiceId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), tunsProperty, &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (tuningServiceId.isEmpty() == true)
			{
				m_log->errCFG3022(m_software->equipmentId(), tunsProperty);

				QString errorStr = tr("TuningClient configuration error %1, property %2 is invalid")
						.arg(m_software->equipmentIdTemplate()).arg(tunsProperty);

				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			Hardware::Software* tunsObject = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId));

			if (tunsObject == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentIdTemplate(), tunsProperty, tuningServiceId);

				QString errorStr = tr("Object %1 is not found").arg(tuningServiceId);

				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			if (tunsObject->type() != E::SoftwareType::TuningService)
			{
				m_log->errCFG3017(m_software->equipmentIdTemplate(), tunsProperty, tuningServiceId);

				QString errorStr = tr("Property '%1.%2' is linked to not compatible software '%3'.")
					.arg(m_software->equipmentIdTemplate())
					.arg(tunsProperty)
					.arg(tuningServiceId);


				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			TuningServiceSettings tunsSettings;
			tunsSettings.readFromDevice(tunsObject, m_log);

			//
			// AutoApply
			//
			bool autoApply = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "AutoApply", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// showSignals
			//
			bool showSignals = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSignals", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// showSchemas
			//
			bool showSchemas = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSchemas", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// showSchemasList
			//
			bool showSchemasList = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSchemasList", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// showSOR
			//
			bool showSOR = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSOR", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// loginPerOperation
			//
			bool loginPerOperation = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "LoginPerOperation", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// usersAccounts
			//
			QString usersAccounts = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "UsersAccounts", &ok);
			if (ok == false)
			{
				return false;
			}

			//
			// loginSessionLength
			//
			int loginSessionLength = getObjectProperty<int>(m_software->equipmentIdTemplate(), "LoginSessionLength", &ok);
			if (ok == false)
			{
				return false;
			}


			{
				xmlWriter.writeStartElement("TuningService");
				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
				{
					xmlWriter.writeEndElement();
				});

				// --
				//
				xmlWriter.writeAttribute("TuningServiceID1", tuningServiceId);

				xmlWriter.writeAttribute("ip1", tunsSettings.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port1", QString::number(tunsSettings.clientRequestIP.port()));
			}

			{
				xmlWriter.writeStartElement("Appearance");
				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
				{
					xmlWriter.writeEndElement();
				});

				xmlWriter.writeAttribute("autoApply", (autoApply ? "true" : "false"));
				xmlWriter.writeAttribute("showSignals", (showSignals ? "true" : "false"));
				xmlWriter.writeAttribute("showSchemas", (showSchemas? "true" : "false"));
				xmlWriter.writeAttribute("showSchemasList", (showSchemasList ? "true" : "false"));
				xmlWriter.writeAttribute("filterByEquipment", (filterByEquipment ? "true" : "false"));
				xmlWriter.writeAttribute("filterBySchema", (filterBySchema ? "true" : "false"));
				xmlWriter.writeAttribute("showSOR", (showSOR ? "true" : "false"));
				xmlWriter.writeAttribute("showDiscreteCounters", (showDiscreteCounters ? "true" : "false"));
				xmlWriter.writeAttribute("loginPerOperation", (loginPerOperation ? "true" : "false"));
				xmlWriter.writeAttribute("loginSessionLength", QString::number(loginSessionLength));
				xmlWriter.writeAttribute("usersAccounts", usersAccounts);
			}
		}

		{
			xmlWriter.writeStartElement("Schemas");
			std::shared_ptr<int*> writeEndSettings(nullptr, [&xmlWriter](void*)
			{
				xmlWriter.writeEndElement();
			});

			// --
			//
			bool ok = true;

			//
			// Schemas
			//
			QString schemas = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Schemas", &ok);
			if (ok == false)
			{
				return false;
			}

			QStringList schemasList;
			if (schemas.isEmpty() == false)
			{
				schemas.replace('\n', ';');
				schemasList = schemas.split(';');
			}


			for (auto s : schemasList)
			{
				// Check if schema with specified ID exists
				//

				QString caption;

				bool found = false;

				for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
				{
					if (schemaFile.id == s)
					{

						VFrame30::SchemaDetails details;
						bool parsed = details.parseDetails(schemaFile.details);
						if (parsed)
						{
							caption = details.m_caption;
						}
						found = true;
						break;
					}
				}

				if (found == false)
				{
					m_log->errEQP6106(s, m_software->equipmentId());
					return false;
				}

				xmlWriter.writeStartElement("Schema");
				xmlWriter.writeAttribute("Id", s);
				xmlWriter.writeAttribute("Caption", caption);
				xmlWriter.writeEndElement();
			}
		}

		return true;
	}

	bool TuningClientCfgGenerator::createTuningSignals(const QStringList& equipmentList)
	{
		// Create signals

		m_tuningSignalManager.reset();

		m_tuningSet.Clear();

		if (equipmentList.empty() == true)
		{
			m_log->errCFG3022(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID");
			return false;
		}

		int signalsCount = m_signalSet->count();

		for (int i = 0; i < signalsCount; i++)
		{
			const Signal& s = (*m_signalSet)[i];

			if (s.enableTuning() == false)
			{
				continue;
			}

			// Check EquipmentIdMasks
			//

			bool result = false;

			for (QString m : equipmentList)
			{
				m = m.trimmed();

				if (m.isEmpty() == true)
				{
					continue;
				}

				QRegExp rx(m);
				rx.setPatternSyntax(QRegExp::Wildcard);
				if (rx.exactMatch(s.equipmentID()))
				{
					result = true;
					break;
				}
			}

			if (result == false)
			{
				continue;
			}

			::Proto::AppSignal* aspMessage = m_tuningSet.add_appsignal();
			s.serializeTo(aspMessage);
		}

		// Write signals to tuningSignalManager

		m_tuningSignalManager.load(m_tuningSet);

		return true;
	}

	bool TuningClientCfgGenerator::createObjectFilters(const QStringList& equipmentList, bool filterByEquipment, bool filterBySchema, bool showDiscreteCounters)
	{
		bool ok = true;

		//
		// Filters
		//
		QString filters = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Filters", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (filters.isEmpty() == true)
		{
			m_log->errCFG3022(m_software->equipmentId(), "Filters");
			return false;
		}

		// Load project filters

		QString errorCode;

		ok = m_tuningFilterStorage.load(filters.toUtf8(), &errorCode);
		if (ok == false)
		{
			m_log->errEQP6107("Filters", m_software->equipmentId());
			return false;
		}

		// Check all filters for non-existing signals

		std::vector<std::pair<QString, QString>> notFoundSignalsAndFilters;

		m_tuningFilterStorage.checkFilterSignals(m_tuningSignalManager.signalHashes(), notFoundSignalsAndFilters);

		if (notFoundSignalsAndFilters.empty() == false)
		{
			for (const std::pair<QString, QString>& p: notFoundSignalsAndFilters)
			{
				m_log->errEQP6108(p.first, p.second, m_software->equipmentId());
			}

			return false;
		}

		// Create schemas and equipment filters

		ok = createAutomaticFilters(equipmentList, filterByEquipment, filterBySchema, showDiscreteCounters);
		if (ok == false)
		{
			assert(false);
			return false;
		}

		return true;
	}


	bool TuningClientCfgGenerator::writeTuningSignals()
	{
		// Write number of signals

		QByteArray data;
		data.resize(m_tuningSet.ByteSize());

		m_tuningSet.SerializeToArray(data.data(), m_tuningSet.ByteSize());

		// Write file

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.dat", CFG_FILE_ID_TUNING_SIGNALS, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningSignals.dat");
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;
	}

	bool TuningClientCfgGenerator::writeObjectFilters()
	{
		// Save filters to file

		QByteArray data;

		bool ok = m_tuningFilterStorage.save(data);
		if (ok == false)
		{
			assert(false);
			return false;
		}

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "ObjectFilters.xml", CFG_FILE_ID_TUNING_FILTERS, "",  QString::fromLocal8Bit(data));

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("ObjectFilters.xml");
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;

	}


	bool TuningClientCfgGenerator::writeTuningSchemas()
	{
		bool ok = true;
		//
		// Schemas
		//
		QString schemas = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Schemas", &ok);
		if (ok == false)
		{
			return false;
		}

		QStringList schemasList;
		if (schemas.isEmpty() == false)
		{
			schemas.replace('\n', ';');
			schemasList = schemas.split(';');
		}


		for (const QString& s : schemasList)
		{
			bool found = false;

			// Check if schema with specified ID exists
			//
			for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
			{
				if (schemaFile.id == s)
				{
					m_cfgXml->addLinkToFile(schemaFile.subDir, schemaFile.fileName);

					found = true;
					break;
				}
			}

			if (found == false)
			{
				m_log->errEQP6106(s, m_software->equipmentId());
				return false;
			}
		}

		return true;
	}

	bool TuningClientCfgGenerator::writeGlobalScript()
	{
		// Writing GlobalScript
		//
		bool result = true;

		if (m_software->propertyExists("GlobalScript") == false)
		{
			m_log->errCFG3000("GlobalScript", m_software->equipmentIdTemplate());
			result = false;
		}
		else
		{
			QString globalScript = m_software->propertyValue("GlobalScript").toString();
			BuildFile* globalScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "GlobalScript.js", CFG_FILE_ID_TUNING_GLOBALSCRIPT, "", globalScript);

			m_cfgXml->addLinkToFile(globalScriptBuildFile);
		}

		return result;
	}

	void TuningClientCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}

	bool TuningClientCfgGenerator::createAutomaticFilters(const QStringList& equipmentList, bool filterByEquipment, bool filterBySchema, bool showDiscreteCounters)
	{
		if (filterBySchema == true)
		{
			// Filter for Schema
			//
			std::shared_ptr<TuningFilter> ofSchema = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			ofSchema->setID("%AUTOFILTER%_SCHEMA");
			ofSchema->setCaption(QObject::tr("Schemas"));
			ofSchema->setSource(TuningFilter::Source::Schema);

			for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
			{
				std::shared_ptr<VFrame30::SchemaDetails> details = std::make_shared<VFrame30::SchemaDetails>(schemaFile.details);

				std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
				for (const QString& appSignalID : details->m_signals)
				{
					// find if this signal is a tuning signal
					//
					Hash hash = ::calcHash(appSignalID);

					if (m_tuningSignalManager.signalExists(hash) == false)
					{
						continue;
					}

					TuningFilterValue ofv;
					ofv.setAppSignalId(appSignalID);
					ofTs->addValue(ofv);
				}

				if (ofTs->valuesCount() == 0)
				{
					// Do not add empty filters
					//
					continue;
				}

				ofTs->setID("%AUFOFILTER%_SCHEMA_" + details->m_schemaId);

				//QString s = QString("%1 - %2").arg(schemasDetails.m_Id).arg(schemasDetails.m_caption);
				ofTs->setCaption(details->m_caption);
				ofTs->setSource(TuningFilter::Source::Schema);
				ofTs->setHasDiscreteCounter(showDiscreteCounters);

				ofSchema->addChild(ofTs);
			}

			m_tuningFilterStorage.add(ofSchema, true);
		}	 // filterBySchema



		if (filterByEquipment == true)
		{
			// Filter for EquipmentId
			//
			std::shared_ptr<TuningFilter> ofEquipment = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			ofEquipment->setID("%AUTOFILTER%_EQUIPMENT");
			ofEquipment->setCaption(QObject::tr("Equipment"));
			ofEquipment->setSource(TuningFilter::Source::Equipment);

			for (const QString& ts : equipmentList)
			{
				std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
				ofTs->setEquipmentIDMask(ts);
				ofTs->setID("%AUFOFILTER%_EQUIPMENT_" + ts);
				ofTs->setCaption(ts);
				ofTs->setSource(TuningFilter::Source::Equipment);
				ofTs->setHasDiscreteCounter(showDiscreteCounters);

				ofEquipment->addChild(ofTs);
			}

			m_tuningFilterStorage.add(ofEquipment, true);
		} // filterByEquipment

		return true;
	}

}
