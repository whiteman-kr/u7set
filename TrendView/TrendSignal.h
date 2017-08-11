#ifndef TRENDSIGNAL_H
#define TRENDSIGNAL_H

#include <array>
#include <bitset>
#include <memory>
#include "../lib/Types.h"
#include "../lib/AppSignal.h"

namespace Proto
{
	class TrendStateItem;
	class TrendStateRecord;
	class TrendArchiveHour;
	class TrendArchive;
	class TrendSignalParam;
	class TrendSignalSet;
}

namespace TrendLib
{
	struct TrendStateItem
	{
		TimeStamp system;
		TimeStamp local;
		TimeStamp plant;
		qint64 archiveIndex;
		qint32 flags;
		double value;

		TrendStateItem() = default;
		TrendStateItem(const AppSignalState& state) :
			system(state.m_time.system),
			local(state.m_time.local),
			plant(state.m_time.plant),
			flags(state.m_flags.all),
			value(state.m_value)
		{
		}

		bool isValid() const
		{
			return (flags & 0x000001);
		}

		const TimeStamp& getTime(const TimeType& timeType) const
		{
			switch (timeType)
			{
			case TimeType::Local:	return this->local;
			case TimeType::System:	return this->system;
			case TimeType::Plant:	return this->plant;
			default:
				assert(false);
				return this->local;
			}
		}

		// Serialization
		//
		bool save(Proto::TrendStateItem* message) const;
		bool load(const Proto::TrendStateItem& message);
	};

	struct TrendStateRecord
	{
		std::vector<TrendStateItem> states;
		static const size_t recomendedSize = 1024;

		// Serialization
		//
		bool save(Proto::TrendStateRecord* message) const;
		bool load(const Proto::TrendStateRecord& message);
	};

	struct OneHourData
	{
		enum class State
		{
			NoData,
			Requested,
			Received
		};

		State state = State::NoData;
		std::vector<TrendStateRecord> data;

		// Serialization
		//
		bool save(const TimeStamp& timeStamp, Proto::TrendArchiveHour* message) const;
		bool load(const Proto::TrendArchiveHour& message);
	};

	struct TrendArchive
	{
		QString appSignalId;		// This fiels is not filled in. Don't use it now
		std::map<TimeStamp, std::shared_ptr<OneHourData>> m_hours;			// Key is rounded to hour (like 9:00, 14:00, ...)

		// Serialization
		//
		bool save(QString mapAppSignalId, Proto::TrendArchive* message) const;
		bool load(const Proto::TrendArchive& message);
	};

	class TrendSignalParam
	{
	public:
		TrendSignalParam();
		TrendSignalParam(const AppSignalParam& appSignal);

	public:
		bool save(Proto::TrendSignalParam* message) const;
		bool load(const Proto::TrendSignalParam& message);

		// Methods
		//
	public:
		AppSignalParam toAppSignalParam() const;

		// Properties
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

		QString unit() const;
		void setUnit(const QString& value);

		double highLimit() const;
		void setHighLimit(double value);

		double lowLimit() const;
		void setLowLimit(double value);

		double viewHighLimit() const;
		void setViewHighLimit(double value);

		double viewLowLimit() const;
		void setViewLowLimit(double value);

		QColor color() const;
		void setColor(const QColor& value);

		// Temporary variables properties
		//
		int tempSignalIndex() const;
		void setTempSignalIndex(int value);

		QRectF tempDrawRect() const;
		void setTempDrawRect(const QRectF& value);

		// Data
		//
	private:
		QString m_signalId;				// CustomSignalID
		QString m_appSignalId;			// AppSignalID, starts from # for app data
		QString m_caption;
		QString m_equipmentId;

		E::SignalType m_type = E::SignalType::Analog;
		QString m_unit;

		double m_highLimit = 1.0;
		double m_lowLimit = 0;

		double m_viewHighLimit = 1.0;	// Current view limits for the signals
		double m_viewLowLimit = 0;

		QColor m_color = qRgb(0, 0, 0);

		// Temporary variables used in drawing
		//
	private:
		int m_tempSignalIndex;		// Signal index, separate for disrctes and analogs, filled in getting signal list in TrendSignalSet::analogSignals/discreteSignals
		QRectF m_tempDrawRect;		// Draw signal area
	};

	class TrendSignalSet : public QObject
	{
		Q_OBJECT

	public:
		TrendSignalSet();

	public:
		bool save(::Proto::TrendSignalSet* message) const;
		bool load(const ::Proto::TrendSignalSet& message);

	public:
		bool addSignal(const TrendSignalParam& signal);
		void removeSignal(QString appSignalId);

		TrendLib::TrendSignalParam signalParam(const QString& appSignalId, bool* ok) const;
		bool setSignalParam(const TrendLib::TrendSignalParam& signalParam);		// Update data

		std::vector<TrendLib::TrendSignalParam> trendSignals() const;
		std::vector<TrendLib::TrendSignalParam> analogSignals() const;
		std::vector<TrendLib::TrendSignalParam> discreteSignals() const;

		size_t analogSignalsCount() const;

		bool getExistingTrendData(QString appSignalId, QDateTime from, QDateTime to, TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const;
		bool getTrendData(QString appSignalId, QDateTime from, QDateTime to, TimeType timeType, std::list<std::shared_ptr<OneHourData> >* outData) const;

		void clear(TimeType timeType);

	public slots:
		void slot_dataReceived(QString appSignalId, TimeStamp requestedHour, TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
		void slot_requestError(QString appSignalId, TimeStamp requestedHour, TimeType timeType);

	signals:
		void requestData(QString appSignalId, TimeStamp hourToRequest, TimeType timeType) const;

	private:
		mutable QMutex m_paramMutex;
		std::list<TrendSignalParam> m_signalParams;

		mutable QMutex m_archiveMutex;
		mutable std::map<QString, TrendArchive> m_archiveLocalTime;
		mutable std::map<QString, TrendArchive> m_archiveSystemTime;
		mutable std::map<QString, TrendArchive> m_archivePlantTime;
	};
}

Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::OneHourData>)

#endif // TRENDSIGNAL_H
