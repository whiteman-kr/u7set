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
		int frameSize = 0;
		int frameSizeWithCRC = 0;

		int uartId = -1;
		QString uartType;

		// binary data
		//
		std::vector<std::vector<quint8>> frames;
	};

	class ModuleFirmware
	{
	public:
		void init(int uartId, QString uartType,
				  int eepromFramePayloadSize,
				  int eepromFrameCount,
				  QString caption,
				  QString subsysId,
				  int ssKey,
				  int lmDescriptionNumber);



		// Data access
		//
	public:
		bool isEmpty() const;

		std::vector<UartPair> uartList() const;
		bool uartExists(int uartId) const;

		int eepromFramePayloadSize(int uartId) const;
		int eepromFrameSize(int uartId) const;
		int eepromFrameCount(int uartId) const;

		ModuleFirmwareData& firmwareData(int uartId, bool* ok);

		const std::vector<quint8>& frame(int uartId, int frameIndex) const;

		QString caption() const;
		QString subsysId() const;
		quint16 ssKey() const;
		int lmDescriptionNumber() const;

	private:
		QString m_caption;
		QString m_subsysId;
		quint16 m_ssKey = 0;
		int m_lmDescriptionNumber = 0;

		// Module data map, key is UartID
		//
		std::map<int, ModuleFirmwareData> m_firmwareData;

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
		void setProjectInfo(const QString& projectName, const QString& userName, int buildNumber, bool debug, int changesetId);

		bool load(QString fileName, QString* errorCode);
		bool load(const QByteArray& data, QString* errorCode);

		bool loadHeader(QString fileName, QString* errorCode);

		QStringList subsystemsList();

		void createSubsystemFirmware(QString caption, QString subsysId, int ssKey, int uartId, QString uartType, int frameSize, int frameCount, int lmDescriptionNumber);

		ModuleFirmware& moduleFirmware(const QString& subsysId, bool* ok);

	private:
		bool parse(const QByteArray& data, bool readDataFrames, QString* errorCode);
		bool parse_version1(const QJsonObject& jConfig, bool readDataFrames, QString* errorCode);

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
		int m_maxFileVersion = 1;	//Latest version

		QString m_projectName;
		QString m_userName;
		int m_changesetId = 0;
		int m_buildNumber = 0;
		bool m_debug = false;

		std::map<QString, ModuleFirmware> m_firmwares;

	};
}

