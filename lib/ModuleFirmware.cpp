#include "../lib/ModuleFirmware.h"
#include "../lib/Crc.h"
#include <QMap>
#include <QHash>
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

//-----------------------------------------------------------------------------

namespace Hardware
{

    ModuleFirmware::ModuleFirmware()
	{
	}

	ModuleFirmware::~ModuleFirmware()
	{
	}

	void ModuleFirmware::init(int uartId,
							  int frameSize,
							  int frameCount,
							  QString caption,
							  QString subsysId,
							  int ssKey,
							  int lmDescriptionNumber,
							  QString projectName,
							  QString userName,
							  int buildNumber,
							  QString buildConfig,
							  int changesetId)
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

		data.uartId = uartId;
		data.uartType = E::valueToString<E::UartID>(uartId);
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

	std::vector<UartPair> ModuleFirmware::uartList() const
	{
		std::vector<UartPair> result;

		for (auto it : m_firmwareData)
		{
			ModuleFirmwareData& data = it.second;
			result.push_back(std::make_pair(it.first, data.uartType));
		}

		return result;
	}

	bool ModuleFirmware::uartExists(int uartId) const
	{
		return m_firmwareData.find(uartId) != m_firmwareData.end();
	}

	int ModuleFirmware::eepromFramePayloadSize(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return it->second.frameSize;
	}

	int ModuleFirmware::eepromFrameSize(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return it->second.frameSizeWithCRC;
	}

	int ModuleFirmware::eepromFrameCount(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return static_cast<int>(it->second.frames.size());
	}



	const std::vector<quint8>& ModuleFirmware::frame(int uartId, int frameIndex) const
	{
static const std::vector<quint8> err;

		if (frameIndex < 0 || frameIndex >= eepromFrameCount(uartId))
		{
			assert(frameIndex >= 0 && frameIndex < eepromFrameCount(uartId));
			return err;
		}

		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return err;
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

		if (jConfig.value(QLatin1String("fileVersion")).isUndefined() == true)
		{
			m_fileVersion = 1;	// in old files there is no version information
		}
		else
		{
			m_fileVersion = jConfig.value(QLatin1String("fileVersion")).toInt();
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
		if (jConfig.value(QLatin1String("projectName")).isUndefined() == true)
		{
			return false;
		}
		m_projectName = jConfig.value(QLatin1String("projectName")).toString();

		if (jConfig.value(QLatin1String("userName")).isUndefined() == true)
		{
			return false;
		}
		m_userName = jConfig.value(QLatin1String("userName")).toString();

		if (jConfig.value(QLatin1String("caption")).isUndefined() == true)
		{
			return false;
		}
		m_caption = jConfig.value(QLatin1String("caption")).toString();

		if (jConfig.value(QLatin1String("subsysId")).isUndefined() == true)
		{
			return false;
		}
		m_subsysId = jConfig.value(QLatin1String("subsysId")).toString();

		if (jConfig.value(QLatin1String("buildConfig")).isUndefined() == true)
		{
			m_buildConfig.clear();
		}
		else
		{
			m_buildConfig = jConfig.value(QLatin1String("buildConfig")).toString();
		}

		if (jConfig.value(QLatin1String("buildNumber")).isUndefined() == true)
		{
			m_buildNumber = 0;
		}
		else
		{
			m_buildNumber = jConfig.value(QLatin1String("buildNumber")).toInt();
		}

		if (jConfig.value(QLatin1String("lmDescriptionNumber")).isUndefined() == true)
		{
			m_lmDescriptionNumber = 0;
		}
		else
		{
			m_lmDescriptionNumber = jConfig.value(QLatin1String("lmDescriptionNumber")).toInt();
		}

		if (jConfig.value(QLatin1String("changesetId")).isUndefined() == true)
		{
			return false;
		}
		m_changesetId = jConfig.value(QLatin1String("changesetId")).toInt();

		//--
		//
		if (jConfig.value(QLatin1String("firmwaresCount")).isUndefined() == true)
		{
			return false;
		}
		int firmwaresCount = jConfig.value(QLatin1String("firmwaresCount")).toInt();

		for (int f = 0; f < firmwaresCount; f++)
		{
			QJsonValue jFirmware = jConfig.value("z_firmware_" + QString::number(f));
			if (jFirmware.isUndefined() == true || jFirmware.isObject() == false)
			{
				assert(false);
				return false;
			}

			QJsonObject jFirmwareObject = jFirmware.toObject();

			if (jFirmwareObject.value(QLatin1String("framesCount")).isUndefined() == true)
			{
				return false;
			}
			int framesCount = jFirmwareObject.value(QLatin1String("framesCount")).toInt();

			//

			ModuleFirmwareData data;

			if (jFirmwareObject.value(QLatin1String("frameSize")).isUndefined() == true)
			{
				return false;
			}
			data.frameSize = jFirmwareObject.value(QLatin1String("frameSize")).toInt();

			data.frameSizeWithCRC = jFirmwareObject.value(QLatin1String("frameSizeWithCRC")).toInt();

			if (data.frameSizeWithCRC <= data.frameSize)
			{
				assert(false);
				data.frameSizeWithCRC = data.frameSize + sizeof(quint64);
			}

			if (jFirmwareObject.value(QLatin1String("uartId")).isUndefined() == true)
			{
				return false;
			}
			data.uartId = jFirmwareObject.value(QLatin1String("uartId")).toInt();

			if (jFirmwareObject.value(QLatin1String("uartType")).isUndefined() == true)
			{
				return false;
			}
			data.uartType = jFirmwareObject.value(QLatin1String("uartType")).toString();

			if (readDataFrames == true)
			{
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

					if (jFrame.value(QLatin1String("frameIndex")).isUndefined() == true)
					{
						assert(false);
						return false;
					}

					if (frameStringWidth == -1)
					{
						QString firstString = jFrame.value(QLatin1String("data0000")).toString();

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
			}

			m_firmwareData[data.uartId] = data;
		}

		return true;
	}

}
