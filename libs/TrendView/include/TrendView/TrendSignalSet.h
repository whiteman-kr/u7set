#pragma once
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include <QElapsedTimer>
#include <QMutex>

#include "ITrendDataProvider.h"
#include "TrendArchiveServer.h"
#include "TrendSignal.h"
#include "TrendSignalState.h"

namespace Proto
{
	class TrendArchive;
	class TrendSignalSet;
} // namespace Proto

namespace TrendLib
{
	struct OneHourData;

	struct TrendArchive
	{
		TrendArchive() = delete;

		TrendArchive(TrendSignalPlusServerId trendSignalPlusServerId_) :
			trendSignalPlusServerId(std::move(trendSignalPlusServerId_))
		{
		}

		TrendArchive(QString appSignalId, QString archiveServerId) :
			trendSignalPlusServerId{.appSignalId = std::move(appSignalId), .archiveServerId = std::move(archiveServerId)}
		{
		}

		TrendSignalPlusServerId trendSignalPlusServerId;
		std::map<TimeStamp, std::shared_ptr<OneHourData>> m_hours; // Key is rounded to hour (like 9:00, 14:00, ...)
																   // DO NOT CHANGE type to unordered_map, as it is suppose to be ordered
		QString realTimeActiveServiceId;                           // Current active real-time service id, resets to "" when non valid
																   // points arrived, and set to the new value for service with valid points
		QElapsedTimer serviceUpdateTimer; // If active service was not update too long, then switch to another server,
										  // This can happen when server shutdown process does not send non-valid point

		// Serialization
		//
		bool save(TrendSignalPlusServerId trendSignalPlusServerId, Proto::TrendArchive* message) const;
		bool load(const Proto::TrendArchive& message);
	};


	class TrendSignalSet : public QObject,
						   public ITrendDataProvider
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

		void reorderSignals(std::span<const TrendSignalParam> targetOrder);

		[[nodiscard]] TrendLib::TrendSignalParam signalParam(const QString& appSignalId, const QString& archiveServerId, bool* ok) const;
		bool setSignalParam(const TrendLib::TrendSignalParam& signalParam); // Update data

		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> trendSignals() const;
		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> analogSignals() const;
		[[nodiscard]] std::vector<TrendLib::TrendSignalParam> discreteSignals() const;

		[[nodiscard]] std::vector<Hash> trendSignalsHashes(const QString& equipmentId = QString()) const;
		[[nodiscard]] QStringList trendSignalIds() const;

		[[nodiscard]] int discreteSignalsCount() const;
		[[nodiscard]] int analogSignalsCount() const;

		bool getFullExistingTrendData(const TrendSignalParam& trendSignal,
									  E::TimeType timeType,
									  std::list<std::shared_ptr<OneHourData>>* outData) const;
		bool getExistingTrendData(const TrendSignalParam& trendSignal,
								  QDateTime from,
								  QDateTime to,
								  E::TimeType timeType,
								  std::list<std::shared_ptr<OneHourData>>* outData) const;
		std::optional<TrendStateItem> lastRealtimeState(Hash signalHash, E::TimeType timeType) const;

		// ITrendDataProvider implementation
		//
		virtual bool trendData(QUuid trendUuid,
							   const TrendSignalParam& trendSignal,
							   QDateTime from,
							   QDateTime to,
							   E::TimeType timeType,
							   E::TrendMode mode,
							   std::list<std::shared_ptr<OneHourData>>* outData) const override;

		virtual TimeStamp maxTimeStamp(QUuid trendUuid, E::TimeType timeType) const override;

		// End of ITrendDataProvider
		//
		bool addTrendPoint(const TrendSignalParam& signal, E::TimeType timeType, TrendStateItem stateItem);
		bool removeTrendPoint(const TrendSignalParam& signal, int index, E::TimeType timeType);

		void clear(E::TimeType timeType);

		/// Clear all archive data where request was not processed yet or there is not data.
		/// This function is used when the new configuration is arriving, so the request can be sent again
		///
		void clearArchiveWithoutRecord();

		// Add non valid points to all signals, useful in switching mode Archive/RealTime
		//
	public slots:
		void addNonValidPoint();
		void addNonValidPoint(E::TimeType timeType);
		static void addNonValidPoint(TrendArchive* archive);

		// --
		//
		void slot_archiveDataReceived(TrendSignalPlusServerId trendSignalPlusServerId,
									  TimeStamp requestedHour,
									  E::TimeType timeType,
									  std::shared_ptr<TrendLib::OneHourData> data);
		void slot_archiveRequestError(TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType);

		void slot_realtimeDataReceived(QString sourceEquipmentId,
									   std::shared_ptr<TrendLib::RealtimeData> data,
									   TrendLib::TrendStateItem minState,
									   TrendLib::TrendStateItem maxState);
		void slot_realtimeRequestError(QString errorText);
		void slot_realtimeConnectionLost(QString sourceEquipmentId);

		void slot_trimData(E::TimeType timeType, TimeStamp trimFrom); // Trim data from time trimFrom to the end (right).

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
		mutable std::map<TrendSignalPlusServerId, TrendArchive>
			m_archiveLocalTime;             // Key is "AppSignalID@ArchiveServerID", Example: #ABC01@USB_SHK_WS00_ARCHSRV
		mutable std::map<TrendSignalPlusServerId, TrendArchive> m_archiveSystemTime;
		mutable std::map<TrendSignalPlusServerId, TrendArchive> m_archivePlantTime;

		mutable QMutex m_lastRealtimePointsMutex;
		std::map<Hash, TrendStateItem>
			m_lastRealtimePointsLocalTime;  // Key is hash form signal id, the value is last received real-time state.
		std::map<Hash, TrendStateItem>
			m_lastRealtimePointsSystemTime; // Key is hash form signal id, the value is last received real-time state.
		std::map<Hash, TrendStateItem>
			m_lastRealtimePointsPlantTime;  // Key is hash form signal id, the value is last received real-time state.
	};
} // namespace TrendLib

Q_DECLARE_METATYPE(std::shared_ptr<TrendLib::OneHourData>)
