#pragma once

namespace Sim
{
	struct OverrideRamRecord
	{
		quint16 mask = 0;
		quint16 data = 0;

		void overlapRecord(OverrideRamRecord r) noexcept
		{
			mask |= r.mask;
			data |= r.data;
		}

		void applyOverlapping(quint16* ptrW) const noexcept
		{
			assert(ptrW);
			*ptrW &= ~mask;
			*ptrW |= data;
		}

		auto operator<=>(const OverrideRamRecord&) const = default;
	};
} // namespace Sim