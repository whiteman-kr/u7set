#pragma once

#include "DeviceHelper.h"
#include "../Builder/Context.h"
#include "../Builder/IssueLogger.h"
#include "../TuningService/TuningSource.h"
#include "SoftwareSettings.h"

// -------------------------------------------------------------------------------------------

class SoftwareSettingsGetter
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

	virtual bool readFromDevice(const Builder::Context* context,
								const Hardware::Software* software) = 0;

	bool readFromDeviceByEquipmentID(const Builder::Context* context,
									const QString& softwareID,
									E::SoftwareType requiredSoftwareType = E::SoftwareType::Unknown);
};

// -------------------------------------------------------------------------------------------

class CfgServiceSettingsGetter : public CfgServiceSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;

private:
	bool buildClientsList(const Builder::Context* context, const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class AppDataServiceSettingsGetter : public AppDataServiceSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class DiagDataServiceSettingsGetter : public DiagDataServiceSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class TuningServiceSettingsGetter : public TuningServiceSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;
private:
	bool fillTuningSourcesInfo(const Builder::Context* context,
							   int channel);

	bool fillTuningClientsInfo(const Builder::Context* context,
							   const Hardware::Software* software,
							   bool singleLmControlEnabled);
};

// -------------------------------------------------------------------------------------------

class ArchivingServiceSettingsGetter : public ArchivingServiceSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;

private:
	bool checkSettings(const Hardware::Software* software, Builder::IssueLogger* log);
};

// -------------------------------------------------------------------------------------------

class TestClientSettingsGetter : public TestClientSettings, public SoftwareSettingsGetter
{
public:

	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class MetrologySettingsGetter : public MetrologySettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;
};

// -------------------------------------------------------------------------------------------

class MonitorSettingsGetter : public MonitorSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;


private:
	bool readAppDataServiceAndArchiveSettings(const Builder::Context* context,
											  const Hardware::Software* software);

	bool readTuningSettings(const Builder::Context* context,
							const Hardware::Software* software);
};

// -------------------------------------------------------------------------------------------

class TuningClientSettingsGetter : public TuningClientSettings, public SoftwareSettingsGetter
{
public:
	bool readFromDevice(const Builder::Context* context,
						const Hardware::Software* software) override;
};







