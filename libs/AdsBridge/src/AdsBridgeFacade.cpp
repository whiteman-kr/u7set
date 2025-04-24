#include "AdsBridgeFacade.h"
#include "AdsBridgeLogFile.h"

#include "../../OnlineLib/SoftwareSettings.h"
#include "../../OnlineLib/TcpClientStatistics.h"
#include "../../UtilsLib/XmlHelper.h"

#include <ClientLib/AdsConnection.h>
#include <ClientLib/AppSignalManager.h>

#include <span>

namespace AdsBridge
{
	AdsBridgeFacade::AdsBridgeFacade(ILogFile* log) :
		m_log{log},
		m_signals{std::make_unique<ClientLib::AppSignalManager>(m_log)},
		m_adsConnection{std::make_unique<ClientLib::AdsConnection>(*m_signals.get(), m_signals.get(), m_log)}
	{
		log->writeMessage("AdsBridgeFacade::AdsBridgeFacade()");
	}

	AdsBridgeFacade::~AdsBridgeFacade()
	{
		m_log->writeMessage("AdsBridgeFacade::~AdsBridgeFacade()");
	}

	bool AdsBridgeFacade::setConfiguration(const QByteArray& data, QString profile /*= SettingsProfile::DEFAULT*/)
	{
		bool result = true;

		// Read Xml data and get software info.
		//
		XmlReadHelper xmlReader{data};
		xmlReader.findElement(XmlElement::SOFTWARE);

		if (xmlReader.checkElement(XmlElement::SOFTWARE) == false)
		{
			m_log->writeError(QString{"AdsBridgeFacade::setConfiguration(), Invalid XML data %1."}.arg(XmlElement::SOFTWARE));
			return false;
		}

		int typeInt = 0;
		QString caption;
		QString equipmentId;
		QString controllersIds;
		QStringList softwareControllersIds;

		result &= xmlReader.readStringAttribute(XmlAttribute::CAPTION, &caption);
		result &= xmlReader.readStringAttribute(XmlAttribute::EQUIPMENT_ID, &equipmentId);
		result &= xmlReader.readIntAttribute(XmlAttribute::TYPE, &typeInt);

		result &= xmlReader.readStringAttribute(XmlAttribute::SOFTWARE_CONTROLLERS, &controllersIds);
		softwareControllersIds = controllersIds.split(Separator::COMMA, Qt::SkipEmptyParts);

		if (result == false)
		{
			m_log->writeError("AdsBridgeFacade::setConfiguration(), Failed to read software XML.");
			return false;
		}

		if (static_cast<E::SoftwareType>(typeInt) != E::SoftwareType::AdsBridge)
		{
			m_log->writeError("AdsBridgeFacade::setConfiguration(), Wrong software type, AdsBridge is expected.");
			return false;
		}

		// Read settings from XML.
		//
		SoftwareSettingsSet settingsSet;
		settingsSet.setSoftwareType(E::SoftwareType::AdsBridge);

		result = settingsSet.readFromXml(data);
		if (result == false)
		{
			m_log->writeError("AdsBridgeFacade::setConfiguration(), Failed to read configuration XML.");
			return false;
		}

		result = settingsSet.settingsProfileIsExists(profile);
		if (result == false)
		{
			m_log->writeError(QString{"AdsBridgeFacade::setConfiguration(), Profile %1 not found."}.arg(profile));
			return false;
		}

		auto settings = settingsSet.getSettingsProfile<AdsBridgeSettings>(profile);
		if (settings == nullptr)
		{
			Q_ASSERT(settings);
			m_log->writeError(QString{"AdsBridgeFacade::setConfiguration(), Profile %1 not found."}.arg(profile));
			return false;
		}

		// Parse XML for getting the equipment ID ().
		//
		setEquipmentId(equipmentId);

		// Add connections.
		//
		m_appDataServices.assign(begin(settings->appDataServices), end(settings->appDataServices));

		return result;
	}

	void AdsBridgeFacade::addAppDataService(const QString& adsEquipmentId, const QString& address, int port)
	{
		m_log->writeMessage(
			QString("AdsBridgeFacade::addAppDataService(), Adding AppDataService: %1, %2:%3").arg(adsEquipmentId).arg(address).arg(port));

		SoftwareEndpoint::AppDataService ads{.equipmentId = adsEquipmentId,
											 .shortenId = adsEquipmentId,
											 .address = HostAddressPort{address, port}};

		m_appDataServices.push_back(ads);

		return;
	}

	void AdsBridgeFacade::clearAppDataServices()
	{
		m_log->writeMessage("AdsBridgeFacade::clearAppDataServices()");
		m_appDataServices.clear();
		return;
	}

	void AdsBridgeFacade::connect()
	{
		if (m_equipmentId.isEmpty() == true)
		{
			m_log->writeError("AdsBridgeFacade::connect(), EquipmentID is empty.");
			return;
		}

		if (m_appDataServices.empty() == true)
		{
			m_log->writeWarning("AdsBridgeFacade::connect(), No AppDataService(s) to connect to.");
		}

		m_log->writeMessage(QString("AdsBridgeFacade::connect(), Connecting %1 AppDataService(s)...").arg(m_appDataServices.size()));

		SoftwareInfo si{E::SoftwareType::Monitor, m_equipmentId};
		m_adsConnection->updateConnections(si, m_appDataServices);

		return;
	}

	void AdsBridgeFacade::close()
	{
		m_log->writeMessage("AdsBridgeFacade::close()");

		SoftwareInfo si{E::SoftwareType::Monitor, m_equipmentId};
		m_adsConnection->updateConnections(si, {});

		return;
	}

	const QString& AdsBridgeFacade::equipmentId() const
	{
		return m_equipmentId;
	}

	void AdsBridgeFacade::setEquipmentId(const QString& equipmentId)
	{
		m_equipmentId = equipmentId;

		m_log->writeMessage("AdsBridgeFacade::setEquipmentId(), EquipmentID set to: " + equipmentId);
		return;
	}

	bool AdsBridgeFacade::signalParamsLoaded() const
	{
		return m_adsConnection->signalParamsLoaded();
	}

	bool AdsBridgeFacade::signalStatesLoaded() const
	{
		return m_adsConnection->signalStatesLoaded();
	}

	size_t AdsBridgeFacade::signalCount() const
	{
		if (signalParamsLoaded() == false)
		{
			m_log->writeWarning("AdsBridgeFacade::signalCount(): Signal parameters not fully loaded when signalCount() is called.");
		}

		return m_signals->signalsCount();
	}

	bool AdsBridgeFacade::signalList(MatsSignalHash* out, size_t count)
	{
		static_assert(sizeof(MatsSignalHash) == sizeof(Hash));

		if (out == nullptr)
		{
			m_log->writeError("AdsBridgeFacade::signalList(), out is nullptr.");
			return false;
		}

		auto hashes = m_signals->signalHashes();
		if (hashes.size() != count)
		{
			m_log->writeError(QString("AdsBridgeFacade::signalList(), count is invalid, expected %1.").arg(hashes.size()));
			return false;
		}

		std::memcpy(out, hashes.data(), hashes.size() * sizeof(MatsSignalHash));
		return true;
	}

	size_t AdsBridgeFacade::signalParams(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalParam* out, size_t count)
	{
		static_assert(sizeof(MatsSignalHash) == sizeof(Hash));

		if (structSize != sizeof(MatsAppSignalParam))
		{
			m_log->writeError("AdsBridgeFacade::signalParams(), structSize is invalid.");
			return 0;
		}

		if (signalHashes == nullptr)
		{
			m_log->writeError("AdsBridgeFacade::signalParams(), signalHashes is nullptr.");
			return 0;
		}

		if (out == nullptr)
		{
			m_log->writeError("AdsBridgeFacade::signalParams(), out is nullptr.");
			return 0;
		}

		std::memset(out, 0, sizeof(MatsAppSignalParam) * count);

		int result = 0;

		for (const auto& h : std::span{signalHashes, count})
		{
			bool signalFound = false;
			auto signalParam = m_signals->signalParam(h, &signalFound);

			MatsAppSignalParam& outSignal = *out;
			outSignal.hash = h;

			if (signalFound == false)
			{
				m_log->writeWarning(QString("AdsBridgeFacade::signalParams(), Signal not found: %1").arg(h));
				result = false;
			}
			else
			{
				result++;

				outSignal.appSignalId = getStringConstPointer(signalParam.appSignalId());
				outSignal.customSignalId = getStringConstPointer(signalParam.customSignalId());

				outSignal.caption = getStringConstPointer(signalParam.caption());
				outSignal.equipmentId = getStringConstPointer(signalParam.equipmentId());
				outSignal.lmEquipmentId = getStringConstPointer(signalParam.lmEquipmentId());
				outSignal.units = getStringConstPointer(signalParam.units());
				outSignal.tags = getStringConstPointer(signalParam.tagStringList().join(QChar(' ')));

				outSignal.channel = static_cast<MatsChannel>(signalParam.channel());
				outSignal.inOutType = static_cast<MatsSignalInOutType>(signalParam.inOutType());
				outSignal.type = static_cast<MatsSignalType>(signalParam.type());
				outSignal.decimalPlaces = signalParam.precision();

				outSignal.lowValidRange = signalParam.lowValidRange();
				outSignal.highValidRange = signalParam.highValidRange();

				outSignal.tuning = signalParam.enableTuning();
			}

			out++;
		}

		return result;
	}

	size_t AdsBridgeFacade::signalStates(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalState* out, size_t count)
	{
		static_assert(sizeof(MatsSignalHash) == sizeof(Hash));

		if (structSize != sizeof(MatsAppSignalState))
		{
			m_log->writeError("AdsBridgeFacade::signalStates(), structSize is invalid.");
			return 0;
		}

		if (signalHashes == nullptr)
		{
			m_log->writeError("AdsBridgeFacade::signalStates(), signalHashes is nullptr.");
			return 0;
		}

		if (out == nullptr)
		{
			m_log->writeError("AdsBridgeFacade::signalStates(), out is nullptr.");
			return 0;
		}

		std::memset(out, 0, sizeof(MatsAppSignalState) * count);

		int result = 0;

		for (const auto& h : std::span{signalHashes, count})
		{
			bool signalFound = false;
			auto signalState = m_signals->signalState(h, &signalFound);

			MatsAppSignalState& outSignal = *out;
			outSignal.hash = h;

			if (signalFound == false)
			{
				m_log->writeWarning(QString("AdsBridgeFacade::signalStates(), Signal not found: %1").arg(h));
				result = false;
			}
			else
			{
				result++;
				// plantTime is need to be corrected by TZ and DST, as it's in local time.
				//
				auto localTimeCorrection = QDateTime::currentDateTime().offsetFromUtc() * 1000;
				outSignal.plantTime = signalState.m_time.plant.timeStamp ? signalState.m_time.plant.timeStamp - localTimeCorrection : 0ull;
				outSignal.serverTime = signalState.m_time.system.timeStamp;
				outSignal.value = signalState.m_value;
				outSignal.flags = signalState.m_flags.all;
			}

			out++;
		}

		return result;
	}

	size_t AdsBridgeFacade::connectionCount() const
	{
		return TcpClientStatistics::statistics().size();
	}

	bool AdsBridgeFacade::connectionStatus(size_t structSize, struct AdsConnectionStatus* out, size_t count) const
	{
		if (structSize != sizeof(AdsConnectionStatus))
		{
			m_log->writeError("AdsBridgeFacade::connectionStatus(), structSize is invalid.");
			return false;
		}

		if (out == nullptr)
		{
			m_log->writeError("AdsBridgeFacade::connectionStatus(), out is nullptr.");
			return false;
		}

		auto stats = TcpClientStatistics::statistics();
		if (stats.size() != count)
		{
			m_log->writeError(QString("AdsBridgeFacade::connectionStatus(), count is invalid, expected %1.").arg(stats.size()));
			return false;
		}

		for (const auto& s : TcpClientStatistics::statistics())
		{
			AdsConnectionStatus& state = *out;

			state.id = s.id;
			state.status = s.state.isConnected;
			state.setConnectionResult = static_cast<::MatsConnectionResult>(s.state.setConnectionResult);
			state.connectionType = getStringConstPointer(s.objectName);
			state.port = s.state.peerAddr.port();
			state.address = getStringConstPointer(s.state.peerAddr.addressStr());
			state.adsEquipmentId = getStringConstPointer(s.state.connectedSoftwareInfo.equipmentID());
			state.received = s.state.receivedBytes;
			state.sent = s.state.sentBytes;
			state.requestCount = s.state.requestCount;
			state.replyCount = s.state.replyCount;

			++out;
		}

		return true;
	}

	const char* AdsBridgeFacade::getStringConstPointer(const QString& string) const
	{
		std::string str = string.toStdString();

		// Shared lock for reading. If the string is found, return the pointer.
		//
		{
			std::shared_lock lock(m_stringTableMutex);

			auto it = m_stringTable.find(str);
			if (it != m_stringTable.end())
			{
				return it->c_str();
			}
		}

		// Th string was not found, insert new string with unique lock
		//
		std::unique_lock lock{m_stringTableMutex};
		auto r = m_stringTable.insert(str);
		return r.first->c_str();
	}

} // namespace AdsBridge