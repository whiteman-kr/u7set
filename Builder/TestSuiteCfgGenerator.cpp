#include "TestSuiteCfgGenerator.h"
#include "../OnlineLib//SoftwareSettings.h"
#include "Context.h"
#include "SoftwareSettingsGetter.h"
#include "TuningClientCfgGenerator.h"
#include <QJSValueIterator>

namespace Builder
{

	TestSuiteCfgGenerator::TestSuiteCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	TestSuiteCfgGenerator::~TestSuiteCfgGenerator()
	{
	}

	bool TestSuiteCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TestSuiteSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<TestSuiteSettings>(profile, settingsGetter);
	}

	bool TestSuiteCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr ||
				m_software->softwareType() != E::SoftwareType::TestSuite ||
				m_equipment == nullptr ||
				m_cfgXml == nullptr ||
				m_buildResultWriter == nullptr)
		{
			Q_ASSERT(m_software && m_software->softwareType() == E::SoftwareType::TestSuite);
			Q_ASSERT(m_equipment);
			Q_ASSERT(m_cfgXml);
			Q_ASSERT(m_buildResultWriter);
			return false;
		}

		// Writing GlobalScript
		//
		bool result = true;

		result &= initTuningSources();

		std::shared_ptr<const TestSuiteSettings> settings = m_settingsSet.getSettingsDefaultProfile<TestSuiteSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		if (settings->tuningEnabled == true)
		{
			// Generate tuning signals file
			//
			result &= writeTuningSignals();

			if (settings->login == true && settings->userAccounts.isEmpty() == true)
			{
				m_log->errEQP6202(EquipmentPropNames::TESTING_USER_ACCOUNTS, EquipmentPropNames::TESTING_LOGIN, m_software->equipmentIdTemplate());
				return false;
			}
			
			// Write MATS users
			//
			if (settings->login == true)
			{
				result &= writeMatsUsers(EquipmentPropNames::TESTING_USER_ACCOUNTS,
										 settings->userAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts));
			}
		}

		result &= writeTestScripts();

		result &= writeReportTemplates();

		return result;
	}

	bool TestSuiteCfgGenerator::initTuningSources()
	{
		std::shared_ptr<const TestSuiteSettings> settings = m_settingsSet.getSettingsDefaultProfile<TestSuiteSettings>();

		if (settings->tuningEnabled == false)
		{
			return true;
		}

		if (settings->tuningServices.empty() == true)
		{
			// Property %1.TuningServiceID can't be empty if tuning enabled.
			//
			m_log-> errEQP6206(equipmentID());
			return false;
		}

		bool result = true;

		m_tuningSources.clear();

		for(const SoftwareEndpoint::TuningService& tsc : settings->tuningServices)
		{
			std::shared_ptr<Hardware::DeviceObject> tuningServiceObject = m_equipment->deviceObject(tsc.equipmentId);
			if (tuningServiceObject == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, tsc.equipmentId);
				result = false;
				continue;
			}
			std::shared_ptr<Hardware::Software> tuningServiceSoftware = tuningServiceObject->toSoftware();
			if (tuningServiceSoftware == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, tsc.equipmentId);
				result = false;
				continue;
			}

			TuningServiceSettingsGetter tsg;

			if (tsg.readSoftwareSettings(m_context, tuningServiceSoftware.get()) == false)
			{
				result = false;
				continue;
			}

			TuningServiceSettingsGetter::TuningClient tunClient = tsg.getTuningClient(equipmentID());

			if (tunClient.isValid() == true)
			{
				QStringList clientEquipmentList = tunClient.uniqueSourcesIDs();

				for (const QString& ce : clientEquipmentList )
				{
					if (m_tuningSources.contains(ce) == false)
					{
						m_tuningSources.append(ce);
					}
				}
			}
			else
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("TestSuite %1 isn't found in clients list of TuningService %2").
									   arg(equipmentID()).arg(tsc.equipmentId));
				result = false;
				continue;
			}
		}

		return result;
	}

	bool TestSuiteCfgGenerator::writeTuningSignals()
	{
		if (m_tuningSources.empty() == true)
		{
			//Q_ASSERT(m_tuningSources.empty() == false);
			return false;
		}

		::Proto::AppSignalSet tuningSet;

		bool ok = TuningClientCfgGenerator::createTuningSignals(m_tuningSources, m_signalSet, &tuningSet);
		if (ok == false)
		{
			m_log->errINT1000("Generate tuning signal set error: TestSuiteCfgGenerator::writeTuningSignals, call for TuningClientCfgGenerator::createTuningSignals");
			return false;
		}

		// Write number of signals
		//
		QByteArray data;
		data.resize(static_cast<int>(tuningSet.ByteSizeLong()));

		tuningSet.SerializeToArray(data.data(), static_cast<int>(tuningSet.ByteSizeLong()));

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.dat", CfgFileId::TUNING_SIGNALS, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningSignals.dat");
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);
		return ok;
	}

	bool TestSuiteCfgGenerator::writeTestScripts()
	{
		Q_ASSERT(m_context->m_buildResultWriter);

		DbFileTree fileTree;

		if (bool ok = m_context->m_db.getFileListTree(&fileTree, DbDir::TestsDir, true, nullptr);
				ok == false)
		{
			m_context->m_log->errPDB2001(m_context->m_db.systemFileId(DbDir::TestsDir), "", m_context->m_db.lastError());
			return false;
		}

		bool result = true;

		// Get script tags
		//
		QStringList scriptTags;
		result &= DeviceHelper::getStrListProperty(m_software, EquipmentPropNames::TESTING_SCRIPTTAGS, &scriptTags, m_log);

		// --
		//
		const std::map<int, std::shared_ptr<DbFileInfo>>& files = fileTree.files();

		QString javaScriptFileExtension{Db::File::JavaScriptFileExtension};

		for (auto& [fileId, fileInfo] : files)
		{
			if (fileInfo->isFolder() == true)
			{
				continue;
			}

			QString fileExt = fileInfo->extension();

			if (fileExt.compare(javaScriptFileExtension, Qt::CaseInsensitive) != 0)
			{
				continue;
			}

			std::shared_ptr<DbFile> file;

			bool ok = m_context->m_db.getLatestVersion(*fileInfo, &file, nullptr);
			if (ok == true)
			{
				// Check script tags specified in ScriptTags variable of the script. GlobalScript is placed always.
				//
				if (fileInfo->fileName().endsWith(File::GLOBAL_SCRIPT, Qt::CaseInsensitive) == false)
				{
					bool checkTag = checkScriptFileTags(file, scriptTags);
					if (checkTag == false)
					{
						continue;
					}
				}

				QString folderPath = Db::File::systemDirToName(DbDir::RootDir) + "/";

				{
					QStringList pathList;
					std::shared_ptr<DbFileInfo> f = fileTree.file(fileId);

					while (f != nullptr)
					{
						f = fileTree.file(f->parentId());

						if (f != nullptr)
						{
							pathList.push_front(f->fileName());
						}
					}

					folderPath += pathList.join(QChar('/'));
				}

				// Process files from Tests or HardwareTests folders
				//
				if (folderPath.startsWith(Db::File::systemDirToName(DbDir::TestsDir)) ||
					folderPath.startsWith(Db::File::systemDirToName(DbDir::HardwareTestsDir)))
				{
					BuildFile* buildFile = m_context->m_buildResultWriter->addFile(m_software->equipmentIdTemplate() + fileTree.filePath(fileId),
																				   file->fileName(),
																				   file->data());
					if (buildFile == nullptr)
					{
						Q_ASSERT(buildFile);
						return false;
					}

					m_cfgXml->addLinkToFile(buildFile);
				}
			}
			else
			{
				m_context->m_log->errPDB2002(fileInfo->fileId(), fileInfo->fileName(), m_context->m_db.lastError());
				return false;
			}
		}

		return true;
	}

	bool TestSuiteCfgGenerator::writeReportTemplates()
	{
		// Writing GlobalScript
		//
		bool result = true;

		if (m_software->propertyExists("ReportTemplates") == false)
		{
			m_log->errCFG3000("ReportTemplates", m_software->equipmentIdTemplate());
			result = false;
		}
		else
		{
			QString templates = m_software->propertyValue("ReportTemplates").toString();
			BuildFile* templatesBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(),
																			"ReportTemplates.dat",
																			CfgFileId::TESTSUITE_REPORTTEMPLATES, "", templates);

			m_cfgXml->addLinkToFile(templatesBuildFile);
		}

		return result;
	}

	bool TestSuiteCfgGenerator::checkScriptFileTags(std::shared_ptr<DbFile>& file, const QStringList& scriptTags)
	{
		if (scriptTags.empty() == true)
		{
			// No ScriptTags property is empty, return positive result
			//
			return true;
		}

		QJSEngine jsEngine;

		// Evaluate script.
		//
		QJSValue scriptValue = jsEngine.evaluate(QString::fromStdString(file->data().toStdString()));

		if (scriptValue.isError() == true)
		{
			return false;
		}

		QJSValueIterator it(jsEngine.globalObject());
		while (it.hasNext() == true)
		{
			it.next();

			QString objectName = it.name();

			if (objectName.compare("ScriptTags", Qt::CaseInsensitive) == 0)
			{
				QStringList value = QVariant().fromValue(it.value()).toStringList();

				for (const QString& tag : value)
				{
					if (scriptTags.contains(tag) == true)
					{
						// Tag was found
						return true;
					}
				}

				// Tag was not found
				//
				return false;
			}
		}

		// No ScriptTags variable found, return negative result
		//
		return false;
	}
}

