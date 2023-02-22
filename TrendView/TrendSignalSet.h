#pragma once
#include <map>
#include "TrendArchiveServer.h"
#include "TrendSignal.h"
#include "ITrendDataProvider.h"


namespace Proto
{
	class TrendStateRecord;
	class TrendArchiveHour;
	class TrendArchive;
	class TrendSignalSet;
}



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
			*this = TrendStateItem_v1{};
		}

		[[nodiscard]] bool isValid() const
		{
			return (flags & 0x00000002);	// AppSignalStateFlags::stateAvailable, it is a real non valid
											// validity bit 0 is a combination of stateAvailable and coupled valididty signal
											// So
		}

		void setValid(bool valid)
		{
			if (valid == true)
			{
				flags |= 0x00000003;		// AppSignalStateFlags::stateAvailable && AppSignalStateFlags::valid
			}
			else
			{
				flags &= ~0x00000003;		// AppSignalStateFlags::stateAvailable && AppSignalStateFlags::valid
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
		static const size_t RecomendedSize = 3200;			// TrendStateItem is about 36-40 bytes, 1600 is about 64KB, 3200 is about 128KB

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
		std::list<RealtimeDataChunk> signalData;	// Each item is a signal with the vector of states
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

		TrendArchive(TrendSignalPlusServerId trendSignalPlusServerId_) :
			trendSignalPlusServerId(std::move(trendSignalPlusServerId_))
		{}

		TrendArchive(QString appSignalId, QString archiveServerId) :
			trendSignalPlusServerId{.appSignalId = std::move(appSignalId),
									.archiveServerId = std::move(archiveServerId)}
		{}

		TrendSignalPlusServerId trendSignalPlusServerId;
		std::map<TimeStamp, std::shared_ptr<OneHourData>> m_hours;		// Key is rounded to hour (like 9:00, 14:00, ...)
																		// DO NOT CHANGE type to unordered_map, as it is suppose to be ordered
		QString realTimeActiveServiceId;		// Current active realtime service id, resets to "" when non vlid
												// points arrived, and set to the new value for service with valid points
		QElapsedTimer serviceUpdateTimer;		// If active service was not update too long, then switch to another server,
												// This can happen when server shutdown process does not send non-valid point

		// Serialization
		//
		bool save(TrendSignalPlusServerId trendSignalPlusServerId, Proto::TrendArchive* message) const;
		bool load(const Proto::TrendArchive& message);
	};


	class TrendSignalSet : public QObject, public ITrendDataProvider
	{
		Q_OBJECT

	public:
		TrendSignalSet();

	public:
		bool save(::Proto::TrendSignalSet* message) const;
		bool load(const ::Proto::TrendSignalSet& message);

	public:
		bool addSignal(const TrendSignalParam& signal);
		bool addSignals(std::list<TrendSignalParam>&& signalParams);

		void removeSignal(const TrendLib::TrendSignalParam& signal);
		void removeAllSignals();

		[[nodiscard]] TrendLib::TrendSignalParam signalParam(const QString& appSignalId, const QString& archiveServerId, bool* ok) const;
		bool setSignalParam(const TrendLib::TrendSignalParam& signalParam);		// Update data

		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> trendSignals() const;
		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> analogSignals() const;
		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> discreteSignals() const;

		[[nodiscard]] std::vector<Hash> trendSignalsHashes(const QString& equipmentId = QString()) const;
		[[nodiscard]] QStringList trendSignalIds() const;

		[[nodiscard]] int discretesSignalsCount() const;
		[[nodiscard]] int analogSignalsCount() const;

		bool getFullExistingTrendData(const TrendSignalParam& trendSignal, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const;
		bool getExistingTrendData(const TrendSignalParam& trendSignal, QDateTime from, QDateTime to, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const;

		// ITrendDataProvider implementation
		//
		virtual bool trendData(QUuid trendUuid,
							   const TrendSignalParam& trendSignal,
							   QDateTime from,
							   QDateTime to,
							   E::TimeType timeType,
							   std::list<std::shared_ptr<OneHourData>>* outData) const override;

		virtual TimeStamp maxTimeStamp(QUuid trendUuid, E::TimeType timeType) const override;

		// End of ITrendDataProvider
		//
		bool addTrendPoint(const TrendSignalParam& signal, E::TimeType timeType, TrendStateItem stateItem);
		bool removeTrendPoint(const TrendSignalParam& signal, int index, E::TimeType timeType);

		void clear(E::TimeType timeType);

		/// Clear all rahive data where request was not processed yet or there is not data.
		/// This function is used when the new configuration is arriving, so the request can be sent again
		///
		void clearArchiveWithouthRecord();

		// Add non valid points to all signals, useful in switching mode Archive/RealTime
		//
	public slots:
		void addNonValidPoint();
		void addNonValidPoint(E::TimeType timeType);
		static void addNonValidPoint(TrendArchive* archive);

		// --
		//
		void slot_archiveDataReceived(TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
		void slot_archiveRequestError(TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType);

		void slot_realtimeDataReceived(QString sourceEquipmentId,
									   std::shared_ptr<TrendLib::RealtimeData> data,
									   TrendLib::TrendStateItem minState,
									   TrendLib::TrendStateItem maxState);
		void slot_realtimeRequestError(QString errorText);
		void slot_realtimeConnectionLost(QString sourceEquipmentId);

	private:
		void appendRealtimeDataToArchive(QString sourceEquipmentId,
										 E::TimeType timeType,
										 Hash signalhash,
										 const std::vector<TrendStateItem>& states);

	signals:
		void requestData(TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp hourToRequest, E::TimeType timeType) const;

	private:
		mutable QMutex m_paramMutex;
		std::list<TrendSignalParam> m_signalParams;

		mutable QMutex m_archiveMutex;
		mutable std::map<TrendSignalPlusServerId, TrendArchive> m_archiveLocalTime;		// Key is "AppSignalID@ArchiveServerID", Example: #ABC01@USB_SHK_WS00_ARCHSRV
		mutable std::map<TrendSignalPlusServerId, TrendArchive> m_archiveSystemTime;
		mutable std::map<TrendSignalPlusServerId, TrendArchive> m_archivePlantTime;
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendStateItem)
Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::OneHourData>)
Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::RealtimeData>)
