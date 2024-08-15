#pragma once

#include <pcap.h>
#include <HostAddressPort.h>

#include "CircularLogger.h"
#include "IpHeaders.h"

struct RetranslateEntry
{
	HostAddressPort srcAddr;
	HostAddressPort destAddr;
	HostAddressPort sendToAddr;

	quint64 retranslatedCount = 0;
};

struct RetranslateCfg
{
	QString captureDeviceDescription;

	std::vector<RetranslateEntry> rtrEntries;
};

class CaptureDevice
{
private:
	struct PCapAddr
	{
		PCapAddr(const pcap_addr* pa);

		// fields copied from pcap_addr structure
		//
		sockaddr addr;				// address
		sockaddr netmask;			// netmask for that address
		sockaddr broadaddr;			// broadcast address for that address
		sockaddr dstaddr;

		//
	};

public:
	CaptureDevice(const pcap_if_t* cd, 	CircularLoggerShared log);
	~CaptureDevice();

	static bool getCaptureDevices(std::vector<CaptureDevice>* capDevs, CircularLoggerShared log);

	bool testCapturing();
	bool openForCapture();
	bool setCaptureFilter(const RetranslateCfg& rtrCfg);
	bool setCaptureFilter(const QString& capFilter);
	void dumpPackets();
	void printPacketInfo(const struct pcap_pkthdr* header, const u_char* packetData);

	void close();

	bool retranslate(const RetranslateCfg& rtrCfg, int threadNo, bool isService);
	void retranslatePacket(const struct pcap_pkthdr* header, const u_char* packetData);

	static void breakAllCaptures();

	QString name() const;
	QString description() const;
	CircularLoggerShared log();
	const RetranslateCfg& retranslateCfg() const;

private:
	static void addCaptureHandle(pcap_t* capHandle);
	static void removeCaptureHandle(pcap_t* capHandle);

	bool getPacketHeaders(const u_char* packetData, quint32 packetLen,
						  EthernetHeader **ethHeader,
						  IpHeader** ipHeader,
						  UdpHeader** udpHeader);

private:
	inline static std::mutex m_capHandlesMutex;
	inline static std::set<pcap_t*> m_capHandles;

	// fields copied from pcap_if_t structure
	//
	QString m_name;						// name to hand to "pcap_open_live()
	QString m_description;				// textual description of interface, or NULL
	bpf_u_int32 m_flags = 0;			// PCAP_IF_ interface flags
	std::vector<PCapAddr> m_addresses;

	//

	CircularLoggerShared m_log;

	pcap_t* m_capHandle = nullptr;

	RetranslateCfg m_rtrCfg;

	bool m_isService = false;

	std::map<HostAddressPort, RetranslateEntry> m_rtrEnriesWithPorts;
	std::map<quint32, RetranslateEntry> m_rtrEnriesWithoutPorts;

	quint8* m_rtrBuffer = nullptr;
	quint32 m_rtrBufferSize = 0;
};
