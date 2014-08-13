#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QObject>
#include "../include/configdata.h"

class Log;

const int ProtocolMaxVersion = 1;					// The maximum protocol version, changing it, change CONF_HEADER typedef

const uint16_t ConfigurationUartMask = 0x000F;		// The least 4bit of CONF_HEADER::moduleUartId
const uint16_t ConfigurationUartValue = 0x0003;		// The least 4bit of CONF_HEADER::moduleUartId

const uint16_t IdentificationFrameIndex = 0x0000;	// Frame index for security data
const uint16_t ConfiguartionFrameIndex = 0x0001;	// Frame index for configuration data (CONF_SERVICE_DATA)


enum ConfigureCommand
{
	Read = 0x0001,
	Write = 0x0002,
	Nop = 0x004
};

enum HeaderFlag
{
	OpDeniedCalibrationIsActive = 0x0001,
	OpDeniedInvalidModuleUartId = 0x0002,
	OpDeniedInvalidOpcode = 0x0004,
	OpDeniedInvalidFrameIndex = 0x0008,
	OpDeniedInvalidCrc = 0x0010,
	OpDeniedEepromHwError = 0x0020,
	OpDeniedInvalidEepromCrc = 0x0040
};

//
//	CONF_HEADER
//
#pragma pack(push, 1)
struct CONF_HEADER_V1
{
	uint16_t version;				// Protocol version
	uint16_t moduleUartId;			// Radiy’s ID of UART Interface ID Code
	uint16_t opcode;				// Command, set to ConfigureCommand::Nop
	uint16_t flags;					// State flags
	uint16_t frameIndex;			// Frame index
	uint16_t frameSize;				// FrameSize (BlockSize - CRC, usually 1016)
	uint16_t blockSize;				// BlockSize (EEPROM block size, usually 1024)
	uint32_t romSize;				// Total EEPROM size (in bytes)
	uint64_t crc64;					// Header CRC 
									// !!! ATTENTION !!!
									// HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN

	void dump(Log& log);
	void dumpFlagsState(Log& log);
	
	void setCrc();
	bool checkCrc();

	bool flagStateSuccess();		// Check the flags field, return true if operation is success (flags == 0)
};
#pragma pack(pop)

typedef CONF_HEADER_V1 CONF_HEADER;	// Current version (ProtocolMaxVersion) Header Struct

//
// CONF_SERVICE_DATA,		ATTENTION STRUCT IN BIG-ENDIAN
//							STRUCT VERSION DEFINED TROUGH CONF_HEADER_V1::version
//
#pragma pack(push, 1)
struct CONF_SERVICE_DATA_V1
{
	uint16_t m_moduleId;				// Radiy’s ID of UART Interface ID Code
	uint16_t m_diagVersion;					// Diagnostics version (NOT THIS STRUCT VERSION, STRUCT VERSION DEFINED TROUGH CONF_HEADER_V1::version)
	uint32_t m_factoryNo;				// Factory No
	uint16_t m_manufactureYear;			// Manufacturing date
	uint16_t m_manufactureMonth;		// 
	uint16_t m_manufactureDay;			// 
	uint16_t m_configureYear;			// Configuration date
	uint16_t m_configureMonth;			//
	uint16_t m_configureDay;			//
	uint16_t m_reserve1;				// Reserve
	uint16_t m_firmwareVersion1;		// Firmware version 1
	uint16_t m_firmwareVersion2;		// Firmware version 2
	uint32_t m_firmwareCrc1;			// Firmware Crc1
	uint32_t m_firmwareCrc2;			// Firmware Crc2
	uint16_t m_reserve3_0;				// Reserve 3 -- size 16bit words
	uint16_t m_reserve3_1;				//
	uint16_t m_reserve3_2;				//
	uint16_t m_reserve3_3;				//
	uint16_t m_reserve3_4;				//
	uint16_t m_reserve3_5;				//

	// set/get members
	//
	uint16_t diagVersion() const;
	void setDiagVersion(uint16_t value);

	uint32_t factoryNo() const;
	void setFactoryNo(uint32_t value);

	uint16_t manufactureYear() const;
	void setManufactureYear(uint16_t value);

	uint16_t manufactureMonth() const;
	void setManufactureMonth(uint16_t value);

	uint16_t manufactureDay() const;
	void setManufactureDay(uint16_t value);

	uint16_t configureYear() const;
	void setConfigureYear(uint16_t value);

	uint16_t configureMonth() const;
	void setConfigureMonth(uint16_t value);

	uint16_t configureDay() const;
	void setConfigureDay(uint16_t value);

	uint32_t firmwareCrc1() const;
	void setFirmwareCrc1(uint32_t value);

	uint32_t firmwareCrc2() const;
	void setFirmwareCrc2(uint32_t value);

	// --
	//
};
#pragma pack(pop)

typedef CONF_SERVICE_DATA_V1 CONF_SERVICE_DATA;		// Current version


//
// CONF_IDENTIFICATION_DATA,		ATTENTION STRUCT IN LITTLE-ENDIAN
//									
//
const uint16_t IdentificationStructMarker = 0xAC17;

#pragma pack(push, 1)
struct CONF_IDENTIFICATION_DATA_V1
{
	uint16_t marker;									// Struct marker, must be 0xAC17
	uint16_t version;									// Struct version
	UUID moduleUuid;									// Uniquie module identifier
	uint32_t count;										// Configurations counter

	struct CONF_IDENTIFICATION_RECORD
	{
		UUID configurationId;							// Uniquie configuration identifier
		uint64_t date;									// Configuration date and time
		uint16_t configuratorFactoryNo;					// Configurator factory no
		char host[24];									// Host name of the first configuration
		uint32_t reserve;								// Reserve
	};

	CONF_IDENTIFICATION_RECORD firstConfiguration;		// The first configuration Id record
	CONF_IDENTIFICATION_RECORD lastConfiguration;		// The last configuration Id record

	void dump(Log& log);
};
#pragma pack(pop)

typedef CONF_IDENTIFICATION_DATA_V1 CONF_IDENTIFICATION_DATA;	// Current version


//
//	Configurator
//
class Configurator : public QObject
{
	Q_OBJECT

public:
	Configurator(QString device, QObject* parent = nullptr);
	~Configurator();
	
protected:
	HANDLE openConnection();
	bool closeConnection(HANDLE hDevice);

	bool send(HANDLE hDevice, int moduleUartId, ConfigureCommand opcode, uint16_t frameIndex, uint16_t blockSize, const std::vector<uint8_t>& requestData, CONF_HEADER* pReceivedHeader, std::vector<uint8_t>* replyData);

	// Slots
	//
public slots:
	void setSettings(QString device, bool showDebugInfo);
	void readConfiguration(int param);
	void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc1, quint32 firmwareCrc2);
	void writeConfData(ConfigDataReader conf);
	void eraseFlashMemory(int param);

	// Signals
	//
signals:
	void communicationStarted();
	void communicationFinished();
	void communicationReadFinished(int protocolVersion, std::vector<uint8_t> data);

	// Properties
	//
protected:
	QString device() const;
	void setDevice(const QString& device);

	bool showDebugInfo() const;
	void setShowDebugInfo(bool showDebugInfo);

	// Data
	//
private:
	QString m_Device;
	bool m_showDebugInfo;
	uint32_t m_configuratorfactoryNo;

	mutable QMutex mutex;			// m_Device
};

#endif // CONFIGURATOR_H
