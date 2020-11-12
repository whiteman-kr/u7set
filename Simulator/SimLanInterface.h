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

		virtual bool enabled() const;
		virtual void setEnabled(bool value);

		bool isTuning() const;
		bool isAppData() const;
		bool isDiagData() const;

	protected:
		Lans* m_lans = nullptr;
		mutable ScopedLog m_log;
		std::atomic<bool> m_enabled{false};			// Allow AppData trasmittion to AppDataSrv

		::LanControllerInfo m_lanControllerInfo;
	};


}

