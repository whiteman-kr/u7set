#pragma once

#include "../Builder/Context.h"
#include "../Builder/IssueLogger.h"
#include "../TuningService/TuningSource.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../lib/DeviceHelper.h"

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
										const QString &defaultIP,
										int defaultPort,
										E::SoftwareType requiredSoftwareType,
										Builder::IssueLogger* log);

	static bool getSoftwareConnectionBySoftwareID(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* thisSoftware,
										const QString& connectedSoftwareID, const QString &propConnectedSoftwareID,
										const QString& propConnectedSoftwareIP,
										const QString& propConnectedSoftwarePort,
										HostAddressPort* connectedSoftwareIP,
										bool emptyAllowed,
										const QString &defaultIP,
										int defaultPort,
										E::SoftwareType requiredSoftwareType,
										Builder::IssueLogger* log);

	static bool getCfgServiceConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* software,
										QString* cfgServiceID1, HostAddressPort* cfgServiceAddrPort1,
										QString* cfgServiceID2, HostAddressPort* cfgServiceAddrPort2,
										Builder::IssueLogger* log);

	static bool getLmPropertiesFromDevice(const Hardware::DeviceModule* lm,
											E::LanControllerType lanControllerType,
											const Builder::Context* context,
											DataSource* ds);

	bool readSoftwareSettings(const Builder::Context* context,
							  const Hardware::Software* software);

	bool readFromDeviceByEquipmentID(const Builder::Context* context,
									const QString& softwareID,
									E::SoftwareType requiredSoftwareType = E::SoftwareType::Unknown);

protected:
	virtual bool readSettings(const Builder::Context* context,
								const Hardware::Software* software) = 0;
};

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

class MonitorSettingsGetter : public MonitorSettings, public SoftwareSettingsGetter
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

#pragma warning(pop)







