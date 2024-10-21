#pragma once
#include "SimScriptRamAddress.h"

namespace Sim
{
	class SimulatorPrivate;

	class ScriptDevUtils
	{
		Q_GADGET

	public:
		ScriptDevUtils();
		ScriptDevUtils(SimulatorPrivate* simulator);

	public:
		Q_INVOKABLE bool addrInIoModuleBuf(QString lmEquipmentId, quint32 modulePlace, RamAddress addr) const;
		Q_INVOKABLE bool addrInRegBuf(QString lmEquipmentId, RamAddress addr) const;
		Q_INVOKABLE quint32 regBufStartAddr(QString lmEquipmentId) const;

		Q_INVOKABLE quint32 reverseUnsignedInt32(quint32 v) const;
		Q_INVOKABLE qint32 reverseSignedInt32(quint32 v) const;
		Q_INVOKABLE quint16 reverseUnsignedInt16(quint16 v) const;
		Q_INVOKABLE qint16 reverseSignedInt16(quint16 v) const;
		Q_INVOKABLE float reverseFloat(float v) const;

	private:
		SimulatorPrivate* m_simulator = nullptr;
	};

}

Q_DECLARE_METATYPE(Sim::ScriptDevUtils);
