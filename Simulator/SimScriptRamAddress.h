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
		Q_INVOKABLE RamAddress() = default;
		Q_INVOKABLE RamAddress(const RamAddress&) = default;
		Q_INVOKABLE RamAddress(const Address16& addr16);
		Q_INVOKABLE RamAddress(quint32 offset, quint32 bit);

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

		quint32 bitAddress() const;

	private:
		quint32 m_offset = BadAddress;
		quint32 m_bit = BadAddress;
	};

}

Q_DECLARE_METATYPE(Sim::RamAddress);

