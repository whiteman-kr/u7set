#pragma once
#include "SimScriptSignal.h"


namespace Sim
{
	class Simulator;


	class ScriptDevUtils
	{
		Q_GADGET

	public:
		ScriptDevUtils();
		ScriptDevUtils(Simulator* simulator);

	public:
		Q_INVOKABLE bool addrInIoModuleBuf(QString lmEquipmentId, quint32 modulePlace, RamAddress addr) const;
		Q_INVOKABLE bool addrInRegBuf(QString lmEquipmentId, RamAddress addr) const;
		Q_INVOKABLE quint32 regBufStartAddr(QString lmEquipmentId) const;

	private:
		Simulator* m_simulator = nullptr;
	};

}

Q_DECLARE_METATYPE(Sim::ScriptDevUtils);
