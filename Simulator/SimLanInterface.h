#pragma once
#include "SimScopedLog.h"
#include "../lib/LanControllerInfo.h"

namespace Sim
{
	class Lans;
	class Simulator;

	class AppDataLanInterface;
	class DiagDataLanInterface;
	class TuningLanInterface;

	// The base class for:
	//		Sim::AppDataLanInterface
	//		Sim::DiagDataLanInterface
	//		Sim::TuningLanInterface
	//
	class LanInterface : public QObject
	{
		Q_OBJECT

	public:
		explicit LanInterface(const ::LanControllerInfo& lci, Lans* lans);
		virtual ~LanInterface();

	public:
		const QString& lmEquipmentId() const;
		const QString& portEquipmentId() const;

		bool enabled() const;
		void setEnabled(bool value);

		// Tuning
		//
		bool isTuning() const;

		TuningLanInterface* toTuningLanInterface();
		const TuningLanInterface* toTuningLanInterface() const;

		// AppData
		//
		bool isAppData() const;
		int appDataSizeBytes() const;

		// Diagnostics
		//
		bool isDiagData() const;

	protected:
		Lans* m_lans = nullptr;
		mutable ScopedLog m_log;
		std::atomic<bool> m_enabled{false};			// Allow/Disable AppData trasmittion to AppDataSrv, TuningService, DiagService

		::LanControllerInfo m_lanControllerInfo;
	};


}

