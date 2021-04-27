#pragma once
#include <map>
#include <vector>
#include <QObject>
#include "../CommonLib/Types.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------

using UartPair = std::pair<int, QString>;

namespace Hardware
{

	struct ModuleFirmwareData
	{
		int eepromFramePayloadSize = 0;
		int eepromFrameSize = 0;
		int maxFrameIndex = 0;	// Used for statistics, contains maximum frame index

		int uartId = -1;
		QString uartType;

		// binary data
		//
		std::vector<std::vector<quint8>> frames;

		[[nodiscard]] QByteArray toByteArray() const;
	};


	struct LogicModuleInfo
	{
		QString equipmentId;
		QString subsystemId;
		int lmNumber = 0;
		E::Channel channel = E::Channel::A;

		int moduleFamily = 0;
		int customModuleFamily = 0;
		int moduleVersion = 0;
		int moduleType = 0;
	};


	class ModuleFirmware
	{
	public:
		void addFirmwareData(int uartId,
							 const QString& uartType,
							 int eepromFramePayloadSize,
							 int eepromFrameCount);

		void init(const QString& subsysId,
				  int ssKey,
				  const QString& lmDescriptionFile,
				  int lmDescriptionNumber);

		// Data access
		//
	public:
		[[nodiscard]] std::vector<UartPair> uartList() const;
		[[nodiscard]] bool uartExists(int uartId) const;

		[[nodiscard]] ModuleFirmwareData& firmwareData(int uartId, bool* ok);
		[[nodiscard]] const ModuleFirmwareData& firmwareData(int uartId, bool* ok) const;

		[[nodiscard]] int eepromFramePayloadSize(int uartId) const;
		[[nodiscard]] int eepromFrameSize(int uartId) const;
		[[nodiscard]] int eepromFrameCount(int uartId) const;

		[[nodiscard]] const std::vector<quint8>& frame(int uartId, int frameIndex) const;

		[[nodiscard]] QString subsysId() const;
		[[nodiscard]] quint16 ssKey() const;
		[[nodiscard]] QString lmDescriptionFile() const;
		[[nodiscard]] int lmDescriptionNumber() const;

		void addLogicModuleInfo(const QString& equipmentId,
								const QString& subsystemId,
								int lmNumber,
								E::Channel channel,
								int moduleFamily,
								int customModuleFamily,
								int moduleVersion,
								int moduleType
								);
		void addLogicModuleInfo(const LogicModuleInfo& lmi);

		[[nodiscard]] const std::vector<LogicModuleInfo>& logicModulesInfo() const;

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
		void setProjectInfo(const QString& projectName, const QString& userName, int buildNumber, int changesetId);

		bool load(QString fileName, QString* errorCode);
		bool load(const QByteArray& data, QString* errorCode);
		bool loadHeader(QString fileName, QString* errorCode);

		[[nodiscard]] bool hasBinaryData() const;

		void clear();

		// Firmware operations
		//
		void createFirmware(const QString& subsysId, int ssKey, int uartId, const QString& uartType, int frameSize, int frameCount, const QString& lmDescriptionFile, int lmDescriptionNumber);
		[[nodiscard]] ModuleFirmware& firmware(const QString& subsysId, bool* ok);

		[[nodiscard]] bool isEmpty() const;
		[[nodiscard]] QStringList subsystems() const;
		[[nodiscard]] QString subsystemsString() const;

	private:
		bool parse(const QByteArray& data, bool readDataFrames, QString* errorCode);
		bool parse_version2(const QJsonObject& jConfig, bool readDataFrames, QString* errorCode);

		// Properties, for access from JS it is "public slots"
		//
	public slots:
		[[nodiscard]] int fileVersion() const;
		[[nodiscard]] int maxFileVersion() const;
		[[nodiscard]] QString projectName() const;
		[[nodiscard]] QString userName() const;
		[[nodiscard]] int changesetId() const;
		[[nodiscard]] int buildNumber() const;

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

		std::map<QString, ModuleFirmware> m_firmwares;
	};
}

