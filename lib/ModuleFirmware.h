#pragma once
#include <map>
#include <vector>
#include <QObject>
#include "../lib/Types.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------

typedef std::pair<int, QString> UartPair;

namespace Hardware
{

	struct ModuleFirmwareData
	{
		int eepromFramePayloadSize = 0;
		int eepromFrameSize = 0;

		int uartId = -1;
		QString uartType;

		// binary data
		//
		std::vector<std::vector<quint8>> frames;
	};

	struct LogicModuleInfo
	{
		QString equipmentId;
		int lmNumber;
		int channel;

		int moduleFamily;
		int customModuleFamily;
		int moduleVersion;
		int moduleType;
	};

	class ModuleFirmware
	{
	public:
		void addFirmwareData(int uartId, const QString& uartType,
				  int eepromFramePayloadSize,
				  int eepromFrameCount);

		void init(const QString& subsysId,
				  int ssKey, const QString& lmDescriptionFile,
				  int lmDescriptionNumber);

		// Data access
		//
	public:
		std::vector<UartPair> uartList() const;
		bool uartExists(int uartId) const;

		ModuleFirmwareData& firmwareData(int uartId, bool* ok);
		const ModuleFirmwareData& firmwareData(int uartId, bool* ok) const;

		int eepromFramePayloadSize(int uartId) const;
		int eepromFrameSize(int uartId) const;
		int eepromFrameCount(int uartId) const;

		const std::vector<quint8>& frame(int uartId, int frameIndex) const;

		QString subsysId() const;
		quint16 ssKey() const;
		QString lmDescriptionFile() const;
		int lmDescriptionNumber() const;

		void addLogicModuleInfo(const QString& equipmentId,
								int lmNumber,
								int channel,
								int moduleFamily,
								int customModuleFamily,
								int moduleVersion,
								int moduleType
								);
		void addLogicModuleInfo(const LogicModuleInfo& lmi);

		const std::vector<LogicModuleInfo>& logicModulesInfo() const;

	private:
		QString m_subsysId;
		quint16 m_ssKey = 0;
		QString m_lmDescriptionFile;
		int m_lmDescriptionNumber = 0;

		// Module data map, key is UartID
		//
		std::map<int, ModuleFirmwareData> m_firmwareData;

		std::vector<LogicModuleInfo> m_lmInfo;
	};

	class ModuleFirmwareStorage : public QObject
	{
		Q_OBJECT

	public:
		ModuleFirmwareStorage();
		virtual ~ModuleFirmwareStorage();

		// Methods
		//
	public:
		// Initializing and loading
		//
		void setProjectInfo(const QString& projectName, const QString& userName, int buildNumber, bool debug, int changesetId);

		bool load(QString fileName, QString* errorCode);
		bool load(const QByteArray& data, QString* errorCode);
		bool loadHeader(QString fileName, QString* errorCode);

		bool hasBinaryData() const;

		void clear();

		// Firmware operations
		//
		void createFirmware(const QString& subsysId, int ssKey, int uartId, const QString& uartType, int frameSize, int frameCount, const QString& lmDescriptionFile, int lmDescriptionNumber);
		ModuleFirmware& firmware(const QString& subsysId, bool* ok);

		bool isEmpty() const;
		QStringList subsystems() const;
		QString subsystemsString() const;

	private:
		bool parse(const QByteArray& data, bool readDataFrames, QString* errorCode);
		bool parse_version2(const QJsonObject& jConfig, bool readDataFrames, QString* errorCode);

		// Properties, for access from JS it is "public slots"
		//
	public slots:
		int fileVersion() const;
		int maxFileVersion() const;

		QString projectName() const;
		QString userName() const;
		int changesetId() const;
		int buildNumber() const;
		QString buildConfig() const;

		// Data
		//
	protected:
		int m_fileVersion = 0;
		static const int m_maxFileVersion = 2;	// Latest version

		bool m_hasBinaryData = false;

		QString m_buildSoftware;
		QString m_projectName;
		QString m_userName;
		int m_changesetId = 0;
		int m_buildNumber = 0;
		bool m_debug = false;

		std::map<QString, ModuleFirmware> m_firmwares;
	};
}

