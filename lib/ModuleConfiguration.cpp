#include "../lib/ModuleConfiguration.h"
#include "../lib/Crc.h"
#include "../lib/CUtils.h"
#include <QMap>
#include <QHash>
#include <QtEndian>

//-----------------------------------------------------------------------------

namespace Hardware
{

    ModuleFirmware::ModuleFirmware()
	{
	}

	ModuleFirmware::~ModuleFirmware()
	{
	}

	void ModuleFirmware::init(int uartId, int frameSize, int frameCount,
							  QString caption, QString subsysId, int ssKey, int lmDescriptionNumber,
							  const QString &projectName, const QString &userName, int buildNumber, const QString& buildConfig, int changesetId)
	{
		m_caption = caption;
		m_subsysId = subsysId;
		m_ssKey = ssKey;
		m_lmDescriptionNumber = lmDescriptionNumber;
		m_projectName = projectName;
		m_userName = userName;
		m_buildNumber = buildNumber;
		m_buildConfig = buildConfig;
		m_changesetId = changesetId;
		m_fileVersion = maxFileVersion();

		ModuleFirmwareData data;

		data.frameSize = frameSize;
		data.frameSizeWithCRC = frameSize + sizeof(quint64);

		data.frames.resize(frameCount);
		for (int i = 0; i < frameCount; i++)
		{
			data.frames[i].resize(data.frameSizeWithCRC);
		}

		m_firmwareData[uartId] = data;

		return;
	}

	bool ModuleFirmware::loadHeader(QString fileName, QString &errorCode)
	{
		return loadFromFile(fileName, errorCode, false);
	}

	bool ModuleFirmware::load(QString fileName, QString &errorCode)
	{
		return loadFromFile(fileName, errorCode, true);
	}

	bool ModuleFirmware::isEmpty() const
	{
		return m_firmwareData.size() == 0;
	}

	bool ModuleFirmware::firmwareExists(int uartId) const
	{
		return m_firmwareData.find(uartId) != m_firmwareData.end();
	}

	int ModuleFirmware::frameSize(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return it->second.frameSize;
	}

	int ModuleFirmware::frameSizeWithCRC(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return it->second.frameSizeWithCRC;
	}

	int ModuleFirmware::frameCount(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return static_cast<int>(it->second.frames.size());
	}

	const std::vector<quint8> ModuleFirmware::frame(int uartId, int frameIndex) const
	{
		if (frameIndex < 0 || frameIndex >= frameCount(uartId))
		{
			assert(frameIndex >= 0 && frameIndex < frameCount(uartId));
			return std::vector<quint8>();
		}

		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return std::vector<quint8>();
		}

		return it->second.frames[frameIndex];
	}

	int ModuleFirmware::fileVersion() const
	{
		return m_fileVersion;
	}

	int ModuleFirmware::maxFileVersion() const
	{
		return m_maxFileVersion;
	}


	QString ModuleFirmware::caption() const
    {
		return m_caption;
    }

	QString ModuleFirmware::subsysId() const
	{
		return m_subsysId;
	}

	quint16 ModuleFirmware::ssKey() const
	{
		return m_ssKey;
	}

	int ModuleFirmware::changesetId() const
	{
		return m_changesetId;
	}

	QString ModuleFirmware::projectName() const
	{
		return m_projectName;
	}

	QString ModuleFirmware::userName() const
	{
		return m_userName;
	}

	int ModuleFirmware::buildNumber() const
	{
		return m_buildNumber;
	}

	QString ModuleFirmware::buildConfig() const
	{
		return m_buildConfig;
	}

	int ModuleFirmware::lmDescriptionNumber() const
	{
		return m_lmDescriptionNumber;
	}

	bool ModuleFirmware::loadFromFile(QString fileName, QString& errorCode, bool readDataFrames)
	{
		errorCode.clear();
		m_firmwareData.clear();

		QFile file(fileName);
		if (file.open(QIODevice::ReadOnly)  == false)
		{
			return false;
		}

		QByteArray data;
		data = file.readAll();

		file.close();

		QJsonDocument document = QJsonDocument::fromJson(data);

		if (document.isEmpty() == true || document.isNull() == true || document.isObject() == false)
		{
			return false;
		}

		QJsonObject jConfig = document.object();

		if (jConfig.value("fileVersion").isUndefined() == true)
		{
			m_fileVersion = 1;	// in old files there is no version information
		}
		else
		{
			m_fileVersion = (int)jConfig.value("fileVersion").toInt();
		}

		switch (m_fileVersion)
		{
		case 1:
			return load_version1(jConfig, readDataFrames, errorCode);
		default:
			errorCode = tr("This file version is not supported. Max supported version is %1.").arg(maxFileVersion());
			return false;
		}

	}

	bool ModuleFirmware::load_version1(const QJsonObject& jConfig, bool readDataFrames, QString& errorCode)
	{

		if (jConfig.value("projectName").isUndefined() == true)
		{
			return false;
		}
		m_projectName = jConfig.value("projectName").toString();

		if (jConfig.value("userName").isUndefined() == true)
		{
			return false;
		}
		m_userName = jConfig.value("userName").toString();

		if (jConfig.value("caption").isUndefined() == true)
		{
			return false;
		}
		m_caption = jConfig.value("caption").toString();

		if (jConfig.value("subsysId").isUndefined() == true)
		{
			return false;
		}
		m_subsysId = jConfig.value("subsysId").toString();

		if (jConfig.value("buildConfig").isUndefined() == true)
		{
			m_buildConfig.clear();
		}
		else
		{
			m_buildConfig = jConfig.value("buildConfig").toString();
		}

		if (jConfig.value("buildNumber").isUndefined() == true)
		{
			m_buildNumber = 0;
		}
		else
		{
			m_buildNumber = (int)jConfig.value("buildNumber").toDouble();
		}

		if (jConfig.value("lmDescriptionNumber").isUndefined() == true)
		{
			m_lmDescriptionNumber = 0;
		}
		else
		{
			m_lmDescriptionNumber = (int)jConfig.value("lmDescriptionNumber").toDouble();
		}

		if (jConfig.value("changesetId").isUndefined() == true)
		{
			return false;
		}
		m_changesetId = (int)jConfig.value("changesetId").toDouble();

		//

		if (readDataFrames == false)
		{
			return true;
		}


		if (jConfig.value("firmwaresCount").isUndefined() == true)
		{
			return false;
		}
		int firmwaresCount = (int)jConfig.value("firmwaresCount").toDouble();

		for (int f = 0; f < firmwaresCount; f++)
		{
			QJsonValue jFirmware = jConfig.value("z_firmware_" + QString::number(f));
			if (jFirmware.isUndefined() == true || jFirmware.isObject() == false)
			{
				assert(false);
				return false;
			}

			QJsonObject jFirmwareObject = jFirmware.toObject();

			if (jFirmwareObject.value("framesCount").isUndefined() == true)
			{
				return false;
			}
			int framesCount = (int)jFirmwareObject.value("framesCount").toDouble();

			//

			ModuleFirmwareData data;

			if (jFirmwareObject.value("frameSize").isUndefined() == true)
			{
				return false;
			}
			data.frameSize = (int)jFirmwareObject.value("frameSize").toDouble();

			data.frameSizeWithCRC = (int)jFirmwareObject.value("frameSizeWithCRC").toDouble();

			if (data.frameSizeWithCRC <= data.frameSize)
			{
				assert(false);
				data.frameSizeWithCRC = data.frameSize + sizeof(quint64);
			}

			std::vector<quint8> frameVec;
			frameVec.resize(data.frameSizeWithCRC);

			quint16* framePtr = (quint16*)frameVec.data();

			int frameStringWidth = -1;
			int linesCount = 0;

			for (int v = 0; v < framesCount; v++)
			{

				QJsonValue jFrameVal = jFirmwareObject.value("z_frame_" + QString::number(v).rightJustified(4, '0'));
				if (jFrameVal.isUndefined() == true || jFrameVal.isObject() == false)
				{
					assert(false);
					return false;
				}

				QJsonObject jFrame = jFrameVal.toObject();

				if (jFrame.value("frameIndex").isUndefined() == true)
				{
					assert(false);
					return false;
				}

				if (frameStringWidth == -1)
				{
					QString firstString = jFrame.value("data0000").toString();

					frameStringWidth = firstString.split(' ').size();
					if (frameStringWidth == 0)
					{
						assert(false);
						return false;
					}

					linesCount = ceil((float)data.frameSize / 2 / frameStringWidth);
				}

				int dataPos = 0;

				quint16* ptr = framePtr;

				for (int l = 0; l < linesCount; l++)
				{
					QString stringName = "data" + QString::number(l * frameStringWidth, 16).rightJustified(4, '0');

					QJsonValue v = jFrame.value(stringName);

					if (v.isUndefined() == true)
					{
						assert(false);
						return false;
					}

					QString stringValue = v.toString();

					for (QString& s : stringValue.split(' ')) // split takes much time, try to optimize
					{
						bool ok = false;
						quint16 v = s.toUInt(&ok, 16);

						if (ok == false)
						{
							assert(false);
							return false;
						}

						if (dataPos >= data.frameSizeWithCRC / sizeof(quint16))
						{
							assert(false);
							break;
						}

						dataPos++;

						*ptr++ = qToBigEndian(v);
					}
				}

				if (Crc::checkDataBlockCrc(v, frameVec) == false)
				{
					errorCode = tr("File data is corrupt, CRC check error in frame %1.").arg(v);
					return false;
				}

				data.frames.push_back(frameVec);
			}

			if (jFirmwareObject.value("uartId").isUndefined() == true)
			{
				return false;
			}
			int uartId = (int)jFirmwareObject.value("uartId").toDouble();

			m_firmwareData[uartId] = data;
		}

		return true;
	}

}
