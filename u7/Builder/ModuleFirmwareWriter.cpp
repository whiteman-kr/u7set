#include "../Builder/ModuleFirmwareWriter.h"
#include "../lib/CsvFile.h"
#include "../lib/Crc.h"
#include <QtEndian>
#include "version.h"

//
// JsVariantList
//

JsVariantList::JsVariantList(QObject* parent):
	QObject(parent)
{

}

void JsVariantList::append(QVariant v)
{
	m_list.append(v);
}

int JsVariantList::jsSize()
{
	return m_list.size();
}

QVariant JsVariantList::jsAt(int i)
{
	return m_list.at(i);
}


namespace Hardware
{

	//
	// ModuleFirmwareWriter
	//

	ModuleFirmwareWriter::ModuleFirmwareWriter()
	{
	}


	void ModuleFirmwareWriter::setDescriptionFields(const QString& subsysId, int uartId, int descriptionVersion, const QStringList& fields)
	{
		ModuleChannelData& subsystemChannelData = m_moduleChannelData[subsysId];

		UartChannelData& data = subsystemChannelData.channelData[uartId];

		data.descriptionFieldsVersion = descriptionVersion;
		data.descriptionFields = fields;
	}

	bool ModuleFirmwareWriter::setChannelData(const QString& subsysId, int uartId, QString equipmentID, int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& binaryData, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		// Check for correct firmware parameters

		bool ok = false;

		const ModuleFirmware& sf = moduleFirmware(subsysId, &ok);

		if (ok == false)
		{
			log->errINT1000(QString("FirmwareWriter::setChannelData error, subsystem %1 does not exist.").arg(subsysId));
			return false;
		}

		if (sf.eepromFramePayloadSize(uartId) != frameSize)
		{
			log->errINT1000(QString("FirmwareWriter::setChannelData error, LM number %1: wrong frameSize (%2), expected %3.").arg(channel).arg(frameSize).arg(sf.eepromFramePayloadSize(uartId)));
			return false;
		}

		if (sf.eepromFrameCount(uartId) != frameCount)
		{
			log->errINT1000(QString("FirmwareWriter::setChannelData error, LM number %1: wrong frameCount (%2), expected %3.").arg(channel).arg(frameSize).arg(sf.eepromFramePayloadSize(uartId)));
			return false;
		}

		// Set the data for channel

		ModuleChannelData& subsystemChannelData = m_moduleChannelData[subsysId];

		UartChannelData& moduleChannelData = subsystemChannelData.channelData[uartId];

		// Check if data for this channel already exists

		if (moduleChannelData.binaryDataMap.find(channel) != moduleChannelData.binaryDataMap.end())
		{
			log->errCFG3003(channel, equipmentID);
			return false;
		}

		moduleChannelData.uniqueIdMap[channel] = uniqueID;

		moduleChannelData.descriptonDataMap[channel] = descriptionData;

		moduleChannelData.binaryDataMap[channel] = binaryData;

		return true;
	}

	bool ModuleFirmwareWriter::save(QByteArray& dest, Builder::IssueLogger* log)
	{

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (storeChannelData(log) == false)
		{
			return false;
		}

		QJsonObject jObject;

		int subsystemIndex = 0;

		QJsonArray jSubsystemArray;

		for (auto fwi : m_firmwares)
		{
			const QString& subsystemId = fwi.first;

			ModuleFirmware& fw = fwi.second;

			ModuleChannelData& moduleChannelData = m_moduleChannelData[subsystemId];

			QJsonObject jSubsystem;

			std::vector<UartPair> uarts = fw.uartList();

			QJsonArray jFirmwaresArray;

			for (auto it : uarts)
			{
				int	uartId = it.first;

				bool ok = false;
				ModuleFirmwareData& data = fw.firmwareData(uartId, &ok);
				if (ok == false)
				{
					assert(false);
					continue;
				}

				UartChannelData& uartChannelData = moduleChannelData.channelData[uartId];

				// Count CRC64 for all frames

				int framesCount = static_cast<int>(data.frames.size());

				for (int i = 0; i < framesCount; i++)
				{
					std::vector<quint8>& frame = data.frames[i];

					Crc::setDataBlockCrc(i, frame.data(), (int)frame.size());
				}

				// Save all frames to file

				QJsonObject jFirmware;

				const int frameStringWidth = 16;

				for (int i = 0; i < framesCount; i++)
				{
					const std::vector<quint8>& frame = data.frames[i];

					QJsonObject jFrame;

					int dataSize = (int)frame.size();
					int linesCount = ceil((float)dataSize / 2 / frameStringWidth);

					int dataPos = 0;

					const int numCharsCount = 4;					// number of symbols in number "0000" (4)
					const int recCharsCount = numCharsCount + 1;	// number of symbols in number "0000 " (with space)

					QByteArray str;
					str.resize(recCharsCount * frameStringWidth - 1);

					char buf[10];

					for (int l = 0; l < linesCount; l++)
					{
						str.fill(' ');

						for (int i = 0; i < frameStringWidth; i++)
						{
							quint16 value = ((quint16)frame[dataPos++] << 8);
							if (dataPos >= dataSize)
							{
								assert(false);
								break;
							}

							value |= frame[dataPos++];

							snprintf(buf, sizeof(buf), "%hx", value);

							int len = static_cast<int>(strlen(buf));

							memset(str.data() + i * recCharsCount, '0', numCharsCount);
							memcpy(str.data() + i * recCharsCount + (numCharsCount - len), buf, len);

							if (dataPos >= dataSize)
							{
								str[i * recCharsCount + numCharsCount] = 0;
								break;
							}
						}

						jFrame.insert("data" + QString().number(l * frameStringWidth, 16).rightJustified(4, '0'), QJsonValue(str.data()));
					}
					jFrame.insert("frameIndex", i);

					jFirmware.insert("z_frame_" + QString().number(i).rightJustified(4, '0'), jFrame);
				}

				//description
				//
				if (uartChannelData.descriptionFields.empty() == false)
				{
					for (auto channelDescription : uartChannelData.descriptonDataMap)
					{
						QJsonObject jDescription;

						int channel = channelDescription.first;
						const std::vector<QVariantList>& descriptionItems = channelDescription.second;

						// Description header string

						QString descriptionHeaderString = "Version;" + CsvFile::getCsvString(uartChannelData.descriptionFields, true);

						jDescription.insert("desc fields", descriptionHeaderString);

						// Description strings

						int diIndex = 0;
						for (auto di : descriptionItems)
						{
							if (di.size() != uartChannelData.descriptionFields.size())
							{
								qDebug() << "number of data items (" << di.size() << ") must be equal to number of header items (" << uartChannelData.descriptionFields.size() << ")";
								assert(false);
								return false;
							}

							QString descriptionString = QString("%1;%2").arg(uartChannelData.descriptionFieldsVersion).arg(CsvFile::getCsvString(di, true));

							jDescription.insert("desc" + QString::number(diIndex++).rightJustified(8, '0'), descriptionString);
						}

						if (descriptionItems.empty() == false)
						{
							jFirmware.insert("z_description_channel_" + QString::number(channel).rightJustified(2, '0'), jDescription);
						}
					}

					jFirmware.insert("uartId", uartId);

					jFirmware.insert("uartType", data.uartType);

					jFirmware.insert("eepromFramePayloadSize", data.frameSize);
					jFirmware.insert("eepromFrameSize", data.frameSizeWithCRC);
					jFirmware.insert("eepromFrameCount", static_cast<int>(data.frames.size()));

					jFirmwaresArray.push_back(jFirmware);
				}


				jSubsystem.insert("z_firmwares", jFirmwaresArray);

				jSubsystem.insert("caption", fw.caption());
				jSubsystem.insert("lmDescriptionNumber", fw.lmDescriptionNumber());
				jSubsystem.insert("subsystemId", subsystemId);
			}

			jSubsystemArray.push_back(jSubsystem);
		}

		jObject.insert("z_subsystems", jSubsystemArray);

		// properties
		//

		QString m_buildSoftware = qApp->applicationName() +" v" + qApp->applicationVersion();
#ifndef Q_DEBUG
		m_buildSoftware += ", release";
#else
		m_buildSoftware += ", debug";
#endif
		m_buildSoftware += ", commit SHA1: " USED_SERVER_COMMIT_SHA;

		QString m_buildTime = QDateTime().currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

		QStringList subsystemList = subsystemsList();
		QString subsystems;
		for (const QString& s : subsystemList)
		{
			subsystems.push_back(s + " ");
		}

		jObject.insert("subsystems", subsystems.trimmed());
		jObject.insert("userName", m_userName);
		jObject.insert("projectName", m_projectName);
		jObject.insert("buildNumber", m_buildNumber);
		jObject.insert("buildConfig", m_debug ? "debug" : "release");
		jObject.insert("changesetId", m_changesetId);
		jObject.insert("fileVersion", fileVersion());
		jObject.insert("buildSoftware", m_buildSoftware);
		jObject.insert("buildTime", m_buildTime);

		dest = QJsonDocument(jObject).toJson();

		return true;
	}

	void ModuleFirmwareWriter::setScriptFirmware(QString subsysId, int uartID)
	{
		scriptFirmware = nullptr;
		scriptFirmwareData = nullptr;

		bool ok = false;
		ModuleFirmware& fw = moduleFirmware(subsysId, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		ModuleFirmwareData& fd = fw.firmwareData(uartID, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		ModuleChannelData& cd = m_moduleChannelData[subsysId];

		scriptFirmware = &fw;
		scriptFirmwareData = &fd;
		scriptUartChannelData = &cd.channelData[uartID];
	}

	bool ModuleFirmwareWriter::setData8(int frameIndex, int offset, quint8 data)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint8* ptr = static_cast<quint8*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = data;

		return true;
	}

	bool ModuleFirmwareWriter::setData16(int frameIndex, int offset, quint16 data)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint16 dataBE= qToBigEndian(data);

		quint16* ptr = reinterpret_cast<quint16*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	bool ModuleFirmwareWriter::setData32(int frameIndex, int offset, quint32 data)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint32 dataBE = qToBigEndian(data);

		quint32* ptr = reinterpret_cast<quint32*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	bool ModuleFirmwareWriter::setData64(int frameIndex, int offset, quint64 data)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmware);
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint64 dataBE = qToBigEndian(data);

		quint64* ptr = reinterpret_cast<quint64*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	quint8 ModuleFirmwareWriter::data8(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(quint8)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return static_cast<quint8>(*(scriptFirmwareData->frames[frameIndex].data() + offset));

	}

	quint16 ModuleFirmwareWriter::data16(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(quint16)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = scriptFirmwareData->frames[frameIndex];

		quint16 data = *(reinterpret_cast<quint16*>(frameData.data() + offset));
		return qFromBigEndian(data);
	}

	quint32 ModuleFirmwareWriter::data32(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(quint32)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = scriptFirmwareData->frames[frameIndex];

		quint32 data = *(reinterpret_cast<quint32*>(frameData.data() + offset));
		return qFromBigEndian(data);
	}

	JsVariantList* ModuleFirmwareWriter::calcHash64(QString dataString)
	{

		QByteArray bytes = dataString.toUtf8();

		quint64 result = CUtils::calcHash(bytes.data(), bytes.size());

		quint32 h = (result >> 32) & 0xffffffff;
		quint32 l = result & 0xffffffff;

		JsVariantList* vl = new JsVariantList(this);
		vl->append(QVariant(h));
		vl->append(QVariant(l));
		return vl;
	}

	QString ModuleFirmwareWriter::storeCrc64(int frameIndex, int start, int count, int offset)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(quint64)) || start + count > scriptFirmwareData->frameSize)
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString();
		}

		quint64 result = Crc::crc64(scriptFirmwareData->frames[frameIndex].data() + start, count);
		setData64(frameIndex, offset, result);

		return QString::number(result, 16);
	}

	QString ModuleFirmwareWriter::storeHash64(int frameIndex, int offset, QString dataString)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->frameSize - sizeof(quint64)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString("");
		}

		QByteArray bytes = dataString.toUtf8();

		quint64 result = CUtils::calcHash(bytes.data(), bytes.size());
		setData64(frameIndex, offset, result);

		return QString::number(result, 16);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

	}

	quint32 ModuleFirmwareWriter::calcCrc32(int frameIndex, int start, int count)
	{
		if (scriptFirmwareData == nullptr)
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				start + count > scriptFirmwareData->frameSize)
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		quint64 result = Crc::crc64(scriptFirmwareData->frames[frameIndex].data() + start, count);

		return result & 0xffffffff;
	}

	void ModuleFirmwareWriter::jsSetDescriptionFields(int descriptionVersion, QString fields)
	{
		if (scriptFirmware == nullptr || scriptFirmwareData == nullptr)
		{
			assert(scriptFirmware);
			assert(scriptFirmwareData);
			return;
		}

		setDescriptionFields(scriptFirmware->subsysId(), scriptFirmwareData->uartId , descriptionVersion, fields.split(';'));
	}


	void ModuleFirmwareWriter::jsAddDescription(int channel, QString descriptionCSV)
	{
		if (scriptUartChannelData == nullptr)
		{
			assert(scriptUartChannelData);
			return;
		}

		QStringList l = descriptionCSV.split(';');

		QVariantList v;
		for (auto s : l)
		{
			v.append(s);
		}

		//

		auto dd = scriptUartChannelData->descriptonDataMap.find(channel);
		if (dd == scriptUartChannelData->descriptonDataMap.end())
		{
			std::vector<QVariantList> descriptonData;
			descriptonData.push_back(v);

			scriptUartChannelData->descriptonDataMap[channel] = descriptonData;

		}
		else
		{
			std::vector<QVariantList>& descriptonData = dd->second;
			descriptonData.push_back(v);
		}

	}

	void ModuleFirmwareWriter::jsSetUniqueID(int lmNumber, quint64 uniqueID)
	{
		if (scriptUartChannelData == nullptr)
		{
			assert(scriptUartChannelData);
			return;
		}

		scriptUartChannelData->uniqueIdMap[lmNumber] = uniqueID;

	}


	void ModuleFirmwareWriter::writeLog(QString logString)
	{
		QByteArray& log = m_scriptLog[scriptFirmware->subsysId()];

		log.append(logString);
	}

	const QByteArray& ModuleFirmwareWriter::scriptLog(const QString& subsysId) const
	{
static QByteArray err;
		if (m_scriptLog.find(subsysId) == m_scriptLog.end())
		{
			return err;
		}
		return m_scriptLog.at(subsysId);
	}

	quint64 ModuleFirmwareWriter::uniqueID(const QString& subsysId, int uartId, int lmNumber) const
	{
		const ModuleChannelData& subsystemChannelData = m_moduleChannelData.at(subsysId);

		const UartChannelData& data = subsystemChannelData.channelData.at(uartId);

		auto it = data.uniqueIdMap.find(lmNumber);
		if (it == data.uniqueIdMap.end())
		{
			assert(false);
			return 0;
		}

		return it->second;
	}

	void ModuleFirmwareWriter::setGenericUniqueId(const QString& subsysId, int lmNumber, quint64 genericUniqueId)
	{
		bool ok = false;

		ModuleFirmware& sf = moduleFirmware(subsysId, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		ModuleChannelData& subsystemChannelData = m_moduleChannelData[subsysId];

		for (auto it = subsystemChannelData.channelData.begin(); it != subsystemChannelData.channelData.end(); it++)
		{
			int uartId = it->first;

			UartChannelData& channelData = it->second;

			if (channelData.binaryDataMap.empty() == true)
			{
				// This case is used for Configuration data (data is stored in frames, no binary data exists)

				if (sf.uartExists(uartId) == false)
				{
					assert(false);
					return;
				}

				ModuleFirmwareData& data = sf.firmwareData(uartId, &ok);
				if (ok == false)
				{
					assert(false);
					return;
				}

				const int ConfigDataStartFrame = 2;

				const int LMNumberConfigFrameCount = 19;

				int frameNumber = ConfigDataStartFrame + LMNumberConfigFrameCount * (lmNumber - 1);

				const int UniqueIdOffset = 4;

				quint64 uidBE = qToBigEndian(genericUniqueId);

				if (frameNumber < 0 || frameNumber >= (int)data.frames.size())
				{
					assert(false);
					return;
				}

				std::vector<quint8>& frame = data.frames[frameNumber];

				quint8* pData = frame.data();

				quint64* pUniqueIdPtr = (quint64*)(pData + UniqueIdOffset);

				*pUniqueIdPtr = uidBE;
			}
			else
			{
				// This case is used for Application and Tuning data (data is stored in Channel Data)

				if (channelData.uniqueIdMap.find(lmNumber) == channelData.uniqueIdMap.end())
				{
					assert(false);
					return;
				}

				channelData.uniqueIdMap[lmNumber] = genericUniqueId;
			}
		}
	}

	bool ModuleFirmwareWriter::storeChannelData(Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		const int storageConfigFrame = 1;
		const int startDataFrame = 2;

		for (auto it : m_firmwares)
		{
			ModuleFirmware& fw = it.second;

			quint16 ssKeyValue = fw.ssKey() << 6;

			ModuleChannelData& cd = m_moduleChannelData[fw.subsysId()];

			for (auto it = cd.channelData.begin(); it != cd.channelData.end(); it++)
			{
				// Check if channel data exists for current uartId

				const UartChannelData& channelData = it->second;

				if (channelData.binaryDataMap.empty() == true)
				{
					// no channel data supplied
					//
					continue;
				}

				int uartId = it->first;

				if (fw.eepromFrameCount(uartId) < 3)
				{
					log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, subsystem %1: At least 3 frames needed.").arg(fw.subsysId()));
					return false;
				}

				// sort channel data by growing channel number
				//
				const int LMNumber_Min = 1;
				const int LMNumber_Max = 64;

				std::vector<std::pair<int, int>> channelNumbersAndSize;
				for (auto it = channelData.binaryDataMap.begin(); it != channelData.binaryDataMap.end(); it++)
				{
					int channel = it->first;

					if (channel < LMNumber_Min || channel > LMNumber_Max)
					{
						log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, LM number %1: Wrong LM number, expected %2..%3.").arg(channel).arg(LMNumber_Min).arg(LMNumber_Max));
						return false;
					}

					const QByteArray& data = it->second;

					double fSize = (double)data.size() / fw.eepromFramePayloadSize(uartId);
					fSize = ceil(fSize);
					int size = (int)fSize;

					channelNumbersAndSize.push_back(std::make_pair(channel, size));
				}
				std::sort(channelNumbersAndSize.begin(), channelNumbersAndSize.end(), [](std::pair<int, int> a, std::pair<int, int> b)
				{
					return a.first < b.first;
				});

				// place channel data to frames

				bool ok = false;

				ModuleFirmwareData& firmwareData = fw.firmwareData(uartId, &ok);
				if (ok == false)
				{
					assert(false);
					return false;
				}

				std::vector<int> channelStartFrame;

				int frame = startDataFrame;
				int maxChannel = 1;

				for (size_t c = 0; c < channelNumbersAndSize.size(); c++)
				{
					int channel = channelNumbersAndSize[c].first;
					if (channel > maxChannel)
					{
						maxChannel = channel;
					}
					int size = (quint16)channelNumbersAndSize[c].second;

					if (frame >= fw.eepromFrameCount(uartId))
					{
						log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, SubsystemID %1, LM number %2: data is too big. frame = %3, frameCount = %4").arg(fw.subsysId()).arg(channel).arg(frame).arg(fw.eepromFrameCount(uartId)));
						return false;
					}

					channelStartFrame.push_back(frame);

					// channel data
					//
					const QByteArray& binaryData = channelData.binaryDataMap.at(channel);

					quint64 uniqueId = channelData.uniqueIdMap.at(channel);

					// channel service information
					//

					quint8* ptr = firmwareData.frames[frame].data();

					*(quint16*)ptr = qToBigEndian((quint16)0x0001);		//Channel configuration version
					ptr += sizeof(quint16);

					*(quint16*)ptr = qToBigEndian((quint16)uartId);	//Data type (configuration)
					ptr += sizeof(quint16);

					if (uniqueId == 0)
					{
						QByteArray bytes = fw.subsysId().toUtf8();

						*(quint64*)ptr = qToBigEndian(CUtils::calcHash(bytes.data(), bytes.size()));
						ptr += sizeof(quint64);
					}
					else
					{
						*(quint64*)ptr = qToBigEndian(uniqueId);
						ptr += sizeof(quint64);
					}

					*(quint16*)ptr = qToBigEndian((quint16)size);           // Frames count
					ptr += sizeof(quint16);

					frame++;

					if (size != 0)
					{
						// store channel data in frames
						//
						int index = 0;
						for (int i = 0; i < binaryData.size(); i++)
						{
							if (index >= fw.eepromFramePayloadSize(uartId))
							{
								// data is bigger than frame - switch to the next frame
								//
								frame++;
								index = 0;
							}

							if (frame >= fw.eepromFrameCount(uartId))
							{
								log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, SubsystemID %1, LM number %2: data is too big. frame = %3, frameCount = %4").arg(fw.subsysId()).arg(channel).arg(frame).arg(fw.eepromFrameCount(uartId)));
								return false;
							}

							firmwareData.frames[frame][index++] = binaryData[i];
						}

						//switch to the next frame
						//
						frame++;
					}
				}


				// fill storage config frame
				//
				quint8* ptr = firmwareData.frames[storageConfigFrame].data();

				*(quint16*)ptr = qToBigEndian((quint16)0xCA70);	// Configuration reference mark
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)0x0001);	// Configuration structure version
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)ssKeyValue);	// Subsystem key
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)m_buildNumber);	// Build number
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)fw.lmDescriptionNumber());	// Description number
				ptr += sizeof(quint16);	//reserved

				ptr += sizeof(quint32);	//reserved

				*(quint16*)ptr = qToBigEndian((quint16)maxChannel);	// Configuration channels quantity
				ptr += sizeof(quint16);

				if (channelNumbersAndSize.size() != channelStartFrame.size())
				{
					assert(channelNumbersAndSize.size() == channelStartFrame.size());
					return false;
				}

				quint8* ptrChannelTable = ptr;

				for (size_t i = 0; i < channelNumbersAndSize.size(); i++)	// Start frames
				{
					int channel = channelNumbersAndSize[i].first;

					quint8* ptrChannel = ptrChannelTable + (sizeof(quint16) * 3) *(channel - 1);

					quint16 startFrame = (quint16)channelStartFrame[i];

					*(quint16*)ptrChannel = qToBigEndian(startFrame);
					ptrChannel += sizeof(quint16);

					ptrChannel += sizeof(quint32);
				}
			}
		}

		return true;
	}
}
