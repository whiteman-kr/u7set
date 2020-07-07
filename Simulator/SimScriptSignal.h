#pragma once
#include "../lib/Signal.h"
#include "SimScriptRamAddress.h"

namespace Sim
{

	class ScriptSignal
	{
		Q_GADGET

		Q_PROPERTY(bool isNull READ isNull)

		Q_PROPERTY(QString appSignalID READ (m_signal.appSignalID))
		Q_PROPERTY(QString customAppSignalID READ (m_signal.customAppSignalID))

		Q_PROPERTY(int sizeW READ (m_signal.sizeW))
		Q_PROPERTY(int sizeBit READ (m_signal.sizeBit))

		Q_PROPERTY(bool isAcquired READ (m_signal.isAcquired))
		Q_PROPERTY(bool isArchived READ (m_signal.isArchived))

		Q_PROPERTY(RamAddress ualAddr READ (m_signal.ualAddr))
		Q_PROPERTY(RamAddress ioBufAddr READ (m_signal.ioBufAddr))
		Q_PROPERTY(RamAddress tuningAddr READ (m_signal.tuningAddr))
		Q_PROPERTY(RamAddress tuningAbsAddr READ (m_signal.tuningAbsAddr))
		Q_PROPERTY(RamAddress regBufAddr READ (m_signal.regBufAddr))
		Q_PROPERTY(RamAddress regValueAddr READ (m_signal.regValueAddr))
		Q_PROPERTY(RamAddress regValidityAddr READ (m_signal.regValidityAddr))

	public:
		ScriptSignal() = default;
		ScriptSignal(const Signal& s);
		~ScriptSignal() = default;

		bool isNull() const;

		void setSignal(const Signal& s);

	private:
		Signal m_signal;
	};

}

Q_DECLARE_METATYPE(Sim::ScriptSignal);

