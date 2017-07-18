#ifndef TRENDSIGNAL_H
#define TRENDSIGNAL_H

#include <array>
#include "../lib/Types.h"

namespace TrendLib
{
	struct TrendStateIten
	{
		qint64	system;
		qint64 local;
		qint64 lant;
		qint32 flags;
		double value;
	};

	struct TrendStateRecord
	{
		std::array<TrendStateIten, 512>  m_states;
	};

	class TrendSignalParam
	{
	public:
		TrendSignalParam();

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

		QColor color() const;
		void setColor(const QColor& value);

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

		QColor m_color = qRgb(0, 0, 0);

		std::vector<std::shared_ptr<TrendStateRecord>> m_records;
	};

	class TrendSignalSet
	{
	public:
		TrendSignalSet();

		bool addSignal(const TrendSignalParam& signal);
		void removeSignal(QString signalId);

		std::vector<TrendSignalParam> analogSignals() const;
		std::vector<TrendSignalParam> discreteSignals() const;

		TrendSignalParam signalParam() const;

	private:
		mutable QMutex m_paramMutex;
		std::list<TrendSignalParam> m_signalParams;

//		mutable QMutex m_stateMutex;
//		std::map<QString, TrendSignalSates> m_signalStates;
	};

}

#endif // TRENDSIGNAL_H
