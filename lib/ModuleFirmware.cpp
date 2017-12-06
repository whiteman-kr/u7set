#include "../lib/ModuleFirmware.h"
#include "../lib/Crc.h"
#include <QFile>
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

	void ModuleFirmware::init(int uartId,
							  const QString& uartType,
							  int frameSize,
							  int frameCount,
							  const QString& subsysId,
							  int ssKey,
							  const QString& lmDescriptionFile,
							  int lmDescriptionNumber)
	{
		m_subsysId = subsysId;
		m_ssKey = ssKey;
		m_lmDescriptionFile = lmDescriptionFile;
		m_lmDescriptionNumber = lmDescriptionNumber;

		ModuleFirmwareData data;

		data.uartId = uartId;
		data.uartType = uartType;
		data.eepromFramePayloadSize = frameSize;
		data.eepromFrameSize = frameSize + sizeof(quint64);

		data.frames.resize(frameCount);
		for (int i = 0; i < frameCount; i++)
		{
			data.frames[i].resize(data.eepromFrameSize);
		}

		m_firmwareData[uartId] = data;

		return;
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

	ModuleFirmwareData& ModuleFirmware::firmwareData(int uartId, bool* ok)
	{
static ModuleFirmwareData err;
		if (ok == false)
		{
			assert(ok);
			return err;
		}

		if (uartExists(uartId) == false)
		{
			*ok = false;
			return err;
		}

		*ok = true;
		return m_firmwareData[uartId];
	}

	int ModuleFirmware::eepromFramePayloadSize(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return it->second.eepromFramePayloadSize;
	}

	int ModuleFirmware::eepromFrameSize(int uartId) const
	{
		auto it = m_firmwareData.find(uartId);
		if (it == m_firmwareData.end())
		{
			assert(false);
			return -1;
		}

		return it->second.eepromFrameSize;
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

	QString ModuleFirmware::subsysId() const
	{
		return m_subsysId;
	}

	quint16 ModuleFirmware::ssKey() const
	{
		return m_ssKey;
	}

	QString ModuleFirmware::lmDescriptionFile() const
	{
		return m_lmDescriptionFile;
	}

	int ModuleFirmware::lmDescriptionNumber() const
	{
		return m_lmDescriptionNumber;
	}

	void ModuleFirmware::addLogicModuleInfo(
							const QString& equipmentId,
							int lmNumber,
							int subsystemChannel,
							int moduleFamily,
							int customModuleVersion,
							int moduleVersion,
							int moduleType
							)
	{
		LogicModuleInfo info;

		info.equipmentId = equipmentId;
		info.lmNumber = lmNumber;
		info.subsystemChannel = subsystemChannel;
		info.moduleFamily = moduleFamily;
		info.customModuleVersion = customModuleVersion;
		info.moduleVersion = moduleVersion;
		info.moduleType = moduleType;

		m_lmInfo.push_back(info);
	}

	const std::vector<LogicModuleInfo>& ModuleFirmware::logicModulesInfo() const
	{
		return m_lmInfo;
	}

	//
	// ModuleFirmware
	//

	ModuleFirmwareStorage::ModuleFirmwareStorage()
	{
	}

	ModuleFirmwareStorage::~ModuleFirmwareStorage()
	{
	}

	void ModuleFirmwareStorage::setProjectInfo(const QString& projectName, const QString& userName, int buildNumber, bool debug, int changesetId)
	{
		m_projectName = projectName;
		m_userName = userName;
		m_buildNumber = buildNumber;
		m_debug = debug;
		m_changesetId = changesetId;
	}

	bool ModuleFirmwareStorage::load(QString fileName, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		QFile file(fileName);
		if (file.open(QIODevice::ReadOnly)  == false)
		{
			*errorCode = QString("Open file %1 error: ")
							.arg(fileName)
							.arg(file.errorString());
			return false;
		}

		QByteArray data;
		data = file.readAll();
		file.close();

		return parse(data, true, errorCode);
	}

	bool ModuleFirmwareStorage::load(const QByteArray& data, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		return parse(data, true, errorCode);
	}

	bool ModuleFirmwareStorage::loadHeader(QString fileName, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		QFile file(fileName);
		if (file.open(QIODevice::ReadOnly)  == false)
		{
			*errorCode = QString("Open file %1 error: ")
							.arg(fileName)
							.arg(file.errorString());
			return false;
		}

		QByteArray data;
		data = file.readAll();
		file.close();

		return parse(data, false, errorCode);
	}

	void ModuleFirmwareStorage::createFirmware(const QString& subsysId,
											   int ssKey,
											   int uartId,
											   const QString& uartType,
											   int frameSize,
											   int frameCount,
											   const QString& lmDescriptionFile,
											   int lmDescriptionNumber)
	{
		bool newSubsystem = m_firmwares.count(subsysId) == 0;

		ModuleFirmware& subsystemData = m_firmwares[subsysId];

		if (newSubsystem == true || subsystemData.uartExists(uartId) == false)
		{
			subsystemData.init(uartId, uartType, frameSize, frameCount, subsysId, ssKey, lmDescriptionFile, lmDescriptionNumber);
		}

		return;
	}

	ModuleFirmware& ModuleFirmwareStorage::firmware(const QString& subsysId, bool* ok)
	{
static ModuleFirmware err;
		if (ok == nullptr)
		{
			assert(ok);
			return err;
		}

		if (m_firmwares.find(subsysId) == m_firmwares.end())
		{
			*ok = false;
			return err;
		}

		*ok = true;
		return m_firmwares.at(subsysId);
	}

	QStringList ModuleFirmwareStorage::subsystems()
	{
		QStringList result;

		for (auto fw : m_firmwares)
		{
			result.push_back(fw.first);
		}

		return result;
	}

	bool ModuleFirmwareStorage::parse(const QByteArray& data, bool readDataFrames, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		m_firmwares.clear();

		QJsonParseError error;
		QJsonDocument document = QJsonDocument::fromJson(data, &error);

		if (document.isEmpty() == true || document.isNull() == true || document.isObject() == false)
		{
			*errorCode = "Parse firmware error: " + error.errorString();
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
		//case 1:
			//return parse_version1(jConfig, readDataFrames, errorCode);
		default:
			*errorCode = tr("This file version is not supported. Max supported version is %1.").arg(maxFileVersion());
			return false;
		}
	}

	/*bool ModuleFirmware::parse_version1(const QJsonObject& jConfig, bool readDataFrames, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		if (jConfig.value(QLatin1String("projectName")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field projectName";
			return false;
		}
		m_projectName = jConfig.value(QLatin1String("projectName")).toString();

		if (jConfig.value(QLatin1String("userName")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field userName";
			return false;
		}
		m_userName = jConfig.value(QLatin1String("userName")).toString();

		if (jConfig.value(QLatin1String("caption")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field caption";
			return false;
		}
		m_caption = jConfig.value(QLatin1String("caption")).toString();

		if (jConfig.value(QLatin1String("subsysId")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field subsysId";
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
			*errorCode = "Parse firmware error: cant find field changesetId";
			return false;
		}
		m_changesetId = jConfig.value(QLatin1String("changesetId")).toInt();

		//--
		//
		if (jConfig.value(QLatin1String("firmwaresCount")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field firmwaresCount";
			return false;
		}
		int firmwaresCount = jConfig.value(QLatin1String("firmwaresCount")).toInt();

		for (int f = 0; f < firmwaresCount; f++)
		{
			QString zFirmware = "z_firmware_" + QString::number(f);

			QJsonValue jFirmware = jConfig.value(zFirmware);
			if (jFirmware.isUndefined() == true || jFirmware.isObject() == false)
			{
				assert(false);
				*errorCode = "Parse firmware error: cant find field " + zFirmware;
				return false;
			}

			QJsonObject jFirmwareObject = jFirmware.toObject();

			if (jFirmwareObject.value(QLatin1String("eepromFrameCount")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field eepromFrameCount";
				return false;
			}
			int framesCount = jFirmwareObject.value(QLatin1String("eepromFrameCount")).toInt();

			//

			ModuleFirmwareData data;

			if (jFirmwareObject.value(QLatin1String("eepromFramePayloadSize")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field eepromFramePayloadSize";
				return false;
			}
			data.frameSize = jFirmwareObject.value(QLatin1String("eepromFramePayloadSize")).toInt();

			data.frameSizeWithCRC = jFirmwareObject.value(QLatin1String("eepromFrameSize")).toInt();

			if (data.frameSizeWithCRC <= data.frameSize)
			{
				assert(false);
				data.frameSizeWithCRC = data.frameSize + sizeof(quint64);
			}

			if (jFirmwareObject.value(QLatin1String("uartId")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field uartId";
				return false;
			}
			data.uartId = jFirmwareObject.value(QLatin1String("uartId")).toInt();

			if (jFirmwareObject.value(QLatin1String("uartType")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field uartType";
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
					QString zFrame = "z_frame_" + QString::number(v).rightJustified(4, '0');

					QJsonValue jFrameVal = jFirmwareObject.value(zFrame);
					if (jFrameVal.isUndefined() == true || jFrameVal.isObject() == false)
					{
						assert(false);
						*errorCode = "Parse firmware error: cant find field " + zFrame;
						return false;
					}

					QJsonObject jFrame = jFrameVal.toObject();

					if (jFrame.value(QLatin1String("frameIndex")).isUndefined() == true)
					{
						assert(false);
						*errorCode = "Parse firmware error: cant find frameIndex of " + zFrame;
						return false;
					}

					if (frameStringWidth == -1)
					{
						QString firstString = jFrame.value(QLatin1String("data0000")).toString();

						frameStringWidth = firstString.split(' ').size();
						if (frameStringWidth == 0)
						{
							assert(false);
							*errorCode = QString("Parse firmware error: cant find %1 of ").arg(firstString) + zFrame;
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
							*errorCode = QString("Parse firmware error: cant find %1 of ").arg(stringName) + zFrame;
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
						*errorCode = tr("File data is corrupt, CRC check error in frame %1.").arg(v);
						return false;
					}

					data.frames.push_back(frameVec);
				}
			}

			m_firmwareData[data.uartId] = data;
		}

		return true;
	}*/



	int ModuleFirmwareStorage::fileVersion() const
	{
		return m_fileVersion;
	}

	int ModuleFirmwareStorage::maxFileVersion() const
	{
		return m_maxFileVersion;
	}

	int ModuleFirmwareStorage::changesetId() const
	{
		return m_changesetId;
	}

	QString ModuleFirmwareStorage::projectName() const
	{
		return m_projectName;
	}

	QString ModuleFirmwareStorage::userName() const
	{
		return m_userName;
	}

	int ModuleFirmwareStorage::buildNumber() const
	{
		return m_buildNumber;
	}

	QString ModuleFirmwareStorage::buildConfig() const
	{
		return m_debug ? "debug" : "release";
	}


}
