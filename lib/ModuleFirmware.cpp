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

	void ModuleFirmware::addFirmwareData(int uartId,
							  const QString& uartType,
							  int eepromFramePayloadSize,
							  int frameCount)

	{
		ModuleFirmwareData data;

		data.uartId = uartId;
		data.uartType = uartType;
		data.eepromFramePayloadSize = eepromFramePayloadSize;
		data.eepromFrameSize = eepromFramePayloadSize + sizeof(quint64);

		data.frames.resize(frameCount);
		for (int i = 0; i < frameCount; i++)
		{
			std::vector<quint8>& vec = data.frames[i];

			vec.resize(data.eepromFrameSize);

			for (int j = 0; j < data.eepromFrameSize; j++)
			{
				vec[j] = 0;
			}
		}

		m_firmwareData[uartId] = data;

		return;
	}

	void ModuleFirmware::init(const QString& subsysId,
			  int ssKey, const QString& lmDescriptionFile,
			  int lmDescriptionNumber)
	{
		m_subsysId = subsysId;
		m_ssKey = ssKey;
		m_lmDescriptionFile = lmDescriptionFile;
		m_lmDescriptionNumber = lmDescriptionNumber;
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

	const ModuleFirmwareData& ModuleFirmware::firmwareData(int uartId, bool* ok) const
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
		return m_firmwareData.at(uartId);
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
							int channel,
							int moduleFamily,
							int customModuleFamily,
							int moduleVersion,
							int moduleType
							)
	{
		LogicModuleInfo info;

		info.equipmentId = equipmentId;
		info.lmNumber = lmNumber;
		info.channel = channel;
		info.moduleFamily = moduleFamily;
		info.customModuleFamily = customModuleFamily;
		info.moduleVersion = moduleVersion;
		info.moduleType = moduleType;

		addLogicModuleInfo(info);

	}

	void ModuleFirmware::addLogicModuleInfo(const LogicModuleInfo& lmi)
	{
		m_lmInfo.push_back(lmi);
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
			*errorCode = QString("Open file %1 error: %2")
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
			*errorCode = QString("Open file %1 error: %2")
							.arg(fileName)
							.arg(file.errorString());
			return false;
		}

		QByteArray data;
		data = file.readAll();
		file.close();

		return parse(data, false, errorCode);
	}

	bool ModuleFirmwareStorage::hasBinaryData() const
	{
		return m_hasBinaryData;
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

		if (newSubsystem == true)
		{
			subsystemData.init(subsysId, ssKey, lmDescriptionFile, lmDescriptionNumber);
		}

		if (subsystemData.uartExists(uartId) == false)
		{
			subsystemData.addFirmwareData(uartId, uartType, frameSize, frameCount);
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

	bool ModuleFirmwareStorage::isEmpty() const
	{
		return m_firmwares.empty() == true;
	}

	QStringList ModuleFirmwareStorage::subsystems() const
	{
		QStringList result;

		for (auto fw : m_firmwares)
		{
			result.push_back(fw.first);
		}

		return result;
	}

	QString ModuleFirmwareStorage::subsystemsString() const
	{
		QString result;

		for (auto fw : m_firmwares)
		{
			result.push_back(fw.first + " ");
		}

		return result.trimmed();
	}

	bool ModuleFirmwareStorage::parse(const QByteArray& data, bool readDataFrames, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		m_firmwares.clear();

		m_hasBinaryData = false;

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

		bool result = false;

		switch (m_fileVersion)
		{
		case 1:
			*errorCode = tr("This file version is not supported.");
			return false;
		case 2:
			result = parse_version2(jConfig, readDataFrames, errorCode);
			break;
		default:
			*errorCode = tr("This file version is not supported. Max supported version is %1.").arg(maxFileVersion());
			return false;
		}

		if (result == false)
		{
			m_hasBinaryData = false;
		}

		return result;
	}

	bool ModuleFirmwareStorage::parse_version2(const QJsonObject& jConfig, bool readBinaryData, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			assert(errorCode);
			return false;
		}

		// Load general parameters
		//

		if (jConfig.value(QLatin1String("buildSoftware")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field buildSoftware";
			return false;
		}
		m_buildSoftware = jConfig.value(QLatin1String("buildSoftware")).toString();

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

		if (jConfig.value(QLatin1String("buildConfig")).isUndefined() == true)
		{
			m_debug = true;
		}
		else
		{

			m_debug = jConfig.value(QLatin1String("buildConfig")).toString() == "debug";
		}

		if (jConfig.value(QLatin1String("buildNumber")).isUndefined() == true)
		{
			m_buildNumber = 0;
		}
		else
		{
			m_buildNumber = jConfig.value(QLatin1String("buildNumber")).toInt();
		}

		if (jConfig.value(QLatin1String("changesetId")).isUndefined() == true)
		{
			*errorCode = "Parse firmware error: cant find field changesetId";
			return false;
		}
		m_changesetId = jConfig.value(QLatin1String("changesetId")).toInt();

		// Load subsystems information
		//

		QJsonArray jSubsystemInfoArray = jConfig.value(QLatin1String("z_i_subsystemsInfo")).toArray();

		for (const QJsonValueRef jSubsystemInfoRef : jSubsystemInfoArray)
		{
			ModuleFirmware fw;

			QJsonObject jSubsystemInfo = jSubsystemInfoRef.toObject();

			if (jSubsystemInfo.value(QLatin1String("subsystemId")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field subsystemId";
				return false;
			}
			QString subsysId = jSubsystemInfo.value(QLatin1String("subsystemId")).toString();

			if (jSubsystemInfo.value(QLatin1String("subsystemKey")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field subsystemKey";
				return false;
			}
			int ssKey = jSubsystemInfo.value(QLatin1String("subsystemKey")).toInt();

			if (jSubsystemInfo.value(QLatin1String("lmDescriptionFile")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field lmDescriptionFile";
				return false;
			}
			QString lmDescriptionFile = jSubsystemInfo.value(QLatin1String("lmDescriptionFile")).toString();

			if (jSubsystemInfo.value(QLatin1String("lmDescriptionNumber")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field lmDescriptionNumber";
				return false;
			}
			int lmDescriptionNumber = jSubsystemInfo.value(QLatin1String("lmDescriptionNumber")).toInt();

			fw.init(subsysId, ssKey, lmDescriptionFile, lmDescriptionNumber);

			// Load modules information
			//

			QJsonArray jModuleInfoArray = jSubsystemInfo.value(QLatin1String("z_modules")).toArray();

			for (const QJsonValueRef jModuleInfoRef: jModuleInfoArray)
			{
				LogicModuleInfo lmi;

				QJsonObject jModuleInfo = jModuleInfoRef.toObject();

				if (jModuleInfo.value(QLatin1String("equipmentId")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field equipmentId";
					return false;
				}
				lmi.equipmentId = jModuleInfo.value(QLatin1String("equipmentId")).toString();

				if (jModuleInfo.value(QLatin1String("lmNumber")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field lmNumber";
					return false;
				}
				lmi.lmNumber = jModuleInfo.value(QLatin1String("lmNumber")).toInt();

				if (jModuleInfo.value(QLatin1String("channel")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field channel";
					return false;
				}
				lmi.channel = jModuleInfo.value(QLatin1String("channel")).toInt();

				if (jModuleInfo.value(QLatin1String("moduleFamily")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field moduleFamily";
					return false;
				}
				lmi.moduleFamily = jModuleInfo.value(QLatin1String("moduleFamily")).toInt();

				if (jModuleInfo.value(QLatin1String("customModuleFamily")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field customModuleFamily";
					return false;
				}
				lmi.customModuleFamily = jModuleInfo.value(QLatin1String("customModuleFamily")).toInt();

				if (jModuleInfo.value(QLatin1String("moduleVersion")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field moduleVersion";
					return false;
				}
				lmi.moduleVersion = jModuleInfo.value(QLatin1String("moduleVersion")).toInt();

				if (jModuleInfo.value(QLatin1String("moduleType")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field moduleType";
					return false;
				}
				lmi.moduleType = jModuleInfo.value(QLatin1String("moduleType")).toInt();

				fw.addLogicModuleInfo(lmi);

			}

			m_firmwares[fw.subsysId()] = fw;
		}


		// Load subsystems firmware data
		//

		QJsonArray jSubsystemDataArray = jConfig.value(QLatin1String("z_s_subsystemsData")).toArray();

		for (const QJsonValueRef jSubsystemDataRef : jSubsystemDataArray)
		{
			QJsonObject jSubsystemData = jSubsystemDataRef.toObject();

			if (jSubsystemData.value(QLatin1String("subsystemId")).isUndefined() == true)
			{
				*errorCode = "Parse firmware error: cant find field subsystemId";
				return false;
			}
			QString subsystemId = jSubsystemData.value(QLatin1String("subsystemId")).toString();

			bool ok = false;
			ModuleFirmware& fw = firmware(subsystemId, &ok);
			if (ok == false)
			{
				*errorCode = tr("Parse firmware error: unknown subsystem %1 in z_s_subsystemsData").arg(subsystemId);
				return false;
			}

			// Load firmware data
			//

			QJsonArray jFirmwareDataArray = jSubsystemData.value(QLatin1String("z_firmwareData")).toArray();

			for (const QJsonValueRef jFirmwareDataRef : jFirmwareDataArray)
			{
				QJsonObject jFirmwareData = jFirmwareDataRef.toObject();

				if (jFirmwareData.value(QLatin1String("eepromFrameCount")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field eepromFrameCount";
					return false;
				}
				int eepromFrameCount = jFirmwareData.value(QLatin1String("eepromFrameCount")).toInt();

				if (jFirmwareData.value(QLatin1String("eepromFramePayloadSize")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field eepromFramePayloadSize";
					return false;
				}
				int eepromFramePayloadSize = jFirmwareData.value(QLatin1String("eepromFramePayloadSize")).toInt();

				if (jFirmwareData.value(QLatin1String("eepromFrameSize")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field eepromFrameSize";
					return false;
				}
				//int eepromFrameSize = jFirmwareData.value(QLatin1String("eepromFrameSize")).toInt();

				if (jFirmwareData.value(QLatin1String("uartId")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field uartId";
					return false;
				}
				int uartId = jFirmwareData.value(QLatin1String("uartId")).toInt();

				if (jFirmwareData.value(QLatin1String("uartType")).isUndefined() == true)
				{
					*errorCode = "Parse firmware error: cant find field uartType";
					return false;
				}
				QString uartType = jFirmwareData.value(QLatin1String("uartType")).toString();

				fw.addFirmwareData(uartId, uartType, eepromFramePayloadSize, readBinaryData ? eepromFrameCount : 0);

				if (readBinaryData == true)
				{
					m_hasBinaryData = true;

					// Load data binary frames
					//

					ModuleFirmwareData& firmwareData = fw.firmwareData(uartId, &ok);
					if (ok == false)
					{
						assert(false);
						return false;
					}

					for (int v = 0; v < eepromFrameCount; v++)
					{
						QString zFrame = "z_frame_" + QString::number(v).rightJustified(4, '0');

						QJsonValue jFrameVal = jFirmwareData.value(zFrame);
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

						int dataWordPos = 0;

						quint16* frameWordPtr = (quint16*)firmwareData.frames[v].data();

						const int frameStringWidth = 16;
						const int linesCount = ceil((float)firmwareData.eepromFrameSize / 2 / frameStringWidth);

						for (int l = 0; l < linesCount; l++)
						{
							QJsonValue v;

							QString stringName = "data" + QString::number(l * frameStringWidth, 16).rightJustified(4, '0');
							v = jFrame.value(stringName);

							if (v.isUndefined() == true)
							{
								// data strings may be skipped
								//
								dataWordPos += frameStringWidth;
								continue;
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

								if (dataWordPos >= firmwareData.eepromFrameSize / sizeof(quint16))
								{
									assert(false);
									break;
								}

								frameWordPtr[dataWordPos++] = qToBigEndian(v);
							}
						} // linesCount

						if (Crc::checkDataBlockCrc(v, firmwareData.frames[v]) == false)
						{
							*errorCode = tr("File data is corrupt, CRC check error in frame %1.").arg(v);
							return false;
						}
					}
				} // eepromFrameCount
			} // jFirmwareDataRef
		}// jSubsystemDataRef

		return true;
	}



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
