#pragma once
#include "SimScopedLog.h"
#include "../lib/LanControllerInfo.h"

namespace Sim
{
	class Lans;
	class Simulator;

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
		QString lmEquipmentId() const;
		QString portEquipmentId() const;

		bool enabled() const;
		void setEnabled(bool value);

		bool isTuning() const;
		bool isAppData() const;
		int appDataSizeBytes() const;
		bool isDiagData() const;

	protected:
		Lans* m_lans = nullptr;
		mutable ScopedLog m_log;
		std::atomic<bool> m_enabled{true};			// Allow AppData trasmittion to AppDataSrv, TuningService, DiagService

		::LanControllerInfo m_lanControllerInfo;
	};


}

