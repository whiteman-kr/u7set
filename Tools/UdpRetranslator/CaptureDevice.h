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
};

struct CaptureCfg
{
	QString captureDeviceDescription;

	std::vector<RetranslateEntry> rtrEntry;
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
	bool openForCapturing();
	void capture();
	void close();

	QString name() const;
	QString description() const;

private:

	// fields copied from pcap_if_t structure
	//
	QString m_name;						// name to hand to "pcap_open_live()
	QString m_description;				// textual description of interface, or NULL
	bpf_u_int32 m_flags = 0;			// PCAP_IF_ interface flags
	std::vector<PCapAddr> m_addresses;

	//

	CircularLoggerShared m_log;

	pcap_t* m_capHandle = nullptr;
};

extern pcap_t* currCapHandle;

