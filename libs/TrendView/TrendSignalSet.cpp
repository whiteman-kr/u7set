#include <TrendView/TrendArchiveServer.h>
#include <TrendView/TrendSignalSet.h>
#include <TrendView/TrendSignalState.h>
#include <type_traits>


namespace TrendLib
{
	bool TrendArchive::save(TrendSignalPlusServerId trendSignalPlusServerId, Proto::TrendArchive* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		message->set_app_signal_id(trendSignalPlusServerId.appSignalId.toStdString());
		message->set_archive_server_id(trendSignalPlusServerId.archiveServerId.toStdString());

		for (const auto& p : m_hours)
		{
			if (p.second == nullptr)
			{
				Q_ASSERT(p.second);
				continue;
			}

			ok &= p.second->save(p.first, message->add_hours());
		}

		return ok;
	}

	bool TrendArchive::load(const Proto::TrendArchive& message)
	{
		bool ok = true;

		trendSignalPlusServerId.appSignalId = QString::fromStdString(message.app_signal_id());
		trendSignalPlusServerId.archiveServerId = QString::fromStdString(message.archive_server_id());

		m_hours.clear();
		for (int i = 0; i < message.hours_size(); i++)
		{
			const ::Proto::TrendArchiveHour& messageHour = message.hours(i);

			TimeStamp ts(messageHour.time_stamp());

			auto hodit = m_hours.emplace(ts, std::make_shared<OneHourData>());

			std::shared_ptr<OneHourData>& hourData = hodit.first->second;
			ok &= hourData->load(messageHour);
		}

		return ok;
	}


	//
	//
	//			TrendSignalSet
	//
	//
	TrendSignalSet::TrendSignalSet()
	{
		static_assert(std::is_trivial<TrendStateItem>::value, "TrendStateItem must be trivial as it is stored in bytearray");
	}

	bool TrendSignalSet::save(::Proto::TrendSignalSet* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		// Scope for locking m_paramMutex
		//
		{
			QMutexLocker l(&m_paramMutex);
			for (const TrendSignalParam& p : m_signalParams)
			{
				ok &= p.save(message->add_signal_params());
			}
		}

		// Scope for locking m_archiveMutex
		//
		{
			QMutexLocker l(&m_archiveMutex);

			for (const auto& [hash, trendArchive] : m_archiveLocalTime)
			{
				::Proto::TrendArchive* messageTrenaArchive = message->add_archive_local_time();
				ok &= trendArchive.save(trendArchive.trendSignalPlusServerId, messageTrenaArchive);
			}

			for (const auto& [hash, trendArchive] : m_archiveSystemTime)
			{
				::Proto::TrendArchive* messageTrenaArchive = message->add_archive_system_time();
				ok &= trendArchive.save(trendArchive.trendSignalPlusServerId, messageTrenaArchive);
			}

			for (const auto& [hash, trendArchive] : m_archivePlantTime)
			{
				::Proto::TrendArchive* messageTrenaArchive = message->add_archive_plant_time();
				ok &= trendArchive.save(trendArchive.trendSignalPlusServerId, messageTrenaArchive);
			}
		}

		return ok;
	}

	bool TrendSignalSet::load(const ::Proto::TrendSignalSet& message)
	{
		bool ok = true;

		// Scope for locking m_paramMutex
		//
		{
			QMutexLocker l(&m_paramMutex);

			m_signalParams.clear();
			for (int i = 0; i < message.signal_params_size(); i++)
			{
				TrendSignalParam tsp;

				bool loadTspOk = tsp.load(message.signal_params(i));
				ok &= loadTspOk;

				if (loadTspOk == true)
				{
					m_signalParams.push_back(tsp);
				}
			}
		}

		// Scope for locking m_archiveMutex
		//
		{
			QMutexLocker l(&m_archiveMutex);

			m_archiveLocalTime.clear();
			m_archiveSystemTime.clear();
			m_archivePlantTime.clear();

			for (int i = 0; i < message.archive_local_time_size(); i++)
			{
				const ::Proto::TrendArchive& messageArchive = message.archive_local_time(i);

				TrendSignalPlusServerId tsps{.appSignalId = QString::fromStdString(messageArchive.app_signal_id()),
											 .archiveServerId = QString::fromStdString(messageArchive.archive_server_id())};

				auto itp = m_archiveLocalTime.emplace(tsps, TrendArchive{tsps});

				TrendArchive& ta = itp.first->second;
				ok &= ta.load(messageArchive);
			}

			for (int i = 0; i < message.archive_system_time_size(); i++)
			{
				const ::Proto::TrendArchive& messageArchive = message.archive_system_time(i);

				TrendSignalPlusServerId tsps{.appSignalId = QString::fromStdString(messageArchive.app_signal_id()),
											 .archiveServerId = QString::fromStdString(messageArchive.archive_server_id())};

				auto itp = m_archiveSystemTime.emplace(tsps, TrendArchive{tsps});

				TrendArchive& ta = itp.first->second;
				ok &= ta.load(messageArchive);
			}

			for (int i = 0; i < message.archive_plant_time_size(); i++)
			{
				const ::Proto::TrendArchive& messageArchive = message.archive_plant_time(i);

				TrendSignalPlusServerId tsps{.appSignalId = QString::fromStdString(messageArchive.app_signal_id()),
											 .archiveServerId = QString::fromStdString(messageArchive.archive_server_id())};

				auto itp = m_archivePlantTime.emplace(tsps, TrendArchive{tsps});

				TrendArchive& ta = itp.first->second;
				ok &= ta.load(messageArchive);
			}
		}

		return ok;
	}

	bool TrendSignalSet::addSignal(const TrendSignalParam& signal)
	{
		QMutexLocker locker(&m_paramMutex);

		if (m_signalParams.size() >= 16)
		{
			return false;
		}

		auto foundIt = std::find_if(m_signalParams.begin(),
									m_signalParams.end(),
									[&signal](const TrendSignalParam& s)
									{
										return s.signalId() == signal.signalId() && s.archiveServerId() == signal.archiveServerId();
									});

		if (foundIt != m_signalParams.end())
		{
			return false;
		}

		m_signalParams.push_back(signal);

		return true;
	}

	bool TrendSignalSet::addSignals(std::list<TrendSignalParam>&& signalParams)
	{
		if (signalParams.size() >= 16)
		{
			return false;
		}

		QMutexLocker locker(&m_paramMutex);

		m_signalParams.clear();
		m_signalParams = std::move(signalParams);

		return true;
	}

	void TrendSignalSet::removeSignal(const TrendLib::TrendSignalParam& signal)
	{
		QMutexLocker locker(&m_paramMutex);

		m_signalParams.remove_if(
			[&signal](const TrendSignalParam& s)
			{
				return s.appSignalId() == signal.appSignalId() && s.archiveServerShortId() == signal.archiveServerShortId();
			});

		return;
	}

	void TrendSignalSet::removeAllSignals()
	{
		QMutexLocker locker(&m_paramMutex);
		m_signalParams.clear();
		return;
	}

	void TrendSignalSet::reorderSignals(std::span<const TrendLib::TrendSignalParam> targetOrder)
	{
		QMutexLocker locker(&m_paramMutex);

		// Move items in signalIds order.
		//
		std::list<TrendSignalParam> newSignalList;

		for (const auto& targetSignal : targetOrder)
		{
			auto it = std::find_if(m_signalParams.begin(),
								   m_signalParams.end(),
								   [&targetSignal](const TrendSignalParam& ts)
								   {
									   return ts.appSignalId() == targetSignal.appSignalId() &&
											  ts.archiveServerId() == targetSignal.archiveServerId();
								   });

			if (it != m_signalParams.end())
			{
				newSignalList.splice(newSignalList.end(), m_signalParams, it);
			}
		}

		// Move all other items in the same order
		//
		newSignalList.splice(newSignalList.end(), m_signalParams);

		// Set result.
		//
		m_signalParams = std::move(newSignalList);

		return;
	}

	TrendLib::TrendSignalParam TrendSignalSet::signalParam(const QString& appSignalId, const QString& archiveServerId, bool* ok) const
	{
		if (ok != nullptr)
		{
			*ok = false;
		}

		QMutexLocker locker(&m_paramMutex);

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.appSignalId() == appSignalId && s.archiveServerId() == archiveServerId)
			{
				if (ok != nullptr)
				{
					*ok = true;
				}

				return s;
			}
		}

		return {};
	}

	bool TrendSignalSet::setSignalParam(const TrendLib::TrendSignalParam& signalParam)
	{
		QMutexLocker locker(&m_paramMutex);

		for (TrendSignalParam& s : m_signalParams)
		{
			if (s.appSignalId() == signalParam.appSignalId() && s.archiveServerId() == signalParam.archiveServerId())
			{
				s = signalParam;
				return true;
			}
		}

		return false;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::trendSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		int index = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			auto& is = result.emplace_back(s);
			is.setTempSignalIndex(index);

			index++;
		}

		return result;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::analogSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		int index = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isAnalog() == true)
			{
				auto& is = result.emplace_back(s);
				is.setTempSignalIndex(index);

				index++;
			}
		}

		return result;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::discreteSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		int index = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				auto& is = result.emplace_back(s);
				is.setTempSignalIndex(index);

				index++;
			}
		}

		return result;
	}

	std::vector<Hash> TrendSignalSet::trendSignalsHashes(const QString& equipmentId /*= QString()*/) const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<Hash> result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (equipmentId.isEmpty() == true || s.equipmentId() == equipmentId)
			{
				result.emplace_back(s.appSignalHash());
			}
		}

		return result;
	}

	QStringList TrendSignalSet::trendSignalIds() const
	{
		QMutexLocker locker(&m_paramMutex);

		QStringList result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			result << s.appSignalId();
		}

		return result;
	}

	int TrendSignalSet::discreteSignalsCount() const
	{
		QMutexLocker locker(&m_paramMutex);

		int count = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				count++;
			}
		}

		return count;
	}

	int TrendSignalSet::analogSignalsCount() const
	{
		QMutexLocker locker(&m_paramMutex);

		int count = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isAnalog() == true)
			{
				count++;
			}
		}

		return count;
	}

	bool TrendSignalSet::getFullExistingTrendData(const TrendSignalParam& trendSignal,
												  E::TimeType timeType,
												  std::list<std::shared_ptr<const OneHourData>>* outData) const
	{
		// Get already requested and received (or read from file) data
		// Don't request any data if it is not present
		//
		if (outData == nullptr)
		{
			Q_ASSERT(outData);
			return false;
		}

		outData->clear();

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return false;
		}

		auto archiveIt = m_archive->find(trendSignal.signalPlusServerId());
		if (archiveIt == m_archive->end())
		{
			return false;
		}

		TrendArchive& archive = archiveIt->second; // archive is MUTABLE

		// --
		//
		// for (TimeStamp archHour : allTimeStamps)
		for (const auto& [archHour, hourData] : archive.m_hours)
		{
			if (archHour.toDateTime().time().minute() != 0 || archHour.toDateTime().time().second() != 0 ||
				archHour.toDateTime().time().msec() != 0)
			{
				Q_ASSERT(archHour.toDateTime().time().minute() == 0);
				Q_ASSERT(archHour.toDateTime().time().second() == 0);
				Q_ASSERT(archHour.toDateTime().time().msec() == 0);
				return false;
			}

			if (hourData == nullptr)
			{
				Q_ASSERT(hourData);
				continue;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_ptr<const TrendLib::OneHourData> copiedHourData = hourData;
#else
			// Make a copy of OneHourData, as it can be appended in Realtime trends
			//
			std::shared_ptr<const TrendLib::OneHourData> copiedHourData = std::make_shared<TrendLib::OneHourData>(hourData.operator*());
#endif

			outData->push_back(copiedHourData); // Request state does not matter
		}

		return true;
	}

	bool TrendSignalSet::getExistingTrendData(const TrendSignalParam& trendSignal,
											  QDateTime from,
											  QDateTime to,
											  E::TimeType timeType,
											  std::list<std::shared_ptr<const OneHourData>>* outData) const
	{
		// Get already requested and received (or read from file) data
		// Don't request any data if it is not present
		//
		if (outData == nullptr || from > to)
		{
			Q_ASSERT(outData);
			Q_ASSERT(from <= to);
			return false;
		}

		outData->clear();

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return false;
		}

		auto archiveIt = m_archive->find(trendSignal.signalPlusServerId());
		if (archiveIt == m_archive->end())
		{
			return false;
		}

		TrendArchive& archive = archiveIt->second; // archive is MUTABLE

		// Round from/to to 1hour
		//
		TimeStamp fromTimeStamp((from.toMSecsSinceEpoch() / 1_hour) * 1_hour);
		TimeStamp toTimeStamp((to.toMSecsSinceEpoch() / 1_hour) * 1_hour + (to.toMSecsSinceEpoch() % 1_hour == 0 ? 0 : 1_hour));

		//		qDebug() << "getExistingTrendData for appSignalID: " << appSignalId;
		//		qDebug() << "\tAsk data from " << from << ", rounded to " << fromTimeStamp.toDateTime();
		//		qDebug() << "\tAsk data to  " << to << ", rounded to " << toTimeStamp.toDateTime();

		// --
		//
		for (TimeStamp archHour = fromTimeStamp; archHour < toTimeStamp; archHour.timeStamp += 1_hour)
		{
			if (archHour.toDateTime().time().minute() != 0 || archHour.toDateTime().time().second() != 0 ||
				archHour.toDateTime().time().msec() != 0)
			{
				Q_ASSERT(archHour.toDateTime().time().minute() == 0);
				Q_ASSERT(archHour.toDateTime().time().second() == 0);
				Q_ASSERT(archHour.toDateTime().time().msec() == 0);
				return false;
			}

			auto archHourIt = archive.m_hours.find(archHour);
			if (archHourIt == archive.m_hours.end())
			{
				continue;
			}

			std::shared_ptr<OneHourData> hourData = archHourIt->second;
			if (hourData == nullptr)
			{
				Q_ASSERT(hourData);
				continue;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_ptr<const TrendLib::OneHourData> copiedHourData = hourData;
#else
			// Make a copy of OneHourData, as it can be appended in Realtime trends
			//
			std::shared_ptr<const TrendLib::OneHourData> copiedHourData = std::make_shared<TrendLib::OneHourData>(hourData.operator*());
#endif

			outData->push_back(copiedHourData); // Request state does not matter
		}

		return true;
	}

	std::optional<TrendStateItem> TrendSignalSet::lastRealtimeState(Hash signalHash, E::TimeType timeType) const
	{
		std::optional<TrendStateItem> result;

		QMutexLocker ml(&m_lastRealtimePointsMutex);
		const std::map<Hash, TrendStateItem>* realTimeLastPoints = nullptr;

		switch (timeType)
		{
		case E::TimeType::Local:
			realTimeLastPoints = &m_lastRealtimePointsLocalTime;
			break;
		case E::TimeType::System:
			realTimeLastPoints = &m_lastRealtimePointsSystemTime;
			break;
		case E::TimeType::Plant:
			realTimeLastPoints = &m_lastRealtimePointsPlantTime;
			break;
		default:
			Q_ASSERT(false);
			return result;
		}

		auto it = realTimeLastPoints->find(signalHash);
		if (it == realTimeLastPoints->end())
		{
			return result;
		}

		result = it->second;
		return result;
	}

	bool TrendSignalSet::trendData(QUuid /*trendUuid*/,
								   const TrendSignalParam& trendSignal,
								   QDateTime from,
								   QDateTime to,
								   E::TimeType timeType,
								   E::TrendMode mode,
								   std::list<std::shared_ptr<const OneHourData>>* outData) const
	{
		if (outData == nullptr || from > to)
		{
			Q_ASSERT(outData);
			Q_ASSERT(from <= to);
			return false;
		}

		TrendSignalPlusServerId trendSignalId = trendSignal.signalPlusServerId();

		// Find Signal
		//
		{
			QMutexLocker locker(&m_archiveMutex);

			std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
			switch (timeType)
			{
			case E::TimeType::Local:
				m_archive = &m_archiveLocalTime;
				break;
			case E::TimeType::System:
				m_archive = &m_archiveSystemTime;
				break;
			case E::TimeType::Plant:
				m_archive = &m_archivePlantTime;
				break;
			default:
				Q_ASSERT(false);
				return false;
			}

			auto archiveIt = m_archive->find(trendSignalId);
			if (archiveIt == m_archive->end())
			{
				auto emplaceResult = m_archive->emplace(trendSignalId, TrendArchive{trendSignalId});
				archiveIt = emplaceResult.first;
			}

			TrendArchive& archive = archiveIt->second; // archive is MUTABLE

			// Round from/to to 1hour
			//
			TimeStamp fromTimeStamp((from.toMSecsSinceEpoch() / 1_hour) * 1_hour);
			TimeStamp toTimeStamp((to.toMSecsSinceEpoch() / 1_hour) * 1_hour + (to.toMSecsSinceEpoch() % 1_hour == 0 ? 0 : 1_hour));

			//			qDebug() << "Requested for trend data, appSignalID: " << appSignalId;
			//			qDebug() << "\tRequested from " << from << ", rounded to " << fromTimeStamp.toDateTime();
			//			qDebug() << "\tRequested to " << to << ", rounded to " << toTimeStamp.toDateTime();

			// --
			//
			for (TimeStamp archHour = fromTimeStamp; archHour < toTimeStamp; archHour.timeStamp += 1_hour)
			{
				if (archHour.toDateTime().time().minute() != 0 || archHour.toDateTime().time().second() != 0 ||
					archHour.toDateTime().time().msec() != 0)
				{
					Q_ASSERT(archHour.toDateTime().time().minute() == 0);
					Q_ASSERT(archHour.toDateTime().time().second() == 0);
					Q_ASSERT(archHour.toDateTime().time().msec() == 0);
					return false;
				}

				if (mode == E::TrendMode::Archive)
				{
					std::shared_ptr<OneHourData>& hourData = archive.m_hours[archHour]; // Get hour data, insert if there is no such record

					if (hourData == nullptr)
					{
						// hourData just was created in call "archive.m_hours[archHour]"
						//
						hourData = std::make_shared<OneHourData>();
					}

#ifdef TREND_ZERO_COPY_TREND_DATA
					std::unique_lock lock{hourData->mutex};
#endif

					switch (hourData->state_)
					{
					case OneHourData::State::NoData:
						// No data, request data from archive
						//
						emit requestData(trendSignalId, archHour, timeType);
						hourData->state_ = OneHourData::State::Requested;
						break;
					case OneHourData::State::Requested:
						// Data already requested, wait for it, just do nothing
						//
						break;
					case OneHourData::State::Received:
						{
#ifdef TREND_ZERO_COPY_TREND_DATA
							auto copiedHourData = hourData;
#else
							// Data requested and received, pass it to the result
							// MAKE A COPY of hourData
							//
							auto copiedHourData = std::make_shared<TrendLib::OneHourData>(hourData.operator*());
#endif
							outData->push_back(copiedHourData);
						}
						break;
					default:
						Q_ASSERT(false);
						return false;
					}
				}

				if (mode == E::TrendMode::Realtime)
				{
					auto hit = archive.m_hours.find(archHour);
					if (hit != archive.m_hours.end())
					{
#ifdef TREND_ZERO_COPY_TREND_DATA
						auto copiedHourData = hit->second;
#else
						// MAKE A COPY of hourData
						//
						auto copiedHourData = std::make_shared<TrendLib::OneHourData>(hit->second.operator*());
#endif
						outData->push_back(copiedHourData);
					}
				}
			}

		} // QMutexLocker locker(&m_archiveMutex);

		return true;
	}

	TimeStamp TrendSignalSet::maxTimeStamp(QUuid /*trendUuid*/, E::TimeType timeType) const
	{
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return {};
		}

		TimeStamp result{};

		for (const auto& [signalHash, trendArchive] : *m_archive)
		{
			auto rit = trendArchive.m_hours.rbegin();
			if (rit != trendArchive.m_hours.rend())
			{
				const std::shared_ptr<OneHourData>& sp = rit->second;
				Q_ASSERT(sp);

#ifdef TREND_ZERO_COPY_TREND_DATA
				std::shared_lock lock{sp->mutex};
#endif

				if (sp->data_.empty() == false)
				{
					const TrendStateRecord& record = sp->data_.back();

					if (record.states.empty() == false)
					{
						switch (timeType)
						{
						case E::TimeType::Local:
							result.timeStamp = std::max(result.timeStamp, record.states.back().local);
							break;
						case E::TimeType::System:
							result.timeStamp = std::max(result.timeStamp, record.states.back().system);
							break;
						case E::TimeType::Plant:
							result.timeStamp = std::max(result.timeStamp, record.states.back().plant);
							break;
						default:
							Q_ASSERT(false);
							return {};
						}
					}
				}
			}
		}

		return result;
	}

	bool TrendSignalSet::addTrendPoint(const TrendSignalParam& signal, E::TimeType timeType, TrendStateItem stateItem)
	{
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return false;
		}

		const auto signalPlusServerId = signal.signalPlusServerId();

		auto archiveIt = m_archive->find(signalPlusServerId);
		if (archiveIt == m_archive->end())
		{
			m_archive->emplace(signalPlusServerId, TrendArchive(signalPlusServerId));

			archiveIt = m_archive->find(signalPlusServerId);

			if (archiveIt == m_archive->end())
			{
				Q_ASSERT(false);
				return false;
			}
		}

		TrendArchive& archive = archiveIt->second; // archive is MUTABLE

		// Round time to 1hour
		//
		TimeStamp stateTimeHour = stateItem.getTime(timeType).roundedToHour();

		//		qDebug() << "addTrendPoint for appSignalID: " << appSignalId;
		//		qDebug() << "\tAdd data to " << archHour.toDateTime();

		// Find one hour record or create it
		//
		auto stateTimeHourIt = archive.m_hours.find(stateTimeHour);

		std::shared_ptr<OneHourData> hourData;

		if (stateTimeHourIt == archive.m_hours.end())
		{
			// No such hour, create data and add it
			//
			hourData = std::make_shared<OneHourData>();
			archive.m_hours[stateTimeHour] = hourData;
		}
		else
		{
			hourData = stateTimeHourIt->second;
		}

		if (hourData == nullptr)
		{
			Q_ASSERT(hourData);
			return false;
		}

#ifdef TREND_ZERO_COPY_TREND_DATA
		std::unique_lock lock{hourData->mutex};
#endif

		hourData->state_ = OneHourData::State::Received;

		TimeStamp stateTime = stateItem.getTime(timeType);

		// Try to insert the record between two time-neighbour records
		//
		TimeStamp previousTime;
		TimeStamp nextTime;
		bool timePreviousInitialized = false;

		for (TrendStateRecord& record : hourData->data_)
		{
			for (auto it = record.states.begin(); it != record.states.end(); it++)
			{
				TrendStateItem& nextItem = *it;
				nextTime = nextItem.getTime(timeType);

				if (timePreviousInitialized == true)
				{
					if (previousTime <= stateTime && stateTime <= nextTime)
					{
						record.states.insert(it, stateItem);
						return true;
					}
				}

				if (timePreviousInitialized == false || nextTime > previousTime)
				{
					previousTime = nextTime;
					timePreviousInitialized = true;
				}
			}
		}

		// Record was not inserted between two neighbours, add it at the beginning or at the end
		//
		if (hourData->data_.empty() == true)
		{
			// If no records exist in hour data, add the record
			//
			hourData->data_.emplace_back(); // TrendStateRecord record;
		}

		if (stateTime > nextTime)
		{
			// Insert at the end
			//
			TrendStateRecord& record = hourData->data_[hourData->data_.size() - 1];
			record.states.push_back(stateItem);
		}
		else
		{
			// Insert at the beginning
			//
			TrendStateRecord& record = hourData->data_[0];
			record.states.insert(record.states.begin(), stateItem);
		}

		return true;
	}

	bool TrendSignalSet::removeTrendPoint(const TrendSignalParam& signal, int index, E::TimeType timeType)
	{
		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return false;
		}

		const auto signalPlusServerId = signal.signalPlusServerId();

		auto archiveIt = m_archive->find(signalPlusServerId);
		if (archiveIt == m_archive->end())
		{
			return false;
		}

		TrendArchive& archive = archiveIt->second; // archive is MUTABLE

		std::vector<TimeStamp> allTimeStamps;

		for (auto it = archive.m_hours.begin(); it != archive.m_hours.end(); it++)
		{
			allTimeStamps.push_back(it->first);
		}

		std::sort(allTimeStamps.begin(), allTimeStamps.end(), std::less<TimeStamp>());

		int currentIndex = 0;

		// --
		//
		for (TimeStamp archHour : allTimeStamps)
		{
			if (archHour.toDateTime().time().minute() != 0 || archHour.toDateTime().time().second() != 0 ||
				archHour.toDateTime().time().msec() != 0)
			{
				Q_ASSERT(archHour.toDateTime().time().minute() == 0);
				Q_ASSERT(archHour.toDateTime().time().second() == 0);
				Q_ASSERT(archHour.toDateTime().time().msec() == 0);
				return false;
			}

			auto archHourIt = archive.m_hours.find(archHour);
			if (archHourIt == archive.m_hours.end())
			{
				Q_ASSERT(false);
				return false;
			}

			std::shared_ptr<OneHourData> hourData = archHourIt->second;
			if (hourData == nullptr)
			{
				Q_ASSERT(hourData);
				continue;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::unique_lock lock{hourData->mutex};
#endif

			for (auto recordIt = hourData->data_.begin(); recordIt != hourData->data_.end(); recordIt++)
			{
				TrendStateRecord& record = *recordIt;

				for (auto stateIt = record.states.begin(); stateIt != record.states.end(); stateIt++)
				{
					if (currentIndex == index)
					{
						record.states.erase(stateIt);

						if (record.states.empty() == true)
						{
							hourData->data_.erase(recordIt);

							if (hourData->data_.empty() == true)
							{
								archive.m_hours.erase(archHour);
							}
						}

						return true;
					}

					currentIndex++;
				}
			}
		}

		// Point with specified index was not found...

		Q_ASSERT(false);
		return false;
	}

	void TrendSignalSet::clear(E::TimeType timeType)
	{
		QMutexLocker locker(&m_archiveMutex);

		switch (timeType)
		{
		case E::TimeType::Local:
			m_archiveLocalTime.clear();
			break;
		case E::TimeType::System:
			m_archiveSystemTime.clear();
			break;
		case E::TimeType::Plant:
			m_archivePlantTime.clear();
			break;
		default:
			Q_ASSERT(false);
			break;
		}

		return;
	}

	void TrendSignalSet::clearArchiveWithoutRecord()
	{
		QMutexLocker locker(&m_archiveMutex);

		for (auto [signalId, trendArchive] : m_archiveLocalTime)
		{
			Q_UNUSED(signalId);

			erase_if(trendArchive.m_hours,
					 [](const auto& item)
					 {
						 auto const& [timeStamp, oneHourData] = item;
						 Q_UNUSED(timeStamp);
#ifdef TREND_ZERO_COPY_TREND_DATA
						 std::shared_lock lock{oneHourData->mutex};
#endif
						 return oneHourData->state_ != OneHourData::State::Received;
					 });
		}

		for (auto [signalId, trendArchive] : m_archiveSystemTime)
		{
			Q_UNUSED(signalId);

			erase_if(trendArchive.m_hours,
					 [](const auto& item)
					 {
						 auto const& [timeStamp, oneHourData] = item;
						 Q_UNUSED(timeStamp);
#ifdef TREND_ZERO_COPY_TREND_DATA
						 std::shared_lock lock{oneHourData->mutex};
#endif
						 return oneHourData->state_ != OneHourData::State::Received;
					 });
		}

		for (auto [signalId, trendArchive] : m_archivePlantTime)
		{
			Q_UNUSED(signalId);

			erase_if(trendArchive.m_hours,
					 [](const auto& item)
					 {
						 auto const& [timeStamp, oneHourData] = item;
						 Q_UNUSED(timeStamp);
#ifdef TREND_ZERO_COPY_TREND_DATA
						 std::shared_lock lock{oneHourData->mutex};
#endif
						 return oneHourData->state_ != OneHourData::State::Received;
					 });
		}

		return;
	}

	void TrendSignalSet::addNonValidPoint()
	{
		// Add non valid points to all signals, useful in switching mode Archive/RealTime
		//
		addNonValidPoint(E::TimeType::Local);
		addNonValidPoint(E::TimeType::System);
		addNonValidPoint(E::TimeType::Plant);

		return;
	}

	void TrendSignalSet::addNonValidPoint(E::TimeType timeType)
	{
		{
			QMutexLocker ml(&m_lastRealtimePointsMutex);
			std::map<Hash, TrendStateItem>* realTimeLastPoints = nullptr;

			switch (timeType)
			{
			case E::TimeType::Local:
				realTimeLastPoints = &m_lastRealtimePointsLocalTime;
				break;
			case E::TimeType::System:
				realTimeLastPoints = &m_lastRealtimePointsSystemTime;
				break;
			case E::TimeType::Plant:
				realTimeLastPoints = &m_lastRealtimePointsPlantTime;
				break;
			default:
				Q_ASSERT(false);
				return;
			}

			realTimeLastPoints->clear();
		}

		// Add non valid points to all signals, useful in switching mode Archive/RealTime
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return;
		}

		// --
		//
		for (auto& [hash, trendArchive] : *archive)
		{
			TrendSignalSet::addNonValidPoint(&trendArchive);
		}

		return;
	}

	void TrendSignalSet::addNonValidPoint(TrendArchive* trendArchive)
	{
		if (trendArchive->m_hours.empty() == true)
		{
			// Do not add non-valid point if signal archive is empty
			//
		}
		else
		{
			// Find the last hour with points and add non valid state to it
			//
			for (auto rhit = trendArchive->m_hours.rbegin(); rhit != trendArchive->m_hours.rend(); ++rhit)
			{
				std::shared_ptr<OneHourData> hour = rhit->second;

#ifdef TREND_ZERO_COPY_TREND_DATA
				std::unique_lock lock{hour->mutex};
#endif

				if (hour->data_.empty() == true)
				{
					// It can be just request for ah hour
					// Process the next hour
					//
					continue;
				}
				else
				{
					// Assume that record has some states
					//
					TrendStateRecord& record = hour->data_.back();

					if (record.states.empty() == false &&
						record.states.back().isValid() == true) // Do not add another nonvalid point if it is already present.
					{
						// Just duplicate last state with invalid flag
						//
						TrendStateItem tsi = record.states.back();
						tsi.flags = 0;

						record.states.push_back(tsi);
					}
					else
					{
						Q_ASSERT(record.states.empty() == false);
					}

					break;
				}
			}
		}

		return;
	}

	void TrendSignalSet::slot_archiveDataReceived(TrendSignalPlusServerId trendSignalPlusServerId,
												  TimeStamp requestedHour,
												  E::TimeType timeType,
												  std::shared_ptr<TrendLib::OneHourData> data)
	{
		// Ignore data if there is no such signal in SignalParams
		// It could be requested but later signal was excluded
		//
		{
			QMutexLocker paramLocker(&m_paramMutex);
			auto it = std::find_if(m_signalParams.begin(),
								   m_signalParams.end(),
								   [&trendSignalPlusServerId](const TrendSignalParam& p)
								   {
									   return trendSignalPlusServerId == p;
								   });

			if (it == m_signalParams.end())
			{
				return;
			}
		}

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return;
		}

		auto archiveIt = m_archive->find(trendSignalPlusServerId);
		if (archiveIt == m_archive->end())
		{
			auto emplaceResult = m_archive->emplace(trendSignalPlusServerId, TrendArchive{trendSignalPlusServerId});
			archiveIt = emplaceResult.first;
		}

		TrendArchive& archive = archiveIt->second; // archive is MUTABLE

		// --
		//
		if (requestedHour.toDateTime().time().minute() != 0 || requestedHour.toDateTime().time().second() != 0 ||
			requestedHour.toDateTime().time().msec() != 0)
		{
			Q_ASSERT(requestedHour.toDateTime().time().minute() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().second() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().msec() == 0);
			return;
		}

		archive.m_hours[requestedHour] = std::move(data);

		return;
	}

	void TrendSignalSet::slot_archiveRequestError(TrendSignalPlusServerId trendSignalPlusServerId,
												  TimeStamp requestedHour,
												  E::TimeType timeType)
	{
		// Ignore data if there is no such signal in SignalParams
		// Probably it was requested but later signal was excluded
		//
		{
			QMutexLocker paramLocker(&m_paramMutex);
			auto it = std::find_if(m_signalParams.begin(),
								   m_signalParams.end(),
								   [&trendSignalPlusServerId](const TrendSignalParam& p)
								   {
									   return p == trendSignalPlusServerId;
								   });

			if (it == m_signalParams.end())
			{
				return;
			}
		}

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return;
		}

		auto archiveIt = m_archive->find(trendSignalPlusServerId);
		if (archiveIt == m_archive->end())
		{
			return;
		}

		TrendArchive& archive = archiveIt->second; // archive is MUTABLE

		// --
		//
		if (requestedHour.toDateTime().time().minute() != 0 || requestedHour.toDateTime().time().second() != 0 ||
			requestedHour.toDateTime().time().msec() != 0)
		{
			Q_ASSERT(requestedHour.toDateTime().time().minute() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().second() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().msec() == 0);
			return;
		}

		archive.m_hours.erase(requestedHour);

		return;
	}

	void TrendSignalSet::slot_realtimeDataReceived(QString sourceEquipmentId,
												   std::shared_ptr<TrendLib::RealtimeData> data,
												   E::RtTrendsSamplePeriod samplePeriod,
												   TrendLib::TrendStateItem /*minState*/,
												   TrendLib::TrendStateItem /*maxState*/)
	{
		for (const TrendLib::RealtimeDataChunk& chunk : data->signalData)
		{
			const Hash signalHash = chunk.appSignalHash;
			const std::vector<TrendStateItem>& states = chunk.states;

			// For now add all three times, maybe later it will be changed to add just for one time
			// I just don't know which kind of time is used now
			//
			appendRealtimeDataToArchive(sourceEquipmentId, E::TimeType::Local, samplePeriod, signalHash, states);
			appendRealtimeDataToArchive(sourceEquipmentId, E::TimeType::System, samplePeriod, signalHash, states);
			appendRealtimeDataToArchive(sourceEquipmentId, E::TimeType::Plant, samplePeriod, signalHash, states);
		}

		return;
	}

	void TrendSignalSet::slot_realtimeRequestError(QString /*errorText*/) {}

	void TrendSignalSet::slot_realtimeConnectionLost(QString sourceEquipmentId)
	{
		auto addNunValidFunc = [this, sourceEquipmentId](E::TimeType timeType)
		{
			// Add non valid points to all signals, useful in switching mode Archive/RealTime
			//
			QMutexLocker locker(&m_archiveMutex);

			std::map<TrendSignalPlusServerId, TrendArchive>* archive = nullptr;
			switch (timeType)
			{
			case E::TimeType::Local:
				archive = &m_archiveLocalTime;
				break;
			case E::TimeType::System:
				archive = &m_archiveSystemTime;
				break;
			case E::TimeType::Plant:
				archive = &m_archivePlantTime;
				break;
			default:
				Q_ASSERT(false);
				return;
			}

			for (auto& [hash, trendArchive] : *archive)
			{
				if (sourceEquipmentId == trendArchive.realTimeActiveServiceId)
				{
					TrendSignalSet::addNonValidPoint(&trendArchive);
				}
			}

			return;
		};

		addNunValidFunc(E::TimeType::Local);
		addNunValidFunc(E::TimeType::System);
		addNunValidFunc(E::TimeType::Plant);

		return;
	}

	void TrendSignalSet::slot_trimData(E::TimeType timeType, TimeStamp trimFrom)
	{
		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;

		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return;
		}

		for (auto& [trendSignalPlusServerId, archive] : *m_archive)
		{
			std::erase_if(archive.m_hours,
						  [trimFrom](const auto& item)
						  {
							  auto const& [timeStamp, hourData] = item;
							  return timeStamp >= trimFrom;
						  });

			// Remove TrendStateRecord from the last hour
			//
			if (archive.m_hours.empty() == false)
			{
				std::shared_ptr<OneHourData> lastHour = archive.m_hours.rbegin()->second;
				Q_ASSERT(lastHour);

#ifdef TREND_ZERO_COPY_TREND_DATA
				std::unique_lock lock{lastHour->mutex};
#endif

				std::erase_if(lastHour->data_,
							  [timeType, trimFrom](const TrendStateRecord& record)
							  {
								  return record.states.empty() || record.states.front().getTime(timeType) >= trimFrom;
							  });

				// Remove states from the last record
				//
				if (lastHour->data_.empty() == false)
				{
					TrendStateRecord& lastRecord = lastHour->data_.back();

					std::erase_if(lastRecord.states,
								  [timeType, trimFrom](const TrendStateItem& state)
								  {
									  return state.getTime(timeType) >= trimFrom;
								  });
				}
			}
		}

		return;
	}

	void TrendSignalSet::appendRealtimeDataToArchive(QString sourceEquipmentId,
													 E::TimeType timeType,
													 E::RtTrendsSamplePeriod samplePeriod,
													 Hash signalhash,
													 const std::vector<TrendStateItem>& states)
	{
		if (states.empty() == true)
		{
			return;
		}

		if (states.empty() == false)
		{
			QMutexLocker ml(&m_lastRealtimePointsMutex);
			std::map<Hash, TrendStateItem>* realTimeLastPoints = nullptr;

			switch (timeType)
			{
			case E::TimeType::Local:
				realTimeLastPoints = &m_lastRealtimePointsLocalTime;
				break;
			case E::TimeType::System:
				realTimeLastPoints = &m_lastRealtimePointsSystemTime;
				break;
			case E::TimeType::Plant:
				realTimeLastPoints = &m_lastRealtimePointsPlantTime;
				break;
			default:
				Q_ASSERT(false);
				return;
			}

			(*realTimeLastPoints)[signalhash] = states.back();
		}

		QMutexLocker locker(&m_archiveMutex);

		std::map<TrendSignalPlusServerId, TrendArchive>* m_archive = nullptr;

		switch (timeType)
		{
		case E::TimeType::Local:
			m_archive = &m_archiveLocalTime;
			break;
		case E::TimeType::System:
			m_archive = &m_archiveSystemTime;
			break;
		case E::TimeType::Plant:
			m_archive = &m_archivePlantTime;
			break;
		default:
			Q_ASSERT(false);
			return;
		}

		for (auto& [trendSignalPlusServerId, archive] : *m_archive)
		{
			if (archive.serviceUpdateTimer.isValid() == false)
			{
				// The first start. Timer is created invalid, using anything before start() is UB.
				//
				archive.serviceUpdateTimer.start();
			}

			if (::calcHash(trendSignalPlusServerId.appSignalId) == signalhash)
			{
				if (archive.realTimeActiveServiceId == sourceEquipmentId)
				{
					// Ok, this is correct source, check if it is still valid
					//
					if (states.back().isValid() == false)
					{
						// This source has lost connections, reset active source
						//
						archive.realTimeActiveServiceId.clear();

						// Do not return, add these non valid points to the trend
						//
					}
				}
				else
				{
					if (archive.realTimeActiveServiceId.isEmpty() == true)
					{
						if (states.back().isValid() == true)
						{
							// This source has valid points, set it as active
							//
							archive.realTimeActiveServiceId = sourceEquipmentId;

							// Source is changed, we can add these points to the trend
							//
						}
						else
						{
							// This source is not valid either, ignore it
							//
							return;
						}
					}
					else
					{
						// This is the wrong source, skip it, but check timeout first
						//
						thread_local const std::array periods_ms{5ll, 10ll, 20ll, 50ll, 100ll, 250ll, 500ll, 1000ll, 5000ll, 10000ll};

						qint64 timerExpireTime = std::max(periods_ms[static_cast<size_t>(samplePeriod)] * 2ll + 500ll,
														  2000ll); // twice sample period or 2 seconds if sample period is small

						if (archive.serviceUpdateTimer.hasExpired(timerExpireTime) == true)
						{
							// We have not received from the active server data some time,
							// switch to another server
							//
							archive.realTimeActiveServiceId = sourceEquipmentId;
							// qDebug() << "TrendSignalSet::appendRealtimeDataToArchive " << sourceEquipmentId << " is now active";
						}
						else
						{
							// This is the wrong source and data is coming for active connection (to timeout)
							//
							return;
						}
					}
				}

				archive.serviceUpdateTimer.restart();

				// Add real-time data to all signals with the same AppSignalID, no matter which archive server it is from
				//
				TimeStamp lastHourTime{0};
				std::shared_ptr<OneHourData> hourData;

				for (const TrendStateItem& state : states)
				{
					TimeStamp ts = state.getTime(timeType).roundedToHour();
					if (ts == TimeStamp{0})
					{
						qDebug() << "TrendSignalSet::appendRealtimeDataToArchive: Received wrong timestamp: " << ts.timeStamp << ", "
								 << timeType;
						continue;
					}

					if (lastHourTime == ts)
					{
						Q_ASSERT(hourData);
					}
					else
					{
						hourData = archive.m_hours[ts];

						if (hourData == nullptr) // Just created
						{
							hourData = std::make_shared<TrendLib::OneHourData>();
							archive.m_hours[ts] = hourData;
						}

						lastHourTime = ts;
					}

#ifdef TREND_ZERO_COPY_TREND_DATA
					std::unique_lock lock{hourData->mutex};
#endif

					hourData->state_ = TrendLib::OneHourData::State::Received;

					if (hourData->data_.empty() == true)
					{
						TrendLib::TrendStateRecord& record = hourData->data_.emplace_back();
						record.states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
					}
					else
					{
						TrendLib::TrendStateRecord& lastRecord = hourData->data_.back();

						if (lastRecord.states.size() >= TrendLib::TrendStateRecord::RecomendedSize)
						{
							TrendLib::TrendStateRecord& record = hourData->data_.emplace_back();
							record.states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
						}
					}

					// Add state
					//
					TrendLib::TrendStateRecord& recordToAddState = hourData->data_.back();
					recordToAddState.states.push_back(state);
				}
			}
		}

		return;
	}

} // namespace TrendLib
