#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QObject>
#include <QSerialPort>
#include "../lib/ModuleConfiguration.h"
#include "../lib/OutputLog.h"

class OutputLog;

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

#pragma pack(push, 1)
struct Uuid
{
	uint32_t  data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];

	QUuid toQUuid() const;

    Uuid& operator = (const QUuid& uuid)
    {
        data1 = uuid.data1;
        data2 = uuid.data2;
        data3 = uuid.data3;
        for (int i = 0; i < 8; i++)
            data4[i] = uuid.data4[i];
        return *this;
    }
};
#pragma pack(pop)

//
//	CONF_HEADER
//
#pragma pack(push, 1)
struct CONF_HEADER_V1
{
	uint16_t version;				// Protocol version
    uint16_t moduleUartId;			// Radiys ID of UART Interface ID Code
	uint16_t opcode;				// Command, set to ConfigureCommand::Nop
	uint16_t flags;					// State flags
	uint16_t frameIndex;			// Frame index
	uint16_t frameSize;				// FrameSize (BlockSize - CRC, usually 1016)
	uint16_t blockSize;				// BlockSize (EEPROM block size, usually 1024)
	uint32_t romSize;				// Total EEPROM size (in bytes)
	uint64_t crc64;					// Header CRC 
									// !!! ATTENTION !!!
									// HEADER CRC IS CALCULATED BY NORMAL POLY, BUT STORED IN BIG-ENDIAN

        void dump(OutputLog *log);
        void dumpFlagsState(OutputLog *log);
	
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
    uint16_t m_reserve0_0;				// Radiys ID of UART Interface ID Code
    uint16_t m_reserve0_1;					// Diagnostics version (NOT THIS STRUCT VERSION, STRUCT VERSION DEFINED TROUGH CONF_HEADER_V1::version)
	uint32_t m_factoryNo;				// Factory No
	uint16_t m_manufactureYear;			// Manufacturing date
	uint16_t m_manufactureMonth;		// 
	uint16_t m_manufactureDay;			// 
	uint16_t m_configureYear;			// Configuration date
	uint16_t m_configureMonth;			//
	uint16_t m_configureDay;			//
    uint16_t m_reserve1_0;				// Reserve
    uint16_t m_reserve1_1;		// Firmware version 1
    uint16_t m_reserve1_2;		// Firmware version 2
    uint32_t m_firmwareCrc;			// Firmware Crc1
    uint32_t m_reserve2_0;			// Firmware Crc2
	uint16_t m_reserve3_0;				// Reserve 3 -- size 16bit words
	uint16_t m_reserve3_1;				//
	uint16_t m_reserve3_2;				//
	uint16_t m_reserve3_3;				//
	uint16_t m_reserve3_4;				//
	uint16_t m_reserve3_5;				//

	// set/get members
	//

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

    uint32_t firmwareCrc() const;
    void setFirmwareCrc(uint32_t value);

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
	uint16_t marker;									// Struct marker, must be 0xAC17, DO NOT MOVE THIS FIELD IN THE PACKET
	uint16_t version;									// Struct version, DO NOT MOVE THIS FIELD IN THE PACKET

	Uuid moduleUuid;									// Uniquie module identifier
	uint32_t count;										// Configurations counter

	struct CONF_IDENTIFICATION_RECORD
	{
		Uuid configurationId;							// Uniquie configuration identifier
		uint64_t date;									// Configuration date and time
		uint16_t configuratorFactoryNo;					// Configurator factory no
		char host[24];									// Host name of the first configuration
		uint32_t reserve;								// Reserve
	};

	CONF_IDENTIFICATION_RECORD firstConfiguration;		// The first configuration Id record
	CONF_IDENTIFICATION_RECORD lastConfiguration;		// The last configuration Id record

	void dump(OutputLog *log) const;

	void createFirstConfiguration(Hardware::ModuleFirmware* conf);
	void createNextConfiguration(Hardware::ModuleFirmware* conf);

	static int structVersion() {	return 1;	}
};

struct CONF_IDENTIFICATION_DATA_V2
{
	uint16_t marker;									// Struct marker, must be 0xAC17, DO NOT MOVE THIS FIELD IN THE PACKET
	uint16_t version;									// Struct version, DO NOT MOVE THIS FIELD IN THE PACKET

	Uuid moduleUuid;									// Uniquie module identifier
	uint32_t count;										// Configurations counter

	struct CONF_IDENTIFICATION_RECORD
	{
		Uuid configurationId;							// Uniquie configuration identifier
		uint64_t date;									// Configuration date and time
		char host[24];									// Host name of the first configuration
		uint32_t buildNo;								// Build number
		char buildConfig[10];							// Configuraton name, Debug/Release
		char userName[32];								// User who had built the project
	};

	CONF_IDENTIFICATION_RECORD firstConfiguration;		// The first configuration Id record
	CONF_IDENTIFICATION_RECORD lastConfiguration;		// The last configuration Id record

	void dump(OutputLog *log) const;

	void createFirstConfiguration(Hardware::ModuleFirmware* conf);
	void createNextConfiguration(Hardware::ModuleFirmware* conf);

	static int structVersion() {	return 2;	}
};

#pragma pack(pop)

typedef CONF_IDENTIFICATION_DATA_V2 CONF_IDENTIFICATION_DATA;	// Current version

using namespace Hardware;

//
//	Configurator
//
class Configurator : public QObject
{
	Q_OBJECT

public:
        Configurator(QString serialDevice, OutputLog* log, QObject* parent = nullptr);
	virtual ~Configurator();
	
protected:
	bool openConnection();
	bool closeConnection();

    bool send(int moduleUartId, ConfigureCommand opcode, uint16_t frameIndex, uint16_t blockSize, const std::vector<quint8>& requestData, CONF_HEADER* pReceivedHeader, std::vector<quint8>* replyData);

//	HANDLE openConnection();
//	bool closeConnection(HANDLE hDevice);

//	bool send(HANDLE hDevice, int moduleUartId, ConfigureCommand opcode, uint16_t frameIndex, uint16_t blockSize, const std::vector<uint8_t>& requestData, CONF_HEADER* pReceivedHeader, std::vector<uint8_t>* replyData);

	void readConfigurationWorker(int param);
	void writeConfigurationWorker(ModuleFirmware* conf);

	void dumpIdentificationData(const std::vector<quint8> &identificationData, int blockSize);

	// Slots
	//
public slots:
	void setSettings(QString device, bool showDebugInfo);
	void readConfiguration(int param);
    void writeDiagData(quint32 factoryNo, QDate manufactureDate, quint32 firmwareCrc);
	void writeConfDataFile(const QString& fileName);
	void writeConfData(ModuleFirmware* conf);
	void readFirmware(const QString &fileName);
	void eraseFlashMemory(int param);
	void cancelOperation();

	// Signals
	//
signals:
	void communicationStarted();
	void communicationFinished();
    void communicationReadFinished(int protocolVersion, std::vector<quint8> data);

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
	QString m_device;
	bool m_showDebugInfo = false;
	uint32_t m_configuratorfactoryNo = 0;

        QSerialPort *m_serialPort;

        OutputLog* m_Log;

	mutable QMutex mutex;			// m_device

	bool m_cancelFlag = false;
};


#endif // CONFIGURATOR_H
