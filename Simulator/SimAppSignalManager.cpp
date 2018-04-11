#include "SimAppSignalManager.h"
#include "Simulator.h"
#include "../Proto/serialization.pb.h"

namespace Sim
{

	AppSignalManager::AppSignalManager(Simulator* simulator, QObject* /*parent*/) :
		Sim::Output("AppSignalManager"),
		m_simulator(simulator)
	{
		assert(m_simulator);

		return;
	}

	AppSignalManager::~AppSignalManager()
	{
		reset();
	}

	void AppSignalManager::reset()
	{
		{
			QWriteLocker wl(&m_signalParamLock);

			m_signalParams.clear();
			m_signalParamsExt.clear();
		}

		{
			QWriteLocker wl(&m_ramLock);

			m_ram.clear();
		}

		return;
	}

	bool AppSignalManager::load(QString fileName)
	{
		reset();

		// Open, read, decompress file
		//
		QFile file(fileName);
		bool ok = file.open(QIODevice::ReadOnly);

		if (ok == false)
		{
			writeError(QString("Cannot open file %1, error %2 ").arg(fileName).arg(file.errorString()));
			return false;
		}

		QByteArray data = file.readAll();
		QByteArray uncompressedData = qUncompress(data);

		::Proto::AppSignalSet message;
		ok = message.ParseFromArray(uncompressedData.constData(), uncompressedData.size());

		if (ok == false)
		{
			writeError(QString("Cannot parse file %1").arg(fileName));
			return false;
		}

		std::unordered_map<Hash, AppSignalParam> signalParams;
		std::unordered_map<Hash, Signal> signalParamsExt;

		signalParams.reserve(message.appsignal_size());
		signalParamsExt.reserve(message.appsignal_size());

		for (int i = 0; i < message.appsignal_size(); ++i)
		{
			const ::Proto::AppSignal& signalMessage = message.appsignal(i);

			// Load AppSignalParam
			//
			Hash hash = signalMessage.calcparam().hash();

			ok &= signalParams[hash].load(signalMessage);
			signalParamsExt[hash].serializeFrom(signalMessage);
		}

		if (ok == false)
		{
			writeError(QString("Cannot load Proto::AppSignal"));
			return false;
		}

		// Assign data;
		//
		{
			QWriteLocker wl(&m_signalParamLock);

			std::swap(signalParams, m_signalParams);
			std::swap(signalParamsExt, m_signalParamsExt);
		}

		return ok;
	}

	void AppSignalManager::setData(QString equipmentId, const Sim::Ram& ram)
	{
		QWriteLocker wl(&m_ramLock);

		m_ram[equipmentId] = ram;

		return;
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
		QString appSignalId;
		QString logicModuleId;
		Address16 ualAddress{};
		E::SignalType type{};
		E::ByteOrder byteOrder{};
		E::DataFormat analogDataFormat{};
		int dataSize{};

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

			const Signal& s = it->second;
			assert(signalHash == s.hash());

			appSignalId = s.appSignalID();
			logicModuleId = s.equipmentID();	int to_do_set_logic_module_equipement_id_instead_of_just_id;
			ualAddress = s.ualAddr();
			type = s.signalType();
			byteOrder = s.byteOrder();
			analogDataFormat = s.dataFormat();
			dataSize = s.dataSize();
		}

		// Get data from memory
		//
		{
			QReadLocker rl(&m_ramLock);

			auto it = m_ram.find(logicModuleId);

			if (found != nullptr)
			{
				*found = it != m_ram.end();
			}

			if (it == m_ram.end())
			{
				return state;
			}

			const Ram& ram = it->second;

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
									if (bool ok = ram.readFloat(ualAddress.offset(), &data, byteOrder);
										ok == false)
									{
										writeError(QString("Get signal state error, AppSignlaId: %1, LogicModule %2")
												   .arg(appSignalId)
												   .arg(logicModuleId));
									}
									else
									{
										state.m_flags.valid = true;
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
									if (bool ok = ram.readSignedInt(ualAddress.offset(), &data, byteOrder);
										ok == false)
									{
										writeError(QString("Get signal state error, AppSignlaId: %1, LogicModule %2")
												   .arg(appSignalId)
												   .arg(logicModuleId));
									}
									else
									{
										state.m_flags.valid = true;
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
					bool ok = ram.readBit(ualAddress.offset(), ualAddress.bit(), &data, byteOrder);

					if (ok == false)
					{
						writeError(QString("Get signal state error, AppSignlaId: %1, LogicModule %2")
										.arg(appSignalId)
										.arg(logicModuleId));
					}
					else
					{
						state.m_flags.valid = true;
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

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found) const
	{
		return signalState(::calcHash(appSignalId), found);
	}

	void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	{
		int to_do_signalState;

		// To Do
		//
		assert(false);
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


}
