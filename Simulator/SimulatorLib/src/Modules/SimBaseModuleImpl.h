#pragma once

#include "../SimEeprom.h"
#include "../SimScopedLog.h"

#include <HardwareLib/LmDescription.h>
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	class SimulatorPrivate;

	//
	// class BaseModuleImpl
	//
	class BaseModuleImpl : public QObject
	{
		Q_OBJECT

	public:
		explicit BaseModuleImpl(SimulatorPrivate* simulator);
		virtual ~BaseModuleImpl();

	protected:
		bool loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom);

	public:
		const QString& equipmentId() const;

		QString subsystemId() const;
		int lmNumber() const;
		E::Channel channel() const;

		const Hardware::LogicModuleInfo& logicModuleInfo() const;

		LmDescription& lmDescription();
		const LmDescription& lmDescription() const;

	protected:
		SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;
	};
} // namespace Sim
