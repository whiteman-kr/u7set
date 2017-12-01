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


	class ModuleFirmware : public QObject
	{
		Q_OBJECT

	public:
		ModuleFirmware();
		virtual ~ModuleFirmware();

		// Methods
		//
	public:
		void init(int uartId,
				  int eepromFramePayloadSize,
				  int eepromFrameCount,
				  QString caption,
				  QString subsysId,
				  int ssKey,
				  int lmDescriptionNumber,
				  QString projectName,
				  QString userName,
				  int buildNumber,
				  QString buildConfig,
				  int changesetId);

		bool loadHeader(QString fileName, QString &errorCode);
		bool load(QString fileName, QString &errorCode);

		// Data access
		//
	public:
		bool isEmpty() const;

		std::vector<UartPair> uartList() const;

		bool uartExists(E::UartID) const;
		bool uartExists(int uartId) const;

		int eepromFramePayloadSize(E::UartID uartId) const;
		int eepromFramePayloadSize(int uartId) const;

		int eepromFrameSize(E::UartID uartId) const;
		int eepromFrameSize(int uartId) const;

		int eepromFrameCount(E::UartID uartId) const;
		int eepromFrameCount(int uartId) const;

		const std::vector<quint8>& frame(E::UartID, int frameIndex) const;
		const std::vector<quint8>& frame(int uartId, int frameIndex) const;

		// Properties, for access from JS it is "public slots"
		//
	public slots:
		int fileVersion() const;
		int maxFileVersion() const;

		QString caption() const;
		QString subsysId() const;
		quint16 ssKey() const;
		int changesetId() const;

		QString projectName() const;
		QString userName() const;
		int buildNumber() const;
		QString buildConfig() const;
		int lmDescriptionNumber() const;

	private:
		bool loadFromFile(QString fileName, QString& errorCode, bool readDataFrames);
		bool load_version1(const QJsonObject& jConfig, bool readDataFrames, QString& errorCode);

		// Data
		//
	protected:
		int m_fileVersion = 0;
		int m_maxFileVersion = 1;	//Latest version

		QString m_caption;
		QString m_subsysId;
		quint16 m_ssKey = 0;
		int m_changesetId = 0;

		QString m_projectName;
		QString m_userName;
		int m_buildNumber = 0;
		QString m_buildConfig;
		int m_lmDescriptionNumber = 0;

		// Frame data map, key is UartID
		//
		std::map<int, ModuleFirmwareData> m_firmwareData;
	};
}

