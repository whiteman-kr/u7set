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
	class ModuleFirmware : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(int UartID READ uartId)
		Q_PROPERTY(int SSKey READ ssKey)
		Q_PROPERTY(int FrameSize READ frameSize)
		Q_PROPERTY(int FrameCount READ frameCount)

	public:
		ModuleFirmware();
		virtual ~ModuleFirmware();

		// Methods
		//
	public:
		void init(QString caption, QString subsysId, int uartId, int ssKey, int frameSize, int frameCount, int lmDescriptionNumber, const QString &projectName,
				  const QString &userName, int buildNumber, const QString& buildConfig, int changesetId);

		bool loadHeader(QString fileName, QString &errorCode);
		bool load(QString fileName, QString &errorCode);

		bool isEmpty() const;

		int frameCount() const;
		const std::vector<quint8> frame(int frameIndex) const;

		// Properties
		//
	public:
		int fileVersion() const;
		int maxFileVersion() const;

		QString caption() const;
		QString subsysId() const;
		quint16 ssKey() const;
		int uartId() const;
		int frameSize() const;
		int frameSizeWithCRC() const;
		int changesetId() const;

		QString projectName() const;
		QString userName() const;
		int buildNumber() const;
		QString buildConfig() const;
		int lmDescriptionNumber() const;

	private:
		bool loadFromFile(QString fileName, QString& errorCode, bool readDataFrames);

		bool load_version1(const QJsonObject& jConfig, bool readDataFrames);
		bool load_version2_3_4(const QJsonObject& jConfig, bool readDataFrames);
		bool load_version5_6(const QJsonObject& jConfig, bool readDataFrames, QString& errorCode);

		// Data
		//
	protected:
		int m_fileVersion = 0;
		int m_maxFileVersion = 6;	//Latest version

		QString m_caption;
		QString m_subsysId;
		quint16 m_ssKey = 0;
		int m_uartId = 0;
		int m_frameSize = 0;
		int m_frameSizeWithCRC = 0;
		int m_changesetId = 0;


		QString m_projectName;
		QString m_userName;
		int m_buildNumber = 0;
		QString m_buildConfig;
		int m_lmDescriptionNumber = 0;

		// binary data
		//
		std::vector<std::vector<quint8>> m_frames;
	};
}

