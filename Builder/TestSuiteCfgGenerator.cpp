#include "TestSuiteCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/SoftwareSettingsGetter.h"
#include "../VFrame30/Schema.h"
#include "Context.h"
#include "../lib/ClientBehavior.h"

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
	}

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

	for(const TestSuiteSettings::TuningService& tsc : settings->tuningServices)
	{
		std::shared_ptr<Hardware::DeviceObject> tuningServiceObject = m_equipment->deviceObject(tsc.tuningServiceID);
		if (tuningServiceObject == nullptr)
		{
			m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, tsc.tuningServiceID);
			result = false;
			continue;
		}
		std::shared_ptr<Hardware::Software> tuningServiceSoftware = tuningServiceObject->toSoftware();
		if (tuningServiceSoftware == nullptr)
		{
			m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, tsc.tuningServiceID);
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
										arg(equipmentID()).arg(tsc.tuningServiceID));
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

}
