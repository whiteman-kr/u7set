#pragma once

#include "../lib/LmDescription.h"
#include "SimScriptRamAddress.h"

namespace Sim
{
	class ScriptLmDescription
	{
		Q_GADGET

		Q_PROPERTY(bool isNull READ isNull)

	public:
		ScriptLmDescription() = default;
		~ScriptLmDescription() = default;

		void setLmDescription(const LmDescription& lmDesc);

		bool isNull() const;

	public slots:

		bool isAddrInIoModuleBuf(int modulePlace, RamAddress addr) const;

		RamAddress ioModuleBufStartAddr(int modulePlace) const;
		int ioModuleBufSize() const;

	private:
		LmDescription m_lmDesc;
	};
}

Q_DECLARE_METATYPE(Sim::ScriptLmDescription);
