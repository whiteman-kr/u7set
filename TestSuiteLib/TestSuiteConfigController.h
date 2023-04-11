#pragma once

#include "../ClientLib/ConfigController.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/ILogFile.h"
#include "TestScriptsStorage.h"

namespace TestSuite
{
	struct ConfigSettings
	{
		int configurationId = -1;

		bool isValid() { return configurationId != -1; }

		ClientLib::ConfigurationInfo configInfo;

		int buildNo = -1;
		QString softwareEquipmentId;
		QString project;

		// AppData settings
		//
		std::vector<SoftwareEndpoint::AppDataService> appDataServices;

		// Tuning settings
		//
		bool tuningEnabled = false;
		std::vector<SoftwareEndpoint::TuningService> tuningServices;

		// Scripts list
		//
		QStringList scriptFiles;
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
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const ::TestSuiteSettings& settings, const BuildFileInfoArray& files) override;

		void dump(const ConfigSettings& conf) const;

	signals:
		void configrationError();
		void configurationArrived(ConfigSettings configuration);

	public:
		ConfigSettings configuration() const;
		ClientLib::ConfigurationInfo configInfo() const;

		std::vector<TestSuite::TestScript> scripts() const;

		// Data section
		//
	private:
		inline static int s_configurationIdCounter = 0;

		mutable QReadWriteLock m_confugurationLock;		// for access to m_configuration and m_scripts
		ConfigSettings m_configuration;

		std::vector<TestScript> m_scripts;
	};
}
