#pragma once

#include "IScriptProvider.h"
#include "TestScriptsStorage.h"

#include "../OnlineLib/MatsUsers.h"
#include "../UtilsLib/ILogFile.h"

#include <ClientLib/ConfigController.h>
#include <ReportLib/ReportTemplate.h>

#include <QReadWriteLock>

#include <optional>
#include <type_traits>
#include <vector>


namespace TestSuite
{
	struct ConfigData
	{
		std::vector<TestScript> scripts;
		ReportLib::ReportTemplateStorage reportTemplates;
	};

	static_assert(std::is_copy_assignable_v<ConfigData> == true);
	static_assert(std::is_copy_constructible_v<ConfigData> == true);
	static_assert(std::is_move_assignable_v<ConfigData> == true);
	static_assert(std::is_move_constructible_v<ConfigData> == true);


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
		QStringList scriptTags;

		// Reports settings
		//
		QString plant;
		QString unit;
		QString system;
	};

	static_assert(std::is_copy_assignable_v<ConfigSettings> == true);
	static_assert(std::is_copy_constructible_v<ConfigSettings> == true);
	static_assert(std::is_move_assignable_v<ConfigSettings> == true);
	static_assert(std::is_move_constructible_v<ConfigSettings> == true);


	class TestSuiteConfigController : public ::ClientLib::ConfigController,
									  public ::TestSuite::IScriptProvider
	{
		Q_OBJECT

	public:
		TestSuiteConfigController() = delete;

		TestSuiteConfigController(const SoftwareInfo& softwareInfo,
								  HostAddressPort address1,
								  HostAddressPort address2,
								  ILogFile* appLogFile);
		virtual ~TestSuiteConfigController() = default;

	protected:
		/// This function is called when the new configuarion arrives, it is overrided to get specific Monitor
		/// configuration, after it signal `configurationArrived` is emitted
		///
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf,
										 const ::TestSuiteSettings& settings,
										 const std::vector<OnlineLib::BuildFileInfo>& files) override;

		void dump(const ConfigSettings& conf) const;

		// --
		//
	signals:
		void configrationError();
		void configurationArrived(ConfigSettings configuration);

		// IScriptProvider interface
		//
	public:
		virtual QStringList getScriptFileNames() const override;
		virtual std::vector<TestScript> getScripts() const override;

		virtual std::optional<TestScript> getGloablScript() const override;
		virtual std::optional<TestScript> getScriptByFileName(const QString& fileName) const override;

		// End of IScriptProvider interface

	private:
		void clearData();
		void setData(ConfigSettings configuration, ConfigData configData);

	public:
		ConfigSettings configuration() const;
		ClientLib::ConfigurationInfo configInfo() const;

		bool configurationTuningEnabled() const;

		ConfigData configData() const;

		// Data section
		//
	private:
		inline static int s_configurationIdCounter = 0;

		mutable QReadWriteLock m_confugurationLock; // for access to m_configuration and m_scripts
		ConfigSettings m_configuration;
		ConfigData m_configData;
	};
} // namespace TestSuite
