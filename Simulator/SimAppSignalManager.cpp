#include "SimAppSignalManager.h"
#include "Simulator.h"
#include "../Proto/serialization.pb.h"


namespace Sim
{

	bool FlagsReadStruct::create(const AppSignal& s, const std::unordered_map<Hash, AppSignal>& signalParams, ScopedLog& log)
	{
		auto flagIt = s.stateFlagsSignals().constBegin();

		while (flagIt != s.stateFlagsSignals().constEnd())
		{
			E::AppSignalStateFlagType flagType = flagIt.key();
			const QString& flagAppSignalId = flagIt.value();
			Hash flagSignalHash = ::calcHash(flagAppSignalId);

			if (auto flagSignalIt = signalParams.find(flagSignalHash);
				flagSignalIt == signalParams.end())
			{
				// Error, flag signal is not found
				//
				log.writeError(QObject::tr("AppSignalManager::signalFlags(%1) cannot find flag signal %2")
							   .arg(s.appSignalID())
							   .arg(flagAppSignalId));

				Q_ASSERT(flagSignalIt != signalParams.end());
				return {};
			}
			else
			{
				const AppSignal& flagSignal = flagSignalIt->second;
				Q_ASSERT(flagSignal.hash() == flagSignalHash);

				if (flagSignal.isDiscrete() == false)
				{
					log.writeError(QObject::tr("AppSignalManager::signalFlags(%1) flag is not discrete signal, flag signal %2")
								   .arg(s.appSignalID())
								   .arg(flagAppSignalId));
					Q_ASSERT(flagSignal.isDiscrete());
					return {};
				}

				if (flagSignal.isConst() == true)
				{
					flagsConsts[flagConstsCount] = std::make_pair(flagType, static_cast<quint32>(flagSignal.constValue()));
					flagConstsCount ++;
				}
				else
				{
					Address16 flagAddress = flagSignal.ualAddr();

					if (flagAddress.isValid() == false)
					{
						log.writeError(QObject::tr("AppSignalManager::signalFlags(%1) invalid flag signal address, flag signal %2")
									   .arg(s.appSignalID())
									   .arg(flagAppSignalId));
						Q_ASSERT(flagAddress.isValid());
						return {};
					}
					else
					{
						flagsSignalAddresses[flagCount] = std::make_pair(flagType, flagAddress);
						flagCount ++;
					}
				}
			}

			// --
			//
			++flagIt;
		}

		return true;
	}

	AppSignalStateFlags FlagsReadStruct::signalFlags(const Ram& ram, ScopedLog& log) const
	{
		AppSignalStateFlags result{};
		bool hasValidity = false;

		// Read flags value from memory
		//
		for (size_t i = 0; i < flagCount; i++)
		{
			const auto& p = flagsSignalAddresses[i];
			E::AppSignalStateFlagType flagType = p.first;
			const Address16& flagAddress = p.second;

			quint16 flagValue = 0;

			if (bool readOk = ram.readBit(flagAddress.offset(), static_cast<quint16>(flagAddress.bit()), &flagValue, E::ByteOrder::BigEndian);
				readOk == false)
			{
				log.writeError(QObject::tr("AppSignalManager::signalFlags error read flag signal by address %1")
							   .arg(flagAddress.toString()));

				Q_ASSERT(readOk);
				return result;
			}

			result.setFlag(flagType, flagValue);

			if (flagType == E::AppSignalStateFlagType::Validity)
			{
				hasValidity = true;
			}
		}

		// Some flags can be constant values, write consts to flag
		//
		for (size_t i = 0; i < flagConstsCount; i++)
		{
			const auto& p = flagsConsts[i];
			result.setFlag(p.first, p.second);

			if (p.first == E::AppSignalStateFlagType::Validity)
			{
				hasValidity = true;
			}
		}

		if (hasValidity == false)
		{
			result.valid = 1;		// All signals which do not have explicit validty signal are accounted as valid
		}

		return result;
	}

	//
	//
	//	AppSignalManager
	//
	//
	AppSignalManager::AppSignalManager(Simulator* simulator, QObject* /*parent*/) :
		m_simulator(simulator),
		m_log(simulator->log(), "AppSignalManager")
	{
		assert(m_simulator);

		return;
	}

	AppSignalManager::~AppSignalManager()
	{
		resetAll();
	}

	QString AppSignalManager::ramDump(QString logicModuleId) const
	{
		QReadLocker rl(&m_ramLock);

		auto ramIt = m_ram.find(::calcHash(logicModuleId));
		if (ramIt != m_ram.end())
		{
			return ramIt->second.dump(logicModuleId);
		}
		else
		{
			return {};
		}
	}

	void AppSignalManager::resetAll()
	{
		resetSignalParam();
		resetRam();
		return;
	}

	void AppSignalManager::resetSignalParam()
	{
		{
			QWriteLocker wl(&m_signalParamLock);

			m_signalParams.clear();
			m_signalParamsExt.clear();
			m_customToAppSignalId.clear();
		}

		{
			QWriteLocker wl(&m_ramLock);
			m_flagsStruct.clear();
		}

		return;
	}

	void AppSignalManager::resetRam()
	{
		QWriteLocker wl(&m_ramLock);

		m_ram.clear();
		m_ramTimes.clear();

		return;
	}

	bool AppSignalManager::load(QString fileName)
	{
		resetAll();

		// Open, read, decompress file
		//
		QFile file(fileName);
		bool ok = file.open(QIODevice::ReadOnly);

		if (ok == false)
		{
			m_log.writeError(QString("Cannot open file %1, error %2 ").arg(fileName).arg(file.errorString()));
			return false;
		}

		QByteArray data = file.readAll();
		QByteArray uncompressedData = qUncompress(data);

		::Proto::AppSignalSet message;
		ok = message.ParseFromArray(uncompressedData.constData(), uncompressedData.size());

		if (ok == false)
		{
			m_log.writeError(QString("Cannot parse file %1").arg(fileName));
			return false;
		}

		std::unordered_map<Hash, AppSignalParam> signalParams;
		std::unordered_map<Hash, AppSignal> signalParamsExt;
		std::unordered_map<Hash, Hash> customToAppSignalId;
		std::unordered_map<Hash, FlagsReadStruct> flagsStruct;

		signalParams.reserve(message.appsignal_size());
		signalParamsExt.reserve(message.appsignal_size());
		customToAppSignalId.reserve(message.appsignal_size());

		for (int i = 0; i < message.appsignal_size(); ++i)
		{
			const ::Proto::AppSignal& signalMessage = message.appsignal(i);

			// Load AppSignalParam
			//
			Hash hash = signalMessage.calcparam().hash();

			ok &= signalParams[hash].load(signalMessage);
			signalParamsExt[hash].serializeFrom(signalMessage);

			customToAppSignalId[::calcHash(signalParams[hash].customSignalId())] = hash;
		}

		for (const auto&[h, s] : signalParamsExt)
		{
			flagsStruct[h].create(s, signalParamsExt, m_log);
		}

		if (ok == false)
		{
			m_log.writeError(QString("Cannot load Proto::AppSignal"));
			return false;
		}

		// Assign data;
		//
		{
			QWriteLocker wl(&m_signalParamLock);

			std::swap(signalParams, m_signalParams);
			std::swap(signalParamsExt, m_signalParamsExt);
			std::swap(customToAppSignalId, m_customToAppSignalId);
		}

		{
			QWriteLocker wl(&m_ramLock);
			std::swap(flagsStruct, m_flagsStruct);
		}

		return ok;
	}

	void AppSignalManager::setData(const QString& equipmentId,
								   const Sim::Ram& ram,
								   TimeStamp plantTime,
								   TimeStamp localTime,
								   TimeStamp systemTime)
	{
		Hash lmHash = ::calcHash(equipmentId);

		{
			QWriteLocker wl(&m_ramLock);

			if (ram.isNull() == true)
			{
				m_ram.erase(lmHash);
			}
			else
			{
				m_ram[lmHash].updateFrom(ram);
			}

			Times tm;
			tm.system = systemTime;
			tm.local = localTime;
			tm.plant = plantTime;

			m_ramTimes[lmHash] = tm;		// keep record in m_ramTimes even for for null ram, as it is usefull for getting nonvalid point for trends
		}

		// Fetch data for realtime trends now, while this memory was not updated yet
		// Later trend will fetch it itself
		//
		{
			QMutexLocker ml(&m_trendMutex);

			if (m_trends.empty() == false)
			{
				qint64 currentTime = QDateTime::currentDateTime().toSecsSinceEpoch();

				for (auto it = m_trends.begin(); it != m_trends.end();)
				{
					Trend& trend = *it;

					if (currentTime - trend.lastAccess.toSecsSinceEpoch() >= 10)	// 10 seconds
					{
						// Remove this thend from obeserved, as it did not have fetched data for 10 seconds
						//
						it = m_trends.erase(it);
						continue;
					}

					for (TrendSignal& ts : trend.trendSignals)
					{
						if (ts.lmEquipmentIdHash == lmHash)
						{
							// Fetching appsignals states from ram is cause locking m_ramLock for read,
							// so it is nested lock m_trendMutex -> m_trendMutex.
							// Just keep it in mind and do not try to lock in other dirrection
							//
							AppSignalState& addedState = ts.states.emplace_back(this->signalState(ts.appSignalHash, nullptr));

							if (ts.states.size() > 3 &&
								addedState.hasSameValue(ts.states[ts.states.size() - 2]) == true &&
								addedState.hasSameValue(ts.states[ts.states.size() - 3]) == true)
							{
								ts.states[ts.states.size() - 2] = addedState;
								ts.states.resize(ts.states.size() - 1);		// If last 3 points have the same value, then extend 2nt to 3rd;
							}
						}
					}

					// Increment it here, as we erase some items in loop
					//
					++it;
				}
			}
		}

		return;
	}

	std::shared_ptr<TrendLib::RealtimeData> AppSignalManager::trendData(const QString& trendId,
																		const std::vector<Hash>& trendSignals,
																		TrendLib::TrendStateItem* minState,
																		TrendLib::TrendStateItem* maxState)
	{
		Q_ASSERT(minState);
		Q_ASSERT(maxState);

		minState->clear();
		maxState->clear();

		std::shared_ptr<TrendLib::RealtimeData> result;

		if (trendSignals.empty() == true)
		{
			return result;
		}

		auto createTrendSignal = [this](Hash signalHash) -> TrendSignal
		{
			std::optional<AppSignal> sp = this->signalParamExt(signalHash);

			TrendSignal ts;

			if (sp.has_value() == true)
			{
				ts.appSignalId = sp->appSignalID();
				ts.appSignalHash = signalHash;
				ts.lmEquipmentId = sp->lmEquipmentID();
				ts.lmEquipmentIdHash = ::calcHash(sp->lmEquipmentID());
				ts.states.reserve(100);
			}

			return ts;
		};

		QDateTime currentTime = QDateTime::currentDateTime();

		QMutexLocker ml(&m_trendMutex);

		bool trendIsPresent = false;
		for (Trend& smTrend : m_trends)
		{
			if (smTrend.trendId == trendId)
			{
				// Trend is found, fetch all data from buffers
				//
				if (result == nullptr)
				{
					result = std::make_shared<TrendLib::RealtimeData>();
				}

				smTrend.lastAccess = currentTime;		// Update time, so this trend will not be removed on access timeout

				for (Hash hash : trendSignals)
				{
					TrendLib::RealtimeDataChunk& chunk = result->signalData.emplace_back();

					chunk.appSignalHash = hash;

					bool trendSignalFound = false;
					for (TrendSignal& smTrendSignal : smTrend.trendSignals)
					{
						// It can happen that EquipmentId for signal has changed,
						// then we must check EquipmentId every time here (call signalParam and compare equipmentid for smTrendSignal)
						// I do not do it as very unlikely situation
						// In such cases signal must be removed from trends and added again
						//
						if (smTrendSignal.appSignalHash == hash)
						{
							// found signal, copy all states
							//
							chunk.states.reserve(smTrendSignal.states.size());

							for (const AppSignalState& s : smTrendSignal.states)
							{
								auto& addedState = chunk.states.emplace_back(TrendLib::TrendStateItem{s});

								addedState.setRealtimePointFlag();

								if (minState->system == 0 || minState->system > addedState.system)
								{
									*minState = addedState;
								}

								if (maxState->system == 0 || maxState->system < addedState.system)
								{
									*maxState = addedState;
								}
							}

							smTrendSignal.states.clear();

							trendSignalFound = true;
							break;
						}
					}

					// Signal not found, add it to observations
					//
					if (trendSignalFound == false)
					{
						smTrend.trendSignals.emplace_back(createTrendSignal(hash));
					}
				}

				trendIsPresent = true;
				break;
			}
		}

		if (trendIsPresent == false)
		{
			// Add trend to realtime observe
			//
			Trend& trend = m_trends.emplace_back();

			trend.trendId = trendId;
			trend.lastAccess = QDateTime::currentDateTime();

			trend.trendSignals.reserve(trendSignals.size());

			for (Hash hash : trendSignals)
			{
				trend.trendSignals.emplace_back(createTrendSignal(hash));
			}
		}

		return result;
	}

	std::optional<AppSignal> AppSignalManager::signalParamExt(const QString& appSignalId) const
	{
		return signalParamExt(::calcHash(appSignalId));
	}

	std::optional<AppSignal> AppSignalManager::signalParamExt(Hash hash) const
	{
		QReadLocker locker(&m_signalParamLock);

		auto it = m_signalParamsExt.find(hash);
		if (it == m_signalParamsExt.end())
		{
			return {};
		}
		else
		{
			return it->second;
		}
	}

	Hash AppSignalManager::customToAppSignal(Hash customSignalHash) const
	{
		QReadLocker locker(&m_signalParamLock);

		auto it = m_customToAppSignalId.find(customSignalHash);
		if (it == m_customToAppSignalId.end())
		{
			return UNDEFINED_HASH;
		}
		else
		{
			return it->second;
		}
	}

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found, bool applyOverride) const
	{
		return signalState(::calcHash(appSignalId), found, applyOverride);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found, bool applyOverride) const
	{
		QString appSignalId;
		QString logicModuleId;
		Address16 actualAddress{};
		E::LogicModuleRamAccess ramAccess;
		E::SignalType type{};
		E::ByteOrder byteOrder{};
		E::DataFormat analogDataFormat{};
		int dataSize{};
		bool isConst{};
		double constValue{};

		Hash logicModuleHash = UNDEFINED_HASH;

		AppSignalState state;
		state.m_hash = signalHash;

		{
			QReadLocker rl(&m_signalParamLock);

			auto it = m_signalParamsExt.find(signalHash);

			if (found != nullptr)
			{
				*found = it != m_signalParamsExt.end();
			}

			if (it == m_signalParamsExt.end())
			{
				return state;
			}

			const AppSignal& s = it->second;
			Q_ASSERT(signalHash == s.hash());

			if (s.signalType() == E::SignalType::Bus)
			{
				// s.dataFormat() will crash for Bus signal
				// and it is not possible to return something reasonable for such signals
				//
				return state;
			}

			appSignalId = s.appSignalID();
			logicModuleId = s.lmEquipmentID();
			logicModuleHash = ::calcHash(logicModuleId);
			actualAddress = s.actualAddr();
			ramAccess = s.lmRamAccess();
			type = s.signalType();
			byteOrder = s.byteOrder();
			analogDataFormat = s.dataFormat();
			dataSize = s.dataSize();
			isConst = s.isConst();
			constValue = s.constValue();
		}

		// Get data from memory
		//
		{
			QReadLocker rl(&m_ramLock);

			// Get time for this ram
			//
			auto timeIt = m_ramTimes.find(logicModuleHash);
			if (timeIt != m_ramTimes.end())
			{
				state.m_time = timeIt->second;
			}

			// Get ram
			//
			auto ramIt = m_ram.find(logicModuleHash);

			if (found != nullptr)
			{
				*found = (ramIt != m_ram.end());
			}

			if (ramIt == m_ram.end())
			{
				return state;
			}

			const Ram& ram = ramIt->second;

			// --
			//

			if (m_simulator->logicModule(logicModuleId)->isPowerOff() == true)
			{
				// Time stamp is already set set
				//
				return state;
			}

			// Set flags
			//
			if (auto flagIt = m_flagsStruct.find(signalHash);
				flagIt != m_flagsStruct.end())
			{
				state.m_flags = flagIt->second.signalFlags(ram, m_log);
			}

			if (m_simulator->isStopped() == true)
			{
				state.m_flags.all = 0;		// It resets stateAvailable and all othe flags
			}
			else
			{
				state.m_flags.stateAvailable = 1;
			}

			// If signal is optimized to const then just set its' value
			//
			if (isConst == true)
			{
				state.m_value = constValue;
				return state;
			}

			if (actualAddress.isValid() == false)
			{
				state.m_flags.all = 0;
				// This is can be unused signal, in this case it has non valid addresses
				//
				return state;
			}

			// Read and set value
			//
			switch (type)
			{
			case E::SignalType::Analog:
				{
					//E::ByteOrder byteOrder{};
					//E::DataFormat analogDataFormat{};
					//int dataSize{};

					switch (analogDataFormat)
					{
					case E::DataFormat::Float:
						{
							switch (dataSize)
							{
							case 32:
								{
									float data = 0;
									if (bool ok = ram.readFloat(actualAddress.offset(), &data, byteOrder, ramAccess, applyOverride);
										ok == false)
									{
										m_log.writeError(QString("Get signal state error, AppSignlaId: %1, LogicModule %2")
														 .arg(appSignalId)
														 .arg(logicModuleId));
									}
									else
									{
										state.m_value = data;
									}
								}
								break;
							default:
								assert(false);
							}
						}
						break;
					case E::DataFormat::SignedInt:
						{
							switch (dataSize)
							{
							case 32:
								{
									qint32 data = 0;
									if (bool ok = ram.readSignedInt(actualAddress.offset(), &data, byteOrder, ramAccess, applyOverride);
										ok == false)
									{
										m_log.writeError(QString("Get signal state error, AppSignlaId: %1, LogicModule %2")
														 .arg(appSignalId)
														 .arg(logicModuleId));
									}
									else
									{
										state.m_value = data;
									}
								}
								break;
							default:
								assert(false);
							}
						}
						break;
					case E::DataFormat::UnsignedInt:
						assert(false);
						break;
					default:
						assert(false);
					}
				}
				break;
			case E::SignalType::Discrete:
				{
					quint16 data = 0;
					bool ok = ram.readBit(actualAddress.offset(), static_cast<quint16>(actualAddress.bit()),
										  &data, byteOrder, ramAccess, applyOverride);

					if (ok == false)
					{
						m_log.writeError(QString("Get signal state error, AppSignlaId: %1, LogicModule %2")
										 .arg(appSignalId)
										 .arg(logicModuleId));
					}
					else
					{
						state.m_value = data;
					}
				}
				break;
			case E::SignalType::Bus:
				assert(false);	// To do
				break;
			default:
				assert(false);
			}
		}

		return state;
	}

	bool AppSignalManager::getUpdateForRam(const QString& equipmentId, Sim::Ram* ram) const
	{
		Q_ASSERT(ram);

		Hash lmHash = ::calcHash(equipmentId);

		{
			QReadLocker wl(&m_ramLock);

			auto it = m_ram.find(lmHash);

			if (it == m_ram.end())
			{
				return false;
			}

			ram->updateFrom(it->second);
		}

		return true;
	}

	std::vector<AppSignalParam> AppSignalManager::signalList() const
	{
		std::vector<AppSignalParam> result;

		{
			QReadLocker rl(&m_signalParamLock);

			result.reserve(m_signalParams.size());

			for (const auto&[hash, sp] : m_signalParams)
			{
				assert(hash == sp.hash());
				Q_UNUSED(hash);

				result.push_back(sp);
			}
		}

		return result;
	}

	bool AppSignalManager::signalExists(Hash hash) const
	{
		QReadLocker rl(&m_signalParamLock);
		return m_signalParams.find(hash) != m_signalParams.end();
	}

	bool AppSignalManager::signalExists(const QString& appSignalId) const
	{
		return signalExists(::calcHash(appSignalId));
	}

	AppSignalParam AppSignalManager::signalParam(Hash signalHash, bool* found) const
	{
		QReadLocker rl(&m_signalParamLock);

		auto it = m_signalParams.find(signalHash);

		if (found != nullptr)
		{
			*found = it != m_signalParams.end();
		}

		if (it == m_signalParams.end())
		{
static const AppSignalParam dummy;
			return dummy;
		}

		return it->second;
	}

	AppSignalParam AppSignalManager::signalParam(const QString& appSignalId, bool* found) const
	{
		return signalParam(::calcHash(appSignalId), found);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found) const
	{
		return signalState(signalHash, found, true);
	 }

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found) const
	{
		return signalState(::calcHash(appSignalId), found);
	}

	void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	{
		// This function must be optimized, signalState every times locks/unlocks/locks/unlock...
		//
		if (result == nullptr)
		{
			Q_ASSERT(result);
			return;
		}

		result->clear();
		result->reserve(appSignalHashes.size());

		int foundCount = 0;

		for (Hash signalHash: appSignalHashes)
		{
			bool foundHash = false;
			result->emplace_back(signalState(signalHash, &foundHash));

			if (foundHash == true)
			{
				foundCount ++;
			}
		}

		if (found != nullptr)
		{
			*found = foundCount;
		}

		return;
	}

	void AppSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
	{
		std::vector<Hash> appSignalHashes;
		appSignalHashes.reserve(appSignalIds.size());

		for (const QString& s : appSignalIds)
		{
			appSignalHashes.push_back(::calcHash(s));
		}

		return signalState(appSignalHashes, result, found);
	}

	QStringList AppSignalManager::signalTags(Hash /*signalHash*/) const
	{
		Q_ASSERT(false);		// TO DO
		return {};
	}

	QStringList AppSignalManager::signalTags(const QString& appSignalId) const
	{
		return signalTags(::calcHash(appSignalId));
	}

	bool AppSignalManager::signalHasTag(Hash /*signalHash*/, const QString& /*tag*/) const
	{
		Q_ASSERT(false);		// TO DO
		return {};
	}

	bool AppSignalManager::signalHasTag(const QString& appSignalId, const QString& tag) const
	{
		return signalHasTag(::calcHash(appSignalId), tag);
	}

	QString AppSignalManager::equipmentToAppSiganlId(const QString& /*equipmentId*/) const
	{
		Q_ASSERT(false);	// to do
		return {};
	}

	std::vector<std::shared_ptr<Comparator>> AppSignalManager::setpointsByInputSignalId(const QString& /*appSignalId*/) const
	{
		//int todo_setpointsByInputSignalId = 0;
		//Q_ASSERT(false);		// TO DO
		return {};
	}

	const Simulator* AppSignalManager::simulator() const
	{
		return m_simulator;
	}

	Simulator* AppSignalManager::simulator()
	{
		return m_simulator;
	}


	//
	//	ScriptAppSignalManager
	//
	ScriptAppSignalManager::ScriptAppSignalManager(const IAppSignalManager* appSignalManager, QObject* parent) :
		QObject(parent),
		m_appSignalManager(appSignalManager)
	{
		assert(m_appSignalManager);
	}

	QJSValue ScriptAppSignalManager::signalParam(QString signalId) const
	{
		return signalParam(::calcHash(signalId));
	}

	QJSValue ScriptAppSignalManager::signalParam(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;
		AppSignalParam s = m_appSignalManager->signalParam(signalHash, &ok);

		if (ok == false)
		{
			return {};
		}

		QJSEngine* engine = qjsEngine(this);

		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		return engine->toScriptValue(s);
	}

	QJSValue ScriptAppSignalManager::signalState(QString signalId) const
	{
		return signalState(::calcHash(signalId));
	}

	QJSValue ScriptAppSignalManager::signalState(Hash signalHash) const
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return {};
		}

		bool ok = false;
		AppSignalState s = m_appSignalManager->signalState(signalHash, &ok);

		if (ok == false)
		{
			return {};
		}

		QJSEngine* engine = qjsEngine(this);

		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		return engine->toScriptValue(s);
	}


}
