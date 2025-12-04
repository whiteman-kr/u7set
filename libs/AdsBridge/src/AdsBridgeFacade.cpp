#include "AdsBridgeFacade.h"
#include "AdsBridgeLogFile.h"
#include "version.h"

#include <AdsConnectionLib/AdsConnection.h>
#include <AdsConnectionLib/ClientConnStatsStd.h>

#include "pugixml.hpp"

#include <format>
#include <span>

#if defined(_WIN32)
	#include <Lmcons.h> // UNLEN, etc.
	#include <Windows.h>
#else
	#include <pwd.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

namespace
{
	const int AdsBridgeSoftwareType = 9013;

	std::string hostname()
	{
		std::array<char, 256> buf{};

#if defined(_WIN32)
		DWORD size = static_cast<DWORD>(buf.size());

		if (GetComputerNameA(buf.data(), &size))
		{
			buf.back() = '\0'; // Ensure null-termination
			return std::string(buf.data(), size);
		}
#else
		if (gethostname(buf.data(), buf.size()) == 0)
		{
			buf.back() = '\0'; // Ensure null-termination
			return std::string(buf.data());
		}
#endif
		return {};
	}

	std::string username()
	{
#if defined(_WIN32)
		std::array<char, UNLEN + 1> buf{};
		DWORD size = static_cast<DWORD>(buf.size());
		if (GetUserNameA(buf.data(), &size))
		{
			buf.back() = '\0';                        // Ensure null-termination
			return std::string(buf.data(), size - 1); // size includes '\0'
		}
#else
		// Thread-safe version
		//
		struct passwd pwd;
		struct passwd* result = nullptr;
		std::array<char, 16384> buf{}; // Common buffer size for getpwuid_r

		if (getpwuid_r(getuid(), &pwd, buf.data(), buf.size(), &result) == 0 && result)
		{
			return std::string(pwd.pw_name);
		}
#endif
		return {};
	}

	::Network::SoftwareInfo makeSoftwareInfo(const std::string& equipmentId)
	{
		::Network::SoftwareInfo softwareInfo;

		softwareInfo.set_softwaretype(AdsBridgeSoftwareType);
		softwareInfo.set_equipmentid(equipmentId);

		softwareInfo.set_majorversion(U7SET_MAJOR_VERSION);
		softwareInfo.set_minorversion(U7SET_MINOR_VERSION);
		softwareInfo.set_patchversion(U7SET_PATCH_VERSION);
		softwareInfo.set_releasetype(U7SET_RELEASE_TYPE);
		softwareInfo.set_branchname(U7SET_BRANCH_NAME);
		softwareInfo.set_commithash(U7SET_COMMIT_HASH);
		softwareInfo.set_builddate(U7SET_BUILD_DATE);
		softwareInfo.set_pipelineid(U7SET_PIPELINE_ID);

		softwareInfo.set_hostname(hostname());
		softwareInfo.set_osusername(username());

		return softwareInfo;
	}
} // namespace

namespace AdsBridge
{
	AdsBridgeFacade::AdsBridgeFacade(Resources& res, ILoggerStd& log) :
		m_res{res},
		m_log{log},
		m_signals{std::make_unique<AdsBridge::AdsbAppSignalManager>(res)},
		m_adsConnection{std::make_unique<ClientLib::AdsConnection>(*m_signals.get(), m_signals.get(), nullptr, log)}
	{
		log.writeMessage("AdsBridgeFacade::AdsBridgeFacade()");
	}

	AdsBridgeFacade::~AdsBridgeFacade()
	{
		m_log.writeMessage("AdsBridgeFacade::~AdsBridgeFacade()");
	}

	bool AdsBridgeFacade::setConfiguration(const std::vector<char>& data, const std::string& profile)
	{
		constexpr std::string_view elementConfiguration = "Configuration";
		constexpr std::string_view elementSoftware = "Software";
		constexpr std::string_view elementSettingsSet = "SettingsSet";
		constexpr std::string_view elementSettings = "Settings";
		constexpr std::string_view elementAppDataService = "AppDataService";


		constexpr std::string_view attributeCaption = "Caption";
		constexpr std::string_view attributeEquipmentId = "EquipmentID";
		constexpr std::string_view attributeType = "Type";
		constexpr std::string_view attributeProfile = "Profile";
		constexpr std::string_view attributeClientRequestIP = "ClientRequestIP";
		constexpr std::string_view attributeClientRequestPort = "ClientRequestPort";

		pugi::xml_document doc;
		if (auto result = doc.load_buffer(data.data(), data.size()); //
			result == false)
		{
			m_log.writeError(std::format("AdsBridgeFacade::setConfiguration(), Failed to parse XML data: {}", result.description()));
			return false;
		}

		bool result = true;
		int typeInt = 0;
		std::string equipmentId;
		std::string caption;
		std::vector<ServiceEndpoint> adsServices;

		// <Configuration>
		//
		{
			auto nodeConfiguration = doc.child(elementConfiguration);
			if (nodeConfiguration.empty() == true)
			{
				m_log.writeError("AdsBridgeFacade::setConfiguration(), Missing <Configuration> element.");
				return false;
			}

			// <Software>
			//
			{
				auto nodeSoftware = nodeConfiguration.child(elementSoftware);
				if (nodeSoftware.empty() == true)
				{
					m_log.writeError("AdsBridgeFacade::setConfiguration(), Missing <Software> element.");
					return false;
				}

				caption = nodeSoftware.attribute(attributeCaption.data()).as_string();
				if (caption.empty() == true)
				{
					m_log.writeError("AdsBridgeFacade::setConfiguration(), Caption attribute is missing or empty.");
					return false;
				}

				typeInt = nodeSoftware.attribute(attributeType.data()).as_int(-1);
				if (typeInt != AdsBridgeSoftwareType)
				{
					m_log.writeError("AdsBridgeFacade::setConfiguration(), Wrong software type, AdsBridge is expected.");
					return false;
				}

				equipmentId = nodeSoftware.attribute(attributeEquipmentId.data()).as_string();
				if (equipmentId.empty() == true)
				{
					m_log.writeError("AdsBridgeFacade::setConfiguration(), EquipmentID attribute is missing or empty.");
					return false;
				}

				// <SettingsSet>
				//
				{
					auto nodeSettingsSet = nodeConfiguration.child(elementSettingsSet);
					if (nodeSettingsSet.empty() == true)
					{
						m_log.writeError("AdsBridgeFacade::setConfiguration(), Missing <SettingsSet> element.");
						return false;
					}

					// <Settings>
					//
					{
						// Look for element <Settings> with attribute Profile == profile, inside <SettingsSet>.
						//
						pugi::xml_node nodeSettings;
						for (auto settingsNode : nodeSettingsSet.children(elementSettings.data()))
						{
							std::string profileAttr = settingsNode.attribute(attributeProfile.data()).as_string();
							if (profileAttr == profile)
							{
								nodeSettings = settingsNode;
								break;
							}
						}

						if (nodeSettings.empty() == true)
						{
							m_log.writeError(std::format("AdsBridgeFacade::setConfiguration(), Profile {} not found.", profile));
							return false;
						}

						// <AppDataService EquipmentID="SYSTEMID_CLIENTTEST_WS01_ADS_RC1" ClientRequestIP="127.0.0.1"
						// ClientRequestPort="13323" RtTrendsRequestIP="127.0.0.1" RtTrendsRequestPort="13324"/>
						//
						for (auto nodeAppDataService : nodeSettings.children(elementAppDataService.data()))
						{
							std::string adsEquipmentId = nodeAppDataService.attribute(attributeEquipmentId.data()).as_string();
							if (adsEquipmentId.empty() == true)
							{
								m_log.writeError(
									"AdsBridgeFacade::setConfiguration(), <AppDataService> EquipmentID attribute is missing or empty.");
								result = false;
								continue;
							}

							std::string clientRequestIP = nodeAppDataService.attribute(attributeClientRequestIP.data()).as_string();
							int clientRequestPort = nodeAppDataService.attribute(attributeClientRequestPort.data()).as_int(0);

							if (clientRequestIP.empty() == true || clientRequestPort == 0)
							{
								m_log.writeError("AdsBridgeFacade::setConfiguration(), <AppDataService> ClientRequestIP or "
												 "ClientRequestPort attribute is missing or invalid.");
								result = false;
								continue;
							}
							adsServices.push_back(
								ServiceEndpoint::create(adsEquipmentId, adsEquipmentId, clientRequestIP, clientRequestPort));

							m_log.writeMessage(std::format("AdsBridgeFacade::setConfiguration(), Found AppDataService: {}, {}:{}",
														   adsEquipmentId,
														   clientRequestIP,
														   clientRequestPort));
						} // </AppDataService>
					} // </Settings>
				} // </SettingsSet>
			} // </Software>
		} // </Configuration>

		// Set configuration.
		//
		setEquipmentId(equipmentId);
		m_appDataServices = adsServices;

		return result;
	}

	void AdsBridgeFacade::addAppDataService(const std::string& adsEquipmentId, const std::string& address, int port)
	{
		m_log.writeMessage(
			std::format("AdsBridgeFacade::addAppDataService(), Adding AppDataService: {}, {}:{}", adsEquipmentId, address, port));

		m_appDataServices.push_back(ServiceEndpoint::create(adsEquipmentId, adsEquipmentId, address, port));

		return;
	}

	void AdsBridgeFacade::clearAppDataServices()
	{
		m_log.writeMessage("AdsBridgeFacade::clearAppDataServices()");
		m_appDataServices.clear();
		return;
	}

	void AdsBridgeFacade::connect()
	{
		if (m_equipmentId.empty() == true)
		{
			m_log.writeError("AdsBridgeFacade::connect(), EquipmentID is empty.");
			return;
		}

		if (m_appDataServices.empty() == true)
		{
			m_log.writeWarning("AdsBridgeFacade::connect(), No AppDataService(s) to connect to.");
		}

		m_log.writeMessage(std::format("AdsBridgeFacade::connect(), Connecting {} AppDataService(s)...", m_appDataServices.size()));

		m_adsConnection->updateConnections(makeSoftwareInfo(m_equipmentId), m_appDataServices);

		return;
	}

	void AdsBridgeFacade::close()
	{
		m_log.writeMessage("AdsBridgeFacade::close()");

		m_adsConnection->updateConnections(makeSoftwareInfo(m_equipmentId), {});
		return;
	}

	const std::string& AdsBridgeFacade::equipmentId() const
	{
		return m_equipmentId;
	}

	void AdsBridgeFacade::setEquipmentId(const std::string& equipmentId)
	{
		m_equipmentId = equipmentId;
		m_log.writeMessage("AdsBridgeFacade::setEquipmentId(), EquipmentID set to: " + equipmentId);
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
			m_log.writeWarning("AdsBridgeFacade::signalCount(): Signal parameters not fully loaded when signalCount() is called.");
		}

		return m_signals->signalsCount();
	}

	bool AdsBridgeFacade::signalList(MatsSignalHash* out, size_t count)
	{
		static_assert(sizeof(MatsSignalHash) == sizeof(Hash));

		if (out == nullptr)
		{
			m_log.writeError("AdsBridgeFacade::signalList(), out is nullptr.");
			return false;
		}

		auto hashes = m_signals->signalHashes();
		if (hashes.size() != count)
		{
			m_log.writeError(std::format("AdsBridgeFacade::signalList(), count is invalid, expected {}.", hashes.size()));
			return false;
		}

		std::memset(out, 0, sizeof(MatsSignalHash) * count);
		std::memcpy(out, hashes.data(), count * sizeof(MatsSignalHash));
		return true;
	}

	size_t AdsBridgeFacade::signalParams(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalParam* out, size_t count)
	{
		static_assert(sizeof(MatsSignalHash) == sizeof(Hash));

		if (structSize != sizeof(MatsAppSignalParam))
		{
			m_log.writeError("AdsBridgeFacade::signalParams(), structSize is invalid.");
			return 0;
		}

		if (signalHashes == nullptr)
		{
			m_log.writeError("AdsBridgeFacade::signalParams(), signalHashes is nullptr.");
			return 0;
		}

		if (out == nullptr)
		{
			m_log.writeError("AdsBridgeFacade::signalParams(), out is nullptr.");
			return 0;
		}

		std::memset(out, 0, sizeof(MatsAppSignalParam) * count);

		int result = 0;

		for (const auto& h : std::span{signalHashes, count})
		{
			auto signalParam = m_signals->signalParam(h);

			MatsAppSignalParam& outSignal = *out;
			out++;

			if (signalParam.has_value() == false)
			{
				outSignal.hash = h;
				m_log.writeWarning(std::format("AdsBridgeFacade::signalParams(), Signal not found: {}", h));
			}
			else
			{
				outSignal = signalParam.value();
				result++;
			}
		}

		return result;
	}

	size_t AdsBridgeFacade::signalStates(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalState* out, size_t count)
	{
		static_assert(sizeof(MatsSignalHash) == sizeof(Hash));

		if (structSize != sizeof(MatsAppSignalState))
		{
			m_log.writeError("AdsBridgeFacade::signalStates(), structSize is invalid.");
			return 0;
		}

		if (signalHashes == nullptr)
		{
			m_log.writeError("AdsBridgeFacade::signalStates(), signalHashes is nullptr.");
			return 0;
		}

		if (out == nullptr)
		{
			m_log.writeError("AdsBridgeFacade::signalStates(), out is nullptr.");
			return 0;
		}

		std::memset(out, 0, sizeof(MatsAppSignalState) * count);

		int result = 0;

		// Get states.
		//
		std::vector<std::optional<::MatsAppSignalState>> states;
		m_signals->signalState(std::span{signalHashes, count}, &states);

		assert(states.size() == count);

		size_t index = 0;
		for (const auto& signalState : states)
		{
			result++;
			*out = signalState
					   .or_else(
						   [signalHashes, index, &result]
						   {
							   result--;
							   std::optional<::MatsAppSignalState> s = MatsAppSignalState{};
							   s->hash = signalHashes[index];
							   return s;
						   })
					   .value();
			out++;
			index++;
		}

		return result;
	}

	size_t AdsBridgeFacade::connectionCount() const
	{
		return ClientConnStatsStd::statistics().size();
	}

	bool AdsBridgeFacade::connectionStatus(size_t structSize, struct AdsConnectionStatus* out, size_t count) const
	{
		if (structSize != sizeof(AdsConnectionStatus))
		{
			m_log.writeError("AdsBridgeFacade::connectionStatus(), structSize is invalid.");
			return false;
		}

		if (out == nullptr)
		{
			m_log.writeError("AdsBridgeFacade::connectionStatus(), out is nullptr.");
			return false;
		}

		auto stats = ClientConnStatsStd::statistics();
		if (stats.size() != count)
		{
			m_log.writeError(std::format("AdsBridgeFacade::connectionStatus(), count is invalid, expected {}.", stats.size()));
			return false;
		}

		for (const auto& s : stats)
		{
			AdsConnectionStatus& state = *out;

			state.id = s.id;
			state.status = s.state.isConnected;
			state.setConnectionResult = static_cast<::MatsConnectionResult>(s.state.setConnectionResult);
			state.connectionType = m_res.getString(s.objectName);
			state.port = s.state.peerAddr.port;
			state.address = m_res.getString(s.state.peerAddr.address);
			state.adsEquipmentId = m_res.getString(s.state.connectedSoftwareInfo.equipmentid());
			state.received = s.state.receivedBytes;
			state.sent = s.state.sentBytes;
			state.requestCount = s.state.requestCount;
			state.replyCount = s.state.replyCount;

			++out;
		}

		return true;
	}
} // namespace AdsBridge