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
	bool result = openForCapturing();

	RETURN_IF_FALSE(result);

	SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

	dumpPackets();

	close();

	SetConsoleCtrlHandler(consoleCtrlHandler, FALSE);

	return true;
}

bool CaptureDevice::openForCapturing()
{
	char errbuf[PCAP_ERRBUF_SIZE];

	// Open device to capture
	//

	m_capHandle= pcap_open_live(C_STR(m_name),	// name of the device
								   65536,		// portion of the packet to capture.
												// 65536 grants that the whole packet will be captured on all the MACs.
								   1,			// promiscuous mode (nonzero means promiscuous)
								   1000,		// read timeout
								 errbuf);		// error buffer
	if (m_capHandle == NULL)
	{
		DEBUG_LOG_ERR(m_log, QString("Unable to open device '%1': %2").arg(m_description).arg(errbuf));
		return false;
	}

	// Check the link layer. We support only Ethernet for simplicity.
	//
	if(pcap_datalink(m_capHandle) != DLT_EN10MB)
	{
		DEBUG_LOG_ERR(m_log, QString("This program works only on Ethernet networks."));
		return false;
	}

	/*
	if (d->addresses != NULL)
		// Retrieve the mask of the first address of the interface
		netmask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
	{
		// If the interface is without addresses we suppose to be in a C class network
		//netmask=0xffffff;
		// netmask = 0xc0a80e00;		// 192.168.14.*
		netmask = 0xc0a80e00;			// 192.168.75.*
	} */

	u_int netmask = 0xFFFFFF00;		// ??????
	char packet_filter[] = "udp";		// "ip and udp";

	// udp and ((src 192.168.11.96 and src port 209  and dst 192.168.14.85) or (src 192.168.11.96 or 192.168.14.85))

	bpf_program fcode;

	// compile the filter
	//
	if (pcap_compile(m_capHandle, &fcode, packet_filter, 1, netmask) < 0 )
	{
		DEBUG_LOG_ERR(m_log, QString("Unable to compile the capturing filter. Check filter syntax."));
		return false;
	}

	// set the filter
	//
	if (pcap_setfilter(m_capHandle, &fcode) < 0)
	{
		DEBUG_LOG_ERR(m_log, QString("Error setting the capture filter."));
		return false;
	}

	return true;
}

void CaptureDevice::dumpPackets()
{
	DEBUG_LOG_ERR(m_log, QString("Listening on '%1'...").arg(m_description));
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

	static const QChar sp(' ');

	str += QString("%1.%2.%3.%4:%5").
		   arg(ih->srcIP.byte1).
		   arg(ih->srcIP.byte2).
		   arg(ih->srcIP.byte3).
		   arg(ih->srcIP.byte4).
		   arg(srcPort).leftJustified(21, sp);

	str += QStringLiteral("  ->  ");

	str += QString("%1.%2.%3.%4:%5").
		   arg(ih->destIP.byte1).
		   arg(ih->destIP.byte2).
		   arg(ih->destIP.byte3).
		   arg(ih->destIP.byte4).
		   arg(destPort).leftJustified(21, sp);

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

	m_rtrEnries.clear();
	m_rtrCounters.clear();

	for(const RetranslateEntry& rtrEntry : rtrCfg.rtrEntries)
	{
		m_rtrEnries.emplace(rtrEntry.srcAddr, rtrEntry);
		m_rtrCounters.emplace(rtrEntry.srcAddr, 0);
	}

	bool result = openForCapturing();

	RETURN_IF_FALSE(result);

	if (isService == false)
	{
		SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
	}

	DEBUG_LOG_ERR(m_log, QString("Listening on '%1'...").arg(m_description));
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

	quint32 srcIP = ntohl(ih->srcIP.ip);
	quint16 srcPort = ntohs(uh->srcPort);

	HostAddressPort srcAddressPort(srcIP, srcPort);

	auto it = m_rtrEnries.find(srcAddressPort);

	if (it == m_rtrEnries.end())
	{
		return;
	}

	const RetranslateEntry& rtrEntry = it->second;

	quint32 destIP = ntohl(ih->destIP.ip);
	quint16 destPort = ntohs(uh->destPort);

	if (rtrEntry.destAddr != HostAddressPort(destIP, destPort))
	{
		return;
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

	// destination IP address replacement
	//
	rtrIh->destIP.ip = htonl(rtrEntry.sendToAddr.address32());
	calcIpHeaderChecksum(rtrIh);

	// destination UDP Port replacement
	//
	rtrUh->destPort = htons(rtrEntry.sendToAddr.port());
	calcUdpHeaderChecksum(rtrIh);

	quint32 bytesWritten = pcap_inject(m_capHandle, m_rtrBuffer, header->len);

	if (bytesWritten == header->len)
	{
		auto it2 = m_rtrCounters.find(srcAddressPort);

		if (it2 != m_rtrCounters.end())
		{
			it2->second++;

			if ((it2->second % 1000) == 0)
			{
				DEBUG_LOG_MSG(m_log, QString("Retranslated from %1 to %2 packets: %3").
										arg(srcAddressPort.addressPortStr()).
										arg(rtrEntry.sendToAddr.addressPortStr()).
										arg(it2->second));
			}
		}
		else
		{
			Q_ASSERT(false);
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
