#ifndef TRENDSIGNAL_H
#define TRENDSIGNAL_H

#include <array>
#include <bitset>
#include "../lib/Types.h"
#include "../lib/AppSignal.h"

namespace TrendLib
{
	struct TrendStateItem
	{
		qint64	system;
		qint64 local;
		qint64 lant;
		qint32 flags;
		double value;
	};

	struct TrendStateRecord
	{
		std::array<TrendStateItem, 512>  states;
	};

	struct OneHourData
	{
		bool requested = false;
		std::vector<TrendStateRecord> data;
	};

	struct OneDayData
	{
		QDate date;
		std::array<OneHourData, 24> hours;
	};

	class TrendSignalParam
	{
	public:
		TrendSignalParam();
		TrendSignalParam(const AppSignalParam& appSignal);

		// Proprties
		//
	public:
		QString signalId() const;
		void setSignalId(const QString& value);

		QString appSignalId() const;
		void setAppSignalId(const QString& value);

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
		QString m_signalId;			// CustomSignalID
		QString m_appSignalId;		// AppSignalID, starts from # for app data
		QString m_caption;
		QString m_equipmentId;

		E::SignalType m_type = E::SignalType::Analog;

		double m_lowLimit = 0;
		double m_highLimit = 1.0;
		QString m_unit;

		QColor m_color = qRgb(0, 0, 0);

		std::list<OneDayData> m_days;			// Archive data
	};

	class TrendSignalSet
	{
	public:
		TrendSignalSet();

		bool addSignal(const TrendSignalParam& signal);
		void removeSignal(QString appSignalId);

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
