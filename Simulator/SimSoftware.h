#pragma once
#include "SoftwareXmlReader.h"
#include "SimAppDataTransmitter.h"
#include "SimTuningServiceCommunicator.h"

namespace  Sim
{
	class Simulator;
	class Application;
	class AppMonitor;


	class Software
	{
	public:
		explicit Software(Simulator* simulator);

	public:
		void clear();
		bool load(QString buildPath);
		bool loadSoftwareXml(QString buildPath);

		bool startSimulation(QString profileName);
		bool stopSimulation();

		// Monitor
		//
		QStringList monitors() const;		// Returns EquipmentIDs of all Monitors
		std::shared_ptr<AppMonitor> monitor(QString equipmentId) const;

		// AppDataService
		//
		bool sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp);

		// TuningService Communization
		//
		std::shared_ptr<Sim::TuningServiceCommunicator> tuningService(QString equipmentId) const;

		template<typename T>
		std::shared_ptr<const T> getSettingsProfile(const QString& softwareEquipmentID, const QString& profile) const;

	public:
		[[nodiscard]] bool enabled() const;
		void setEnabled(bool value);

		[[nodiscard]] Sim::AppDataTransmitter& appDataTransmitter();
		[[nodiscard]] const Sim::AppDataTransmitter& appDataTransmitter() const;

	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		std::vector<std::shared_ptr<Application>> m_software;

		std::atomic<bool> m_enabled{false};

		// Sends data to AppDataService
		//
		Sim::AppDataTransmitter m_appDataTransmitter;

		// TuningService Communicators, each instance works with its own TuningService
		// Key is TuningService equipmentId
		//
		std::map<QString, std::shared_ptr<Sim::TuningServiceCommunicator>> m_tuningServiceCommunicators;
		std::map<QString, std::shared_ptr<Sim::TuningServiceCommunicator>> m_tuningServiceControllers;
	};


	class Application
	{
	public:
		Application(const SoftwareXmlInfo& info);
		virtual ~Application() = default;

		virtual bool load(QString appDir);

	public:
		const QString& equipmentId() const;
		E::SoftwareType softwareType() const;

		const SoftwareXmlInfo& info() const;

	private:
		SoftwareXmlInfo m_info;
	};


	class AppMonitor : public Application
	{
	public:
		AppMonitor(const SoftwareXmlInfo& info);
		~AppMonitor() = default;

		virtual bool load(QString appDir) override;

	public:
		QString globalScript() const;

	private:
		QString m_globalScript;
	};


	template<typename T>
	std::shared_ptr<const T> Software::getSettingsProfile(const QString& softwareEquipmentID, const QString& profile) const
	{
		for(const auto& app : m_software)
		{
			if (app->equipmentId() == softwareEquipmentID)
			{
				return app->info().getSettingsProfile<T>(profile);
			}
		}

		return nullptr;
	}
}


