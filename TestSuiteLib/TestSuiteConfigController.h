#pragma once
#include <QReadWriteLock>
#include <ReportLib/ReportTemplate.h>
#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/MatsUsers.h"
#include "../UtilsLib/ILogFile.h"
#include "TestScriptsStorage.h"
#include "TestReport.h"

#include <ClientLib/ConfigController.h>

namespace TestSuite
{
	struct ConfigData
	{
		std::vector<TestScript> scripts;
		ReportLib::ReportTemplateStorage reportTemplates;
	};


	struct ConfigSettings
	{
		int configurationId = -1;

		bool isValid() { return configurationId != -1; }

		ClientLib::ConfigurationInfo configInfo;

		// AppData settings
		//
		std::vector<SoftwareEndpoint::AppDataService> appDataServices;

		// Tuning settings
		//
		bool tuningEnabled = false;
		std::vector<SoftwareEndpoint::TuningService> tuningServices;
		QByteArray tuningSignalsFile;

		// Security
		//
		bool login = false;
		QStringList userAccounts;
		OnlineLib::MatsUserStorage matsUsers;

		// Scripts list
		//
		QStringList scriptFiles;
		QString scriptTags;

		// Reports settings
		//
		QString plant;
		QString unit;
		QString system;
	};


	class TestSuiteConfigController : public ClientLib::ConfigController
	{
		Q_OBJECT

	public:
		TestSuiteConfigController() = delete;

		TestSuiteConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* appLogFile);
		virtual ~TestSuiteConfigController() = default;

	protected:
		/// This function is called when the new configuarion arrives, it is overrided to get specific Monitor
		/// configuration, after it signal `configurationArrived` is emitted
		///
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const ::TestSuiteSettings& settings, const std::vector<OnlineLib::BuildFileInfo>& files) override;

		void dump(const ConfigSettings& conf) const;

	signals:
		void configrationError();
		void configurationArrived(ConfigSettings configuration);

	public:
		ConfigSettings configuration() const;
		ClientLib::ConfigurationInfo configInfo() const;

		bool configurationTuningEnabled() const;

		ConfigData configData() const;

		// Data section
		//
	private:
		inline static int s_configurationIdCounter = 0;

		mutable QReadWriteLock m_confugurationLock;		// for access to m_configuration and m_scripts
		ConfigSettings m_configuration;

		ConfigData m_configData;
	};
}
