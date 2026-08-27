#pragma once

namespace Sim
{
	struct ModuleJumpers
	{
		QString subsystemId;
		uint16_t subsystemKey{};
		uint16_t lmNumber{};

		uint16_t jumpersValue{}; // Composed from subsystemKey and lmNumber,
								 // with CRC4 in upper 4 bits, see ConfigurationBuilder::jumpersCode().
	};
} // namespace Sim