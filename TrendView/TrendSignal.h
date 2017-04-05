#ifndef TRENDSIGNAL_H
#define TRENDSIGNAL_H

#include "../lib/Types.h"

namespace TrendLib
{

	class TrendSignal
	{
	public:
		TrendSignal();

	private:
		QString m_id;
		QString m_caption;
		QString m_equipmentId;

		E::SignalType m_type = E::SignalType::Analog;

	};

}

#endif // TRENDSIGNAL_H
