#ifndef TRENDSIGNAL_H
#define TRENDSIGNAL_H

#include "../lib/Types.h"

namespace TrendLib
{

	class TrendSignal
	{
	public:
		TrendSignal();

		// Proprties
		//
	public:
		QString signalId() const;
		void setSignalId(const QString& value);

		QString caption() const;
		void setCaption(const QString& value);

		QString equipmnetId() const;
		void setEquipmnetId(const QString& value);

		bool isAnalog() const;
		bool isDiscrete() const;
		E::SignalType type() const;
		void setType(E::SignalType value);

		double lowLimit() const;
		void setLowLimit(double value);

		double highLimit() const;
		void setHighLimit(double value);

		QString unit() const;
		void setUnit(const QString& value);

		// Data
		//
	private:
		QString m_signalId;
		QString m_caption;
		QString m_equipmentId;

		E::SignalType m_type = E::SignalType::Analog;

		double m_lowLimit = 0;
		double m_highLimit = 1.0;

		QString m_unit;
	};

}

#endif // TRENDSIGNAL_H