#include "CaptureDevice.h"
#include <WUtils.h>

void dumpPacketHandler(u_char* param, const struct pcap_pkthdr* header, const u_char* packetData);
void retranslatePacketHandler(u_char* param, const struct pcap_pkthdr* header, const u_char* packetData);
void printPacketInfo(const struct pcap_pkthdr* header, const u_char* packetData, CircularLoggerShared log);

BOOL WINAPI consoleCtrlHandler(_In_ DWORD dwCtrlType);

CaptureDevice::PCapAddr::PCapAddr(const pcap_addr* pa)
{
	TEST_PTR_RETURN(pa);

	if (pa->addr != NULL)
	{
		addr = *pa->addr;
	}
	else
	{
		std::memset(&addr, 0, sizeof(addr));
	}

	if (pa->netmask != NULL)
	{
		netmask = *pa->netmask;
	}
	else
	{
		std::memset(&netmask, 0, sizeof(netmask));
	}

	if (pa->broadaddr != NULL)
	{
		broadaddr = *pa->broadaddr;
	}
	else
	{
		std::memset(&broadaddr, 0, sizeof(broadaddr));
	}

	if (pa->dstaddr != NULL)
	{
		dstaddr = *pa->dstaddr;
	}
	else
	{
		std::memset(&dstaddr, 0, sizeof(dstaddr));
	}
}

CaptureDevice::CaptureDevice(const pcap_if_t* cd, CircularLoggerShared log)
{
	TEST_PTR_RETURN(cd);
	TEST_PTR_RETURN(log);

	m_name = QString(cd->name);

	if (cd->description != NULL)
	{
		m_description = QString(cd->description).trimmed();
	}
	else
	{
		m_description = "(No description available)";
	}

	m_flags = cd->flags;

	for(const pcap_addr* paddr = cd->addresses; paddr; paddr = paddr->next)
	{
		m_addresses.emplace_back(paddr);
	}

	m_log = log;
	m_str.reserve(128);
}

CaptureDevice::~CaptureDevice()
{
	close();

	DELETE_ARRAY_IF_NOT_NULL(m_rtrBuffer);
}

bool CaptureDevice::getCaptureDevices(std::vector<CaptureDevice>* capDevs, CircularLoggerShared log)
{
	TEST_PTR_RETURN_FALSE(capDevs);

	capDevs->clear();

	pcap_if_t* captureDevices = nullptr;

	char errbuf[PCAP_ERRBUF_SIZE];

	// Retrieve the capture devices list
	//
	if(pcap_findalldevs(&captureDevices, errbuf) == PCAP_ERROR)
	{
		DEBUG_LOG_ERR(log, QString("Error in pcap_findalldevs: %1").arg(errbuf));
		return false;
	}

	int devCount = 0;

	for(const pcap_if_t* capDev = captureDevices; capDev; capDev = capDev->next)
	{
		devCount++;
	}

	if (devCount == 0)
	{
		DEBUG_LOG_WRN(log, "No capture devices found!");
		return true;
	}

	DEBUG_LOG_MSG(log, QString("Found %1 capture device(s).").arg(devCount));

	capDevs->reserve(devCount);

	for(const pcap_if_t* capDev = captureDevices; capDev; capDev = capDev->next)
	{
		capDevs->emplace_back(capDev, log);
	}

	pcap_freealldevs(captureDevices);

	return true;
}

bool CaptureDevice::testCapturing()
{
	bool result = openForCapture();

	RETURN_IF_FALSE(result);

	result = setCaptureFilter("udp");

	RETURN_IF_FALSE(result);

	SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

	dumpPackets();

	close();

	SetConsoleCtrlHandler(consoleCtrlHandler, FALSE);

	return true;
}

bool CaptureDevice::openForCapture()
{
	char errbuf[PCAP_ERRBUF_SIZE * 2];

	// Open device to capture
	//
	m_capHandle = pcap_create(C_STR(m_name), errbuf);

	if (m_capHandle == NULL)
	{
		DEBUG_LOG_ERR(m_log, QString("Error creating capture handle for device '%1': %2").
								arg(m_description).arg(errbuf));
		return false;
	}

	int res = 0;

	res |= pcap_set_buffer_size(m_capHandle, 65536);
	res |= pcap_set_promisc(m_capHandle, 1);
	res |= pcap_set_immediate_mode(m_capHandle, 1);

	if (res != 0)
	{
		DEBUG_LOG_ERR(m_log, QString("Error set options of capture handle for device '%1'").arg(m_description));
		return false;
	}

	res = pcap_activate(m_capHandle);

	if (res != 0)
	{
		DEBUG_LOG_ERR(m_log, QString("Error activating capture handle for device '%1'").arg(m_description));
		return false;
	}

	// Check the link layer. We support only Ethernet for simplicity.
	//
	if(pcap_datalink(m_capHandle) != DLT_EN10MB)
	{
		DEBUG_LOG_ERR(m_log, QString("This program works only on Ethernet networks."));
		return false;
	}

	// Oldstyle m_capHandle initialization
	//
	// m_capHandle = pcap_open_live(C_STR(m_name),	// name of the device
	// 							65536,		// portion of the packet to capture.
	// 										// 65536 grants that the whole packet will be captured on all the MACs.
	// 							1,			// promiscuous mode (nonzero means promiscuous)
	// 							1000,		// read timeout
	// 							errbuf);		// error buffer
	// if (m_capHandle == NULL)
	// {
	// 	DEBUG_LOG_ERR(m_log, QString("Unable to open device '%1': %2").arg(m_description).arg(errbuf));
	// 	return false;
	// }

	return true;
}

bool CaptureDevice::setCaptureFilter(const RetranslateCfg& rtrCfg)
{
	QString capFilter;

	for(const RetranslateEntry& re : rtrCfg.rtrEntries)
	{
		if (capFilter.isEmpty() == false)
		{
			capFilter += QStringLiteral(" or ");
		}

		capFilter += QString("(src host %1").arg(re.srcAddr.addressStr());

		if (re.srcAddr.port() != 0)
		{
			capFilter += QString(" and src port %1").arg(re.srcAddr.port());
		}

		capFilter += QString(" and dst host %1").arg(re.destAddr.addressStr());

		if (re.destAddr.port() != 0)
		{
			capFilter += QString(" and dst port %1").arg(re.destAddr.port());
		}

		capFilter += QStringLiteral(")");
	}

	capFilter = "udp and (" + capFilter + ")";

	return setCaptureFilter(capFilter);
}

bool CaptureDevice::setCaptureFilter(const QString& capFilter)
{
	bpf_program capFilterCode;

	// compile the filter
	//
	if (pcap_compile(m_capHandle, &capFilterCode, C_STR(capFilter), 1, PCAP_NETMASK_UNKNOWN) < 0 )
	{
		DEBUG_LOG_ERR(m_log, QString("Unable to compile the capturing filter '%1'. Check filter syntax.").arg(capFilter));
		return false;
	}

	// set the filter
	//
	if (pcap_setfilter(m_capHandle, &capFilterCode) < 0)
	{
		DEBUG_LOG_ERR(m_log, QString("Error setting the capture filter '%1'.").arg(capFilter));
		return false;
	}

	DEBUG_LOG_MSG(m_log, QString("Capture filter '%1' applied").arg(capFilter));

	return true;
}

void CaptureDevice::dumpPackets()
{
	DEBUG_LOG_MSG(m_log, QString("Listening on '%1'...").arg(m_description));
	std::cout << "\n";

	Q_ASSERT(m_capHandle != NULL);

	addCaptureHandle(m_capHandle);

	pcap_loop(m_capHandle, 0, dumpPacketHandler, reinterpret_cast<u_char*>(this));

	removeCaptureHandle(m_capHandle);
}

void CaptureDevice::printPacketInfo(const struct pcap_pkthdr* header, const u_char* packetData)
{
	TEST_PTR_RETURN(header);
	TEST_PTR_RETURN(packetData);
	TEST_PTR_RETURN(m_log);

	QDateTime dt = QDateTime::fromSecsSinceEpoch(header->ts.tv_sec);
	QTime tm = dt.time();

	static const QChar z('0');

	QString str = QString("%1:%2:%3.%4  ").
				  arg(tm.hour(), 2, 10, z).
				  arg(tm.minute(), 2, 10, z).
				  arg(tm.second(), 2, 10, z).
				  arg(header->ts.tv_usec, 6, 10, z);

	EthernetHeader* eh = nullptr;
	IpHeader* ih = nullptr;
	UdpHeader* uh = nullptr;

	if (getPacketHeaders(packetData, header->len, &eh, &ih, &uh) == false)
	{
		Q_ASSERT(false);
		return;
	}

	u_short srcPort = ntohs(uh->srcPort);
	u_short destPort = ntohs(uh->destPort);

	str += QString("%1.%2.%3.%4:%5").
		   arg(ih->srcIP.byte1).
		   arg(ih->srcIP.byte2).
		   arg(ih->srcIP.byte3).
		   arg(ih->srcIP.byte4).
		   arg(srcPort).leftJustified(IP_PORT_LEN, SPACE);

	str += QStringLiteral("  ->  ");

	str += QString("%1.%2.%3.%4:%5").
		   arg(ih->destIP.byte1).
		   arg(ih->destIP.byte2).
		   arg(ih->destIP.byte3).
		   arg(ih->destIP.byte4).
		   arg(destPort).leftJustified(IP_PORT_LEN, SPACE);

	str += QString("  len = %1").arg(header->len);

	DEBUG_LOG_MSG(m_log, str);
}

void CaptureDevice::close()
{
	if (m_capHandle != NULL)
	{
		pcap_close(m_capHandle);
		m_capHandle = NULL;

		DEBUG_LOG_ERR(m_log, QString("Capture closed on '%1'...").arg(m_description));
	}
}

bool CaptureDevice::retranslate(const RetranslateCfg& rtrCfg, int threadNo, bool isService)
{
	m_rtrCfg = rtrCfg;
	m_isService = isService;

	Q_ASSERT(rtrCfg.captureDeviceDescription == m_description);

	DEBUG_LOG_MSG(m_log, QString("Retranslating thread #%1 started (mode - %2)").
						 arg(threadNo).arg(m_isService ? "Service" : "Console"));

	m_rtrEnriesWithPorts.clear();
	m_rtrEnriesWithoutPorts.clear();

	for(const RetranslateEntry& rtrEntry : rtrCfg.rtrEntries)
	{
		if (rtrEntry.srcAddr.port() != 0)
		{
			m_rtrEnriesWithPorts.emplace(rtrEntry.srcAddr, rtrEntry);
		}
		else
		{
			m_rtrEnriesWithoutPorts.emplace(rtrEntry.srcAddr.address32(), rtrEntry);
		}
	}

	bool result = openForCapture();

	RETURN_IF_FALSE(result);

	result = setCaptureFilter(rtrCfg);

	RETURN_IF_FALSE(result);

	if (isService == false)
	{
		SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
	}

	DEBUG_LOG_MSG(m_log, QString("Listening on '%1'...").arg(m_description));
	std::cout << "\n";

	Q_ASSERT(m_capHandle != NULL);

	addCaptureHandle(m_capHandle);

	pcap_loop(m_capHandle, 0, retranslatePacketHandler, reinterpret_cast<u_char*>(this));

	removeCaptureHandle(m_capHandle);

	close();

	DEBUG_LOG_MSG(m_log, QString("Retranslating thread #%1 finished").arg(threadNo));

	return true;
}

void CaptureDevice::retranslatePacket(const pcap_pkthdr* header, const u_char* packetData)
{
	TEST_PTR_RETURN(m_capHandle);

	EthernetHeader* eh = nullptr;
	IpHeader* ih = nullptr;
	UdpHeader* uh = nullptr;

	if (getPacketHeaders(packetData, header->len, &eh, &ih, &uh) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TEST_PTR_RETURN(eh);
	TEST_PTR_RETURN(ih);
	TEST_PTR_RETURN(uh);

	HostAddressPort srcAddressPort(ntohl(ih->srcIP.ip), ntohs(uh->srcPort));
	HostAddressPort destAddressPort(ntohl(ih->destIP.ip), ntohs(uh->destPort));

	RetranslateEntry* rtrEntry = nullptr;

	auto it = m_rtrEnriesWithPorts.find(srcAddressPort);

	if (it != m_rtrEnriesWithPorts.end())
	{
		rtrEntry = &it->second;
	}
	else
	{
		auto itwp = m_rtrEnriesWithoutPorts.find(srcAddressPort.address32());

		if (itwp == m_rtrEnriesWithoutPorts.end())
		{
			return;
		}

		rtrEntry = &itwp->second;
	}

	TEST_PTR_RETURN(rtrEntry);

	if (rtrEntry->destAddr.port() != 0)
	{
		if (rtrEntry->destAddr != destAddressPort)
		{
			return;
		}
	}
	else
	{
		if (rtrEntry->destAddr.address32() != destAddressPort.address32())
		{
			return;
		}
	}

	if (m_rtrBufferSize < header->len)
	{
		DELETE_ARRAY_IF_NOT_NULL(m_rtrBuffer);

		m_rtrBufferSize = header->len;
		m_rtrBuffer = new quint8 [m_rtrBufferSize];
	}

	std::memcpy(m_rtrBuffer, packetData, header->len);

	EthernetHeader* rtrEh = nullptr;
	IpHeader* rtrIh = nullptr;
	UdpHeader* rtrUh = nullptr;

	if (getPacketHeaders(m_rtrBuffer, header->len, &rtrEh, &rtrIh, &rtrUh) == false)
	{
		Q_ASSERT(false);
		return;
	}

	TEST_PTR_RETURN(rtrEh);
	TEST_PTR_RETURN(rtrIh);
	TEST_PTR_RETURN(rtrUh);

	bool recalcIpHeaderChecksum = false;
	bool recalcUdpHeaderChecksum = false;

	// source IP address replacement
	//
	quint32 rtrIP = htonl(rtrEntry->rtrSrcAddr.address32());

	if (rtrIh->srcIP.ip != rtrIP)
	{
		rtrIh->srcIP.ip = rtrIP;
		recalcIpHeaderChecksum = true;
	}

	// source UDP Port replacement
	//
	quint16 rtrPort = htons(rtrEntry->rtrSrcAddr.port());

	if (rtrPort != 0 && rtrUh->srcPort != rtrPort)
	{
		rtrUh->srcPort = rtrPort;
		recalcUdpHeaderChecksum = true;
	}

	// destination IP address replacement
	//
	rtrIP = htonl(rtrEntry->rtrDestAddr.address32());

	if (rtrIh->destIP.ip != rtrIP)
	{
		rtrIh->destIP.ip = rtrIP;
		recalcIpHeaderChecksum = true;
	}

	// destination UDP Port replacement
	//
	rtrPort = htons(rtrEntry->rtrDestAddr.port());

	if (rtrPort != 0 && rtrUh->destPort != rtrPort)
	{
		rtrUh->destPort = rtrPort;
		recalcUdpHeaderChecksum = true;
	}

	if (recalcIpHeaderChecksum == true)
	{
		calcIpHeaderChecksum(rtrIh);
	}

	if (recalcUdpHeaderChecksum == true)
	{
		calcUdpHeaderChecksum(rtrIh);
	}

	quint32 bytesWritten = pcap_inject(m_capHandle, m_rtrBuffer, header->len);

	if (bytesWritten == header->len)
	{
		rtrEntry->retranslatedCount++;

		if (m_isService == true)
		{
			if ((rtrEntry->retranslatedCount % (200 * 60)) == 0)			// 1 record per minute for LM packets retranslation
			{
				DEBUG_LOG_MSG(m_log, QString("Retranslated from %1 to %2 packets: %3").
									 arg(srcAddressPort.addressPortStrWithoutPort0()).
									 arg(rtrEntry->rtrDestAddr.addressPortStrWithoutPort0()).
									 arg(rtrEntry->retranslatedCount));
			}
		}
		else
		{
			m_str.clear();

			m_str += srcAddressPort.addressPortStrWithoutPort0().leftJustified(IP_PORT_LEN, SPACE);
			m_str += SRC_DEST_SEPARATOR;
			m_str += destAddressPort.addressPortStrWithoutPort0().leftJustified(IP_PORT_LEN, SPACE);
			m_str += RTR_SEPARATOR;
			m_str += rtrEntry->rtrSrcAddr.addressPortStrWithoutPort0().leftJustified(IP_PORT_LEN, SPACE);
			m_str += SRC_DEST_SEPARATOR;
			m_str += rtrEntry->rtrDestAddr.addressPortStrWithoutPort0().leftJustified(IP_PORT_LEN, SPACE);

			DEBUG_LOG_MSG(m_log, m_str);
		}
	}
	else
	{
		Q_ASSERT(false);
	}
}

void CaptureDevice::breakAllCaptures()
{
	std::lock_guard lg(m_capHandlesMutex);

	for(pcap_t* capHandle : m_capHandles)
	{
		if (capHandle != nullptr)
		{
			pcap_breakloop(capHandle);
		}
	}
}

QString CaptureDevice::name() const
{
	return m_name;
}

QString CaptureDevice::description() const
{
	return m_description;
}

CircularLoggerShared CaptureDevice::log()
{
	return m_log;
}

const RetranslateCfg& CaptureDevice::retranslateCfg() const
{
	return m_rtrCfg;
}

void CaptureDevice::addCaptureHandle(pcap_t* capHandle)
{
	std::lock_guard lg(m_capHandlesMutex);

	Q_ASSERT(m_capHandles.contains(capHandle) == false);

	m_capHandles.insert(capHandle);
}

void CaptureDevice::removeCaptureHandle(pcap_t* capHandle)
{
	std::lock_guard lg(m_capHandlesMutex);

	Q_ASSERT(m_capHandles.contains(capHandle) == true);

	m_capHandles.erase(capHandle);
}

bool CaptureDevice::getPacketHeaders(const u_char* packetData,
									 quint32 packetLen,
									 EthernetHeader** ethHeader,
									 IpHeader** ipHeader,
									 UdpHeader** udpHeader)
{
	TEST_PTR_RETURN_FALSE(packetData);
	TEST_PTR_RETURN_FALSE(ethHeader);
	TEST_PTR_RETURN_FALSE(ipHeader);
	TEST_PTR_RETURN_FALSE(udpHeader);

	u_char* packetDataPtr = const_cast<u_char*>(packetData);

	*ethHeader = reinterpret_cast<EthernetHeader*>(packetDataPtr);

	// retireve ip header pointer
	//
	*ipHeader = reinterpret_cast<IpHeader*>(packetDataPtr + sizeof(EthernetHeader));

	// retireve udp header pointer
	//
	*udpHeader = reinterpret_cast<UdpHeader*>(packetDataPtr + sizeof(EthernetHeader) + (*ipHeader)->headerLenBytes());

	if ((reinterpret_cast<const u_char*>(*udpHeader) - packetDataPtr) <= (packetLen - sizeof(UdpHeader)))
	{
		return true;
	}

	*ethHeader = nullptr;
	*ipHeader = nullptr;
	*udpHeader = nullptr;

	return false;
}

void dumpPacketHandler(u_char* param, const struct pcap_pkthdr* header, const u_char* packetData)
{
	TEST_PTR_RETURN(param);
	TEST_PTR_RETURN(header);
	TEST_PTR_RETURN(packetData);

	CaptureDevice* capDev = reinterpret_cast<CaptureDevice*>(param);

	TEST_PTR_RETURN(capDev);

	capDev->printPacketInfo(header, packetData);
}

void retranslatePacketHandler(u_char* param, const struct pcap_pkthdr* header, const u_char* packetData)
{
	TEST_PTR_RETURN(param);
	TEST_PTR_RETURN(header);
	TEST_PTR_RETURN(packetData);

	CaptureDevice* capDev = reinterpret_cast<CaptureDevice*>(param);

	capDev->retranslatePacket(header, packetData);
}
