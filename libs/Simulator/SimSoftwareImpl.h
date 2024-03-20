#pragma once
#include "SoftwareXmlReader.h"
#include "SimAppDataTransmitter.h"
#include "SimTuningServiceCommunicator.h"

#include <Behavior/MonitorBehavior.h>

namespace Sim
{
	class SimulatorPrivate;
	class Application;
	class AppMonitorImpl;


	class SoftwareImpl
	{
	public:
		explicit SoftwareImpl(SimulatorPrivate* simulator);

	public:
		void clear();
		bool load(QString buildPath);
		bool loadSoftwareXml(QString buildPath);

		bool startSimulation(QString profileName);
		bool stopSimulation();

		// Monitor
		//
		QStringList monitors() const;		// Returns EquipmentIDs of all Monitors
		std::shared_ptr<AppMonitorImpl> monitor(QString equipmentId) const;

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
		SimulatorPrivate* m_simulator = nullptr;
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

		virtual bool load(QString appDir, ILogFile* log);

	public:
		const QString& equipmentId() const;
		E::SoftwareType softwareType() const;

		const SoftwareXmlInfo& info() const;

	private:
		SoftwareXmlInfo m_info;
	};


	class AppMonitorImpl : public Application
	{
	public:
		AppMonitorImpl(const SoftwareXmlInfo& info);
		~AppMonitorImpl() = default;

		virtual bool load(QString appDir, ILogFile* log) override;

	public:
		QString globalScript() const;
		const Behavior::MonitorBehavior& monitorBehavior() const;

	private:
		QString m_globalScript;
		Behavior::MonitorBehavior m_monitorBehavior;
	};


	template<typename T>
	std::shared_ptr<const T> SoftwareImpl::getSettingsProfile(const QString& softwareEquipmentID, const QString& profile) const
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


