#pragma once
#include "../lib/Signal.h"

namespace Sim
{
	/*! \class RamAddress
		\ingroup simulator
		\brief Represents address in RAM of simulated module.
	*/
	class RamAddress
	{
		Q_GADGET

		/// \brief Offset in RAM, 0xFFFFFFFF means non valid address.
		Q_PROPERTY(quint32 offset READ offset WRITE setOffset)

		/// \brief Bit no for for discerete signals, 0xFFFFFFFF means non valid address.
		Q_PROPERTY(quint32 bit READ bit WRITE setBit)

		/// \brief Check address validity
		Q_PROPERTY(bool isValid READ isValid)

	public:
		RamAddress() = default;
		RamAddress(const RamAddress&) = default;
		RamAddress(const Address16& addr16);
		RamAddress(quint32 offset, quint32 bit);

		~RamAddress() = default;

	public:
		static const quint32 BadAddress = std::numeric_limits<quint32>::max();

	public:
		/// \brief Get address validity, returns true if the address is valid and can be used for RAM operations.
		bool isValid() const;

		quint32 offset() const;
		void setOffset(quint32 value);

		quint32 bit() const;
		void setBit(quint32 value);

		/// \brief Convert address to string representation.
		Q_INVOKABLE QString toString() const;

	private:
		quint32 m_offset = BadAddress;
		quint32 m_bit = BadAddress;
	};

}

Q_DECLARE_METATYPE(Sim::RamAddress);


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

		Q_PROPERTY(int isAcquired READ (m_signal.isAcquired))
		Q_PROPERTY(int isArchived READ (m_signal.isArchived))

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

		bool isNull() const;

		void setSignal(const Signal& s);

	private:
		Signal m_signal;
	};

}

Q_DECLARE_METATYPE(Sim::ScriptSignal);

