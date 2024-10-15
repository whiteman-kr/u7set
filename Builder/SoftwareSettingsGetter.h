#pragma once

#include <HardwareLib/Software.h>

#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/WUtils.h"

#include "Context.h"
#include "IssueLogger.h"
#include "DeviceHelper.h"


namespace OnlineLib
{
	class DataSource;
}

// -------------------------------------------------------------------------------------------

class SoftwareSettingsGetter : virtual public SoftwareSettings
{
public:
	virtual ~SoftwareSettingsGetter();

	static bool getSoftwareConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* thisSoftware,
										const QString& propConnectedSoftwareID,
										const QString& propConnectedSoftwareIP,
										const QString& propConnectedSoftwarePort,
										QString* connectedSoftwareID,
										HostAddressPort* connectedSoftwareIP,
										bool emptyAllowed,
										const QString& defaultIP,
										int defaultPort,
										E::SoftwareType requiredSoftwareType,
										Builder::IssueLogger* log);

	static bool getSoftwareConnectionBySoftwareID(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* thisSoftware,
										const QString& connectedSoftwareID,
										const QString& propConnectedSoftwareID,
										const QString& propConnectedSoftwareIP,
										const QString& propConnectedSoftwarePort,
										HostAddressPort* connectedSoftwareIP,
										bool emptyAllowed,
										const QString& defaultIP,
										int defaultPort,
										E::SoftwareType requiredSoftwareType,
										Builder::IssueLogger* log);

	static bool getCfgServiceConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* software,
										QString* cfgServiceID1, HostAddressPort* cfgServiceAddrPort1,
										QString* cfgServiceID2, HostAddressPort* cfgServiceAddrPort2,
										Builder::IssueLogger* log);

	static bool getCfgServiceConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* software,
										SoftwareEndpoint::ConfigService* cfgService1,
										SoftwareEndpoint::ConfigService* cfgService2,
										Builder::IssueLogger* log);

	static bool getLmPropertiesFromDevice(const Hardware::DeviceModule* lm,
										  E::LanControllerType lanControllerType,
										  const Builder::Context* context,
										  OnlineLib::DataSource* ds);

	bool readSoftwareSettings(const Builder::Context* context,
							  const Hardware::Software* software);

	bool readFromDeviceByEquipmentID(const Builder::Context* context,
									const QString& softwareID,
									E::SoftwareType requiredSoftwareType = E::SoftwareType::Unknown);

protected:
	virtual bool readSettings(const Builder::Context* context,
								const Hardware::Software* software) = 0;

	static bool isRqCtrlEquipmentID(const QString& equipmentID, int* rqCtrlID = nullptr);

	static bool getRqCtrlSettings(const Hardware::Software* software,
								  const QString& rqCtrlID,
								  const std::vector<quint32>& rcsPropsFlags,		// array of RcCtrlSettings::RCS_* constants
								  RqCtrlSettings* rcSettings,
								  Builder::IssueLogger* log);

	static bool getRqCtrlSettings(const Hardware::Software* software,
								  const Hardware::DeviceController* rqCtrl,
								  const std::vector<quint32>& rcsPropsFlags,		// array of RcCtrlSettings::RCS_* constants
								  RqCtrlSettings* rcSettings,
								  Builder::IssueLogger* log);

	static bool getRqControllersSettings(const Hardware::Software* software,
								  const std::vector<quint32>& rcsPropsFlags,		// array of RcCtrlSettings::RCS_* constants
								  std::vector<RqCtrlSettings>* rcSettings,
								  Builder::IssueLogger* log);

	static bool isRqCtrlExist(const QString& rqCtrlEquipmentID, const std::vector<RqCtrlSettings>& rcSettings);

	static bool getAppDataServices(const Hardware::EquipmentSet* equipment,
								   const Hardware::Software* software,
								   const QStringList& appDataServiceIds,
								   std::map<QString, const Hardware::Software*>* appDataServices,
								   Builder::IssueLogger* log);
};


// SW_ENDPOINT is any of SoftwareEndpoint:: types except SoftwareEndpoint::TuningService
// that not have 'address' field
//
template <typename SW_ENDPOINT>
bool getSoftwareConnectionsBySoftwareIDs(const Hardware::EquipmentSet* equipment,
									const Hardware::Software* thisSoftware,
									const QString& propSoftwareIDs,				// in thisSoftware
									int maxIDsCount,
									bool emptyAllowed,
									E::SoftwareType requiredSoftwareType,
									const QString& propIP,						// in connected software
									const QString& propPort,					// in connected software
									std::vector<SW_ENDPOINT>* connections,			// on return connections->size() always == maxIDsCount
									Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(connections, log);

	connections->clear();
	connections->resize(maxIDsCount);

	bool result = true;

	QStringList softwareIDs;

	result &= DeviceHelper::getStrListProperty(thisSoftware, propSoftwareIDs,
											   &softwareIDs, log);

	RETURN_IF_FALSE(result);

	if (softwareIDs.isEmpty() == true)
	{
		if (emptyAllowed == false)
		{
			//  Property %1.%2 is empty.
			//
			log->errCFG3022(thisSoftware->equipmentIdTemplate(), propSoftwareIDs);

			return false;
		}

		return true;
	}

	if (softwareIDs.size() > maxIDsCount)
	{
		// Property %1.%2 contains more than %3 software identifier(s).
		//
		log->errCFG3050(thisSoftware->equipmentIdTemplate(), propSoftwareIDs, maxIDsCount);
		return false;
	}

	for(int i = 0; i < softwareIDs.size(); i++)
	{
		const QString& softwareID = softwareIDs[i];
		SW_ENDPOINT& endpoint = connections->at(i);

		endpoint.equipmentId = softwareID;

		result &= SoftwareSettingsGetter::getSoftwareConnectionBySoftwareID(
													equipment, thisSoftware, softwareID,
													propSoftwareIDs,
													propIP,
													propPort,
													&endpoint.address, true, "", 0,
													requiredSoftwareType, log);
	}

	return result;
}

#pragma warning(push)
#pragma warning(disable: 4250)

// -------------------------------------------------------------------------------------------

class CfgServiceSettingsGetter : public CfgServiceSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;

	bool buildClientsList(const Builder::Context* context, const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class AppDataServiceSettingsGetter : public AppDataServiceSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class DiagDataServiceSettingsGetter : public DiagDataServiceSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class TuningServiceSettingsGetter : public TuningServiceSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;
	bool fillTuningSourcesInfo(const Builder::Context* context,
							   int channel);

	bool fillTuningClientsInfo(const Builder::Context* context,
							   const Hardware::Software* software,
							   bool singleLmControlEnabled);

	bool fillMatsUsers(const Builder::Context* context);
};

// -------------------------------------------------------------------------------------------

class ArchivingServiceSettingsGetter : public ArchivingServiceSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;

	bool checkSettings(const Hardware::Software* software, Builder::IssueLogger* log);
};

// -------------------------------------------------------------------------------------------

class TestClientSettingsGetter : public TestClientSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class MetrologySettingsGetter : public MetrologySettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class MonitorSettingsGetter : public MonitorSettings,
							  public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context, const Hardware::Software* software) override;

	bool readAppDataServiceAndArchiveSettings(const Builder::Context* context, const Hardware::Software* software);

	bool readTuningServiceSettings(const Builder::Context* context, const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class AdsBridgeSettingsGetter : public AdsBridgeSettings,
								public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context, const Hardware::Software* software) override;
	bool readAppDataServiceSettings(const Builder::Context* context, const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class DiagnosticsSettingsGetter : public DiagnosticsSettings,
								  public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
					  const Hardware::Software* software) override;

	bool readDiagDataServiceAndArchiveSettings(const Builder::Context* context,
											   const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class TuningClientSettingsGetter : public TuningClientSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class TestSuiteSettingsGetter : public TestSuiteSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
						const Hardware::Software* software) override;

	bool readAppDataServiceAndArchiveSettings(const Builder::Context* context,
											  const Hardware::Software* software);

	bool readTuningServiceSettings(const Builder::Context* context,
											  const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class GatewayServiceSettingsGetter : public GatewayServiceSettings, public SoftwareSettingsGetter
{
private:
	bool readSettings(const Builder::Context* context,
					const Hardware::Software* software) override;
};

#pragma warning(pop)







