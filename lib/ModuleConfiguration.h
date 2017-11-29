#pragma once

#include "../lib/ProtoSerialization.h"
#include "../lib/DeviceObject.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------


namespace Hardware
{

	struct ModuleFirmwareData
	{
		int frameSize = 0;
		int frameSizeWithCRC = 0;

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
		void init(int uartId, int frameSize, int frameCount, QString caption, QString subsysId, int ssKey, int lmDescriptionNumber, const QString &projectName,
				  const QString &userName, int buildNumber, const QString& buildConfig, int changesetId);

		bool loadHeader(QString fileName, QString &errorCode);
		bool load(QString fileName, QString &errorCode);

		// Data access
		//
		bool isEmpty() const;

		bool firmwareExists(int uartId) const;

		int frameSize(int uartId) const;
		int frameSizeWithCRC(int uartId) const;
		int frameCount(int uartId) const;
		const std::vector<quint8> frame(int uartId, int frameIndex) const;

		// Properties
		//
	public:
		int fileVersion() const;
		int maxFileVersion() const;

		QString caption() const;
		QString subsysId() const;
		quint16 ssKey() const;
		int changesetId() const;

		QString projectName() const;
		QString userName() const;
		Q_INVOKABLE int buildNumber() const;
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

		// Frame data map, key is uartID
		//
		std::map<int, ModuleFirmwareData> m_firmwareData;

	};
}

