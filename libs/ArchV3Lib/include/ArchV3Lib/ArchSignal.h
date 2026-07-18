#pragma once

#include <ArchSignal.pb.h>
#include <CommonLib/Types.h>

namespace ArchV3
{
	struct ArchSignal
	{
		E::SignalType signalType;
		QString appSignalID;

		double lowLimit;
		double highLimit;
		
		double fineAperture;
		double coarseAperture;

		QString unit;

		void loadFromProto(const Proto::ArchSignal& ps)
		{ 
			signalType = static_cast<E::SignalType>(ps.signaltype());
			appSignalID = QString::fromStdString(ps.appsignalid());

			if (signalType == E::SignalType::Analog)
			{
				lowLimit = ps.lowlimit();
				highLimit = ps.highlimit();
				fineAperture = ps.fineaperture();
				coarseAperture = ps.coarseaperture();
				unit = QString::fromStdString(ps.unit());
			}
			else
			{
				lowLimit = 0;
				highLimit = 0;
				fineAperture = 0;
				coarseAperture = 0;
				unit.clear();
				unit.squeeze();
			}
		}
	};

} // namespace ArchV3