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

using TrendColor = quint32;		// This is QRgb, the problem is this header is used buy othe libs without GUI (simulator)

namespace TrendLib
{
#pragma pack(push, 1)
	struct TrendStateItem_v1
	{
		qint64 system;
		qint64 local;
		qint64 plant;
		qint32 flags;
		double value;

		TrendStateItem_v1() = default;
		TrendStateItem_v1(const AppSignalState& state) :
			system(state.m_time.system.timeStamp),
			local(state.m_time.local.timeStamp),
			plant(state.m_time.plant.timeStamp),
			flags(state.m_flags.all),
			value(state.m_value)
		{
		}

		void clear()
		{
			system = 0;
			local = 0;
			plant = 0;
			flags = 0;
			value = 0;
		}

		[[nodiscard]] bool isValid() const
		{
			return (flags & 0x00000001);
		}

		void setValid(bool valid)
		{
			if (valid == true)
			{
				flags |= 0x00000001;
			}
			else
			{
				flags &= ~0x00000001;
			}
		}

		[[nodiscard]] bool isRealtimePoint() const
		{
			return (flags & 0x80000000) ? true : false;
		}

		void setRealtimePointFlag()
		{
			flags |= 0x80000000;
		}

		void resetRealtimePointFlag()
		{
			flags &= ~0x80000000;
		}

		[[nodiscard]] TimeStamp getTime(E::TimeType timeType) const
		{
			switch (timeType)
			{
			case E::TimeType::Local:	return TimeStamp{this->local};
			case E::TimeType::System:	return TimeStamp{this->system};
			case E::TimeType::Plant:	return TimeStamp{this->plant};
			default:
				assert(false);
				return TimeStamp{this->local};
			}
		}
	};
#pragma pack(pop)

	using TrendStateItem = TrendStateItem_v1;

	struct TrendStateRecord
	{
		std::vector<TrendStateItem> states;
		static const size_t RecomendedSize = 1600;			// TrendStateItem is about 36-40 bytes, 1600 is abou 64KB

		// Serialization
		//
		bool save(Proto::TrendStateRecord* message) const;
		bool load(const Proto::TrendStateRecord& message);
	};

	struct RealtimeDataChunk
	{
		Hash appSignalHash = UNDEFINED_HASH;
		std::vector<TrendStateItem> states;
	};

	struct RealtimeData
	{
		std::list<RealtimeDataChunk> signalData;	// Each item is a signal with vecro of states
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
		TrendArchive() = delete;
		TrendArchive(QString _appSignalId) : appSignalId(_appSignalId) {}

		QString appSignalId;
		std::map<TimeStamp, std::shared_ptr<OneHourData>> m_hours;		// Key is rounded to hour (like 9:00, 14:00, ...)
																		// DO NOT CHANGE type to unordered_map, as it is suppose to be ordered

		// Serialization
		//
		bool save(QString mapAppSignalId, Proto::TrendArchive* message) const;
		bool load(const Proto::TrendArchive& message);
	};

	struct TrendViewLimits
	{
		double highLimit = 1;
		double lowLimit = 0;
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
		[[nodiscard]] AppSignalParam toAppSignalParam() const;

		// Properties
		//
	public:
		[[nodiscard]] QString signalId() const;
		void setSignalId(const QString& value);

		[[nodiscard]] QString appSignalId() const;
		void setAppSignalId(const QString& value);

		[[nodiscard]] Hash appSignalHash() const;

		[[nodiscard]] QString caption() const;
		void setCaption(const QString& value);

		[[nodiscard]] QString equipmnetId() const;
		void setEquipmnetId(const QString& value);

		[[nodiscard]] bool isAnalog() const;
		[[nodiscard]] bool isDiscrete() const;
		[[nodiscard]] E::SignalType type() const;
		void setType(E::SignalType value);

		[[nodiscard]] QString unit() const;
		void setUnit(const QString& value);

		[[nodiscard]] E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat analogFormat);

		[[nodiscard]] int precision() const;
		void setPrecision(int value);

		[[nodiscard]] double lineWeight() const;
		void setLineWeight(double value);

		[[nodiscard]] double highLimit() const;
		void setHighLimit(double value);

		[[nodiscard]] double lowLimit() const;
		void setLowLimit(double value);

		[[nodiscard]] double viewHighLimit(E::TrendScaleType scaleType) const;
		void setViewHighLimit(E::TrendScaleType scaleType, double value);

		[[nodiscard]] double viewLowLimit(E::TrendScaleType scaleType) const;
		void setViewLowLimit(E::TrendScaleType scaleType, double value);

		[[nodiscard]] TrendColor color() const;
		void setColor(const TrendColor& value);

		// Temporary variables properties
		//
		[[nodiscard]] int tempSignalIndex() const;
		void setTempSignalIndex(int value);

		[[nodiscard]] QRectF tempDrawRect() const;
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

		E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e;
		int m_precision = 0;

		double m_lineWeight = 0;		// 0 is cosmetic pen

		double m_highLimit = 1.0;
		double m_lowLimit = 0;

		std::map<E::TrendScaleType, TrendViewLimits> m_viewLimits; // Current view limits for signals for different scales

		TrendColor m_color = 0xFF000000;	// Black color

		// Temporary variables used in drawing
		//
	private:
		int m_tempSignalIndex = -1;	// Signal index, separate for disrctes and analogs, filled in getting signal list in TrendSignalSet::analogSignals/discreteSignals
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

		[[nodiscard]] TrendLib::TrendSignalParam signalParam(const QString& appSignalId, bool* ok) const;
		bool setSignalParam(const TrendLib::TrendSignalParam& signalParam);		// Update data

		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> trendSignals() const;
		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> analogSignals() const;
		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> discreteSignals() const;

		[[nodiscard]] std::vector<Hash> trendSignalsHashes(const QString& equipmentId = QString()) const;

		[[nodiscard]] int discretesSignalsCount() const;
		[[nodiscard]] int analogSignalsCount() const;

		bool getFullExistingTrendData(QString appSignalId, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const;
		bool getExistingTrendData(QString appSignalId, QDateTime from, QDateTime to, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const;
		bool getTrendData(QString appSignalId, QDateTime from, QDateTime to, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const;

		bool addTrendPoint(QString appSignalId, E::TimeType timeType, TrendStateItem stateItem);
		bool removeTrendPoint(QString appSignalId, int index, E::TimeType timeType);

		void clear(E::TimeType timeType);

		void addNonValidPoint();	// Add non valid points to all signals, useful in switching mode Archive/RealTime

	private:
		void addNonValidPoint(E::TimeType timeType);

	public slots:
		void slot_archiveDataReceived(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
		void slot_archiveRequestError(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType);

		void slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
		void slot_realtimeRequestError(QString errorText);

	private:
		void appendRealtimeDataToArchive(E::TimeType timeType, Hash signalhash, const std::vector<TrendStateItem>& states);

	signals:
		void requestData(QString appSignalId, TimeStamp hourToRequest, E::TimeType timeType) const;

	private:
		mutable QMutex m_paramMutex;
		std::list<TrendSignalParam> m_signalParams;

		mutable QMutex m_archiveMutex;
		mutable std::map<Hash, TrendArchive> m_archiveLocalTime;		// Key is hash from appsignalid
		mutable std::map<Hash, TrendArchive> m_archiveSystemTime;
		mutable std::map<Hash, TrendArchive> m_archivePlantTime;
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendStateItem)
Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::OneHourData>)
Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::RealtimeData>)

#endif // TRENDSIGNAL_H
