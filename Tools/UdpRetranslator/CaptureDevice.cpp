#include "CaptureDevice.h"
#include <WUtils.h>

pcap_t* currCapHandle = nullptr;

void dumpPacketHandler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data);
void printPacketInfo(const struct pcap_pkthdr* header, const u_char* pkt_data);

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);

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
		m_description = QString(cd->description);
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

	Q_ASSERT(devCount > 0);

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

	currCapHandle = m_capHandle;

	BOOL res = SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	capture();

	currCapHandle = NULL;

	close();

	SetConsoleCtrlHandler(HandlerRoutine, FALSE);

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

void CaptureDevice::capture()
{
	DEBUG_LOG_ERR(m_log, QString("Listening on '%1'...").arg(m_description));
	std::cout << "\n";

	pcap_loop(m_capHandle, 0, dumpPacketHandler, NULL);
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

QString CaptureDevice::name() const
{
	return m_name;
}

QString CaptureDevice::description() const
{
	return m_description;
}

void dumpPacketHandler(u_char* param, const struct pcap_pkthdr *header, const u_char* pkt_data)
{
	printPacketInfo(header, pkt_data);
}

void printPacketInfo(const struct pcap_pkthdr* header, const u_char* pkt_data)
{
	struct tm* ltime;
	char timestr[16];
	ip_header* ih;
	udp_header* uh;
	u_int ip_len;
	u_short sport, dport;
	time_t local_tv_sec;

	/* convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

	/* print timestamp and length of the packet */
	printf("%s.%.6d: ", timestr, header->ts.tv_usec);

	/* retireve the position of the ip header */
	ih = (ip_header*)(pkt_data +
					   14); //length of ethernet header

	/* retireve the position of the udp header */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header*)((u_char*)ih + ip_len);

	/* convert from network byte order to host byte order */
	sport = ntohs(uh->sport);
	dport = ntohs(uh->dport);

	/* print ip addresses and udp ports */
	printf("%d.%d.%d.%d.%d -> %d.%d.%d.%d.%d ",
		   ih->saddr.byte1,
		   ih->saddr.byte2,
		   ih->saddr.byte3,
		   ih->saddr.byte4,
		   sport,
		   ih->daddr.byte1,
		   ih->daddr.byte2,
		   ih->daddr.byte3,
		   ih->daddr.byte4,
		   dport);

	printf("len - %d\n", header->len);
}

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		if (currCapHandle != NULL)
		{
			std::cout << "\nCtrl+C pressed by user\n\n";
			pcap_breakloop(currCapHandle);
		}
		return TRUE;
	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}

