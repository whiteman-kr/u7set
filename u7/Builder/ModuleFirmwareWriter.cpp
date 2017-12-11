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


	void ModuleFirmwareWriter::setDescriptionFields(int uartId, int descriptionVersion, const QStringList& fields)
	{
		if (m_channelData.find(uartId) == m_channelData.end())
		{
			m_channelData[uartId] = ModuleFirmwareChannelData();
		}

		ModuleFirmwareChannelData& data = m_channelData[uartId];

		data.descriptionFieldsVersion = descriptionVersion;
		data.descriptionFields = fields;
	}

	bool ModuleFirmwareWriter::setChannelData(int uartId, QString equipmentID, int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& binaryData, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (this->eepromFramePayloadSize(uartId) != frameSize)
		{
			log->errINT1000(QString("ModuleFirmware::setChannelData error, LM number %1: wrong frameSize (%2), expected %3.").arg(channel).arg(frameSize).arg(this->eepromFramePayloadSize(uartId)));
			return false;
		}

		if (this->eepromFrameCount(uartId) != frameCount)
		{
			log->errINT1000(QString("ModuleFirmware::setChannelData error, LM number %1: wrong frameCount (%2), expected %3.").arg(channel).arg(frameSize).arg(this->eepromFramePayloadSize(uartId)));
			return false;
		}

		if (m_channelData.find(uartId) == m_channelData.end())
		{
			m_channelData[uartId] = ModuleFirmwareChannelData();
		}

		ModuleFirmwareChannelData& data = m_channelData[uartId];

		if (data.binaryDataMap.find(channel) != data.binaryDataMap.end())
		{
			log->errCFG3003(channel, equipmentID);
			return false;
		}

		data.channelUniqueId[channel] = uniqueID;

		data.descriptonData[channel] = descriptionData;

		data.binaryDataMap[channel] = binaryData;

		return true;
	}

	bool ModuleFirmwareWriter::save(QByteArray& dest, Builder::IssueLogger* log)
	{

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (m_channelData.size() != 0)
		{
			if (storeChannelData(log) == false)
			{
				return false;
			}
		}

		QJsonObject jObject;

		int firmwareRecordIndex = 0;

		int firmwareCount = 0;

		for (auto it = m_firmwareData.begin(); it != m_firmwareData.end(); it++)
		{

			int	uartId = it->first;

			firmwareCount++;

			ModuleFirmwareData& data = it->second;

			// Count CRC64 for all frames

			for (int i = 0; i < eepromFrameCount(uartId); i++)
			{
				std::vector<quint8>& frame = data.frames[i];

				Crc::setDataBlockCrc(i, frame.data(), (int)frame.size());
			}

			// Save all frames to file

			QJsonObject jFirmware;

			const int frameStringWidth = 16;

			for (int i = 0; i < eepromFrameCount(uartId); i++)
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
			if (m_channelData.find(uartId) != m_channelData.end())
			{

				ModuleFirmwareChannelData& cd = m_channelData[uartId];

				if (cd.descriptionFields.empty() == false && cd.descriptionFields.empty() == false)
				{

					for (auto channelDescription : cd.descriptonData)
					{
						QJsonObject jDescription;

						int channel = channelDescription.first;
						const std::vector<QVariantList>& descriptionItems = channelDescription.second;

						// Description header string

						QString descriptionHeaderString = "Version;" + CsvFile::getCsvString(cd.descriptionFields, true);

						jDescription.insert("desc fields", descriptionHeaderString);

						// Description strings

						int diIndex = 0;
						for (auto di : descriptionItems)
						{
							if (di.size() != cd.descriptionFields.size())
							{
								qDebug() << "number of data items (" << di.size() << ") must be equal to number of header items (" << cd.descriptionFields.size() << ")";
								assert(false);
								return false;
							}

							QString descriptionString = QString("%1;%2").arg(cd.descriptionFieldsVersion).arg(CsvFile::getCsvString(di, true));

							jDescription.insert("desc" + QString::number(diIndex++).rightJustified(8, '0'), descriptionString);
						}

						if (descriptionItems.empty() == false)
						{
							jFirmware.insert("z_description_channel_" + QString::number(channel).rightJustified(2, '0'), jDescription);
						}
					}
				}

				jFirmware.insert("uartId", uartId);

				jFirmware.insert("uartType", data.uartType);

				jFirmware.insert("eepromFramePayloadSize", eepromFramePayloadSize(uartId));
				jFirmware.insert("eepromFrameSize", eepromFrameSize(uartId));
				jFirmware.insert("eepromFrameCount", eepromFrameCount(uartId));
			}


			jObject.insert("z_firmware_" + QString::number(firmwareRecordIndex++), jFirmware);
		}

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

		jObject.insert("userName", m_userName);
		jObject.insert("projectName", m_projectName);
		jObject.insert("caption", caption());
		jObject.insert("subsysId", subsysId());
		jObject.insert("buildNumber", m_buildNumber);
		jObject.insert("buildConfig", m_buildConfig);
		jObject.insert("lmDescriptionNumber", m_lmDescriptionNumber);
		jObject.insert("changesetId", m_changesetId);
		jObject.insert("fileVersion", fileVersion());
		jObject.insert("buildSoftware", m_buildSoftware);
		jObject.insert("buildTime", m_buildTime);
		jObject.insert("firmwaresCount", firmwareCount);

		dest = QJsonDocument(jObject).toJson();

		return true;
	}


	bool ModuleFirmwareWriter::setData8(int uartID, int frameIndex, int offset, quint8 data)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint8* ptr = static_cast<quint8*>(fd.frames[frameIndex].data() + offset);
		*ptr = data;

		return true;
	}

	bool ModuleFirmwareWriter::setData16(int uartID, int frameIndex, int offset, quint16 data)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint16 dataBE= qToBigEndian(data);

		quint16* ptr = reinterpret_cast<quint16*>(fd.frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	bool ModuleFirmwareWriter::setData32(int uartID, int frameIndex, int offset, quint32 data)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint32 dataBE = qToBigEndian(data);

		quint32* ptr = reinterpret_cast<quint32*>(fd.frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	bool ModuleFirmwareWriter::setData64(int uartID, int frameIndex, int offset, quint64 data)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint64 dataBE = qToBigEndian(data);

		quint64* ptr = reinterpret_cast<quint64*>(fd.frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	quint8 ModuleFirmwareWriter::data8(int uartID, int frameIndex, int offset)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(quint8)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return static_cast<quint8>(*(fd.frames[frameIndex].data() + offset));

	}

	quint16 ModuleFirmwareWriter::data16(int uartID, int frameIndex, int offset)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(quint16)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = fd.frames[frameIndex];

		quint16 data = *(reinterpret_cast<quint16*>(frameData.data() + offset));
		return qFromBigEndian(data);
	}

	quint32 ModuleFirmwareWriter::data32(int uartID, int frameIndex, int offset)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(quint32)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = fd.frames[frameIndex];

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

	QString ModuleFirmwareWriter::storeCrc64(int uartID, int frameIndex, int start, int count, int offset)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(quint64)) || start + count > eepromFramePayloadSize(uartID))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString();
		}

		quint64 result = Crc::crc64(fd.frames[frameIndex].data() + start, count);
		setData64(uartID, frameIndex, offset, result);

		return QString::number(result, 16);
	}

	QString ModuleFirmwareWriter::storeHash64(int uartID, int frameIndex, int offset, QString dataString)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				offset > (int)(eepromFramePayloadSize(uartID) - sizeof(quint64)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString("");
		}

		QByteArray bytes = dataString.toUtf8();

		quint64 result = CUtils::calcHash(bytes.data(), bytes.size());
		setData64(uartID, frameIndex, offset, result);

		return QString::number(result, 16);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

	}

	quint32 ModuleFirmwareWriter::calcCrc32(int uartID, int frameIndex, int start, int count)
	{
		if (m_firmwareData.find(uartID) == m_firmwareData.end())
		{
			assert(false);
			return false;
		}

		ModuleFirmwareData& fd = m_firmwareData[uartID];

		//

		if (frameIndex >= static_cast<int>(fd.frames.size()) ||
				start + count > eepromFramePayloadSize(uartID))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		quint64 result = Crc::crc64(fd.frames[frameIndex].data() + start, count);

		return result & 0xffffffff;
	}

	void ModuleFirmwareWriter::jsSetDescriptionFields(int uartID, int descriptionVersion, QString fields)
	{
		setDescriptionFields(uartID, descriptionVersion, fields.split(';'));
	}


	void ModuleFirmwareWriter::jsAddDescription(int uartID, int channel, QString descriptionCSV)
	{
		QStringList l = descriptionCSV.split(';');

		QVariantList v;
		for (auto s : l)
		{
			v.append(s);
		}

		if (m_channelData.find(uartID) == m_channelData.end())
		{
			assert(false);
			return;
		}

		ModuleFirmwareChannelData& cd = m_channelData[uartID];

		//

		auto dd = cd.descriptonData.find(channel);
		if (dd == cd.descriptonData.end())
		{
			std::vector<QVariantList> descriptonData;
			descriptonData.push_back(v);

			cd.descriptonData[channel] = descriptonData;

		}
		else
		{
			std::vector<QVariantList>& descriptonData = dd->second;
			descriptonData.push_back(v);
		}

	}

	void ModuleFirmwareWriter::jsSetUniqueID(int uartID, int lmNumber, quint64 uniqueID)
	{
		if (m_channelData.find(uartID) == m_channelData.end())
		{
			assert(false);
			return;
		}

		ModuleFirmwareChannelData& cd = m_channelData[uartID];

		cd.channelUniqueId[lmNumber] = uniqueID;
	}


	void ModuleFirmwareWriter::writeLog(QString logString)
	{
		m_scriptLog.append(logString);
	}

	const QByteArray& ModuleFirmwareWriter::scriptLog() const
	{
		return m_scriptLog;
	}

	quint64 ModuleFirmwareWriter::uniqueID(int uartId, int lmNumber) const
	{
		if (m_channelData.find(uartId) == m_channelData.end())
		{
			assert(false);
			return 0;
		}

		const ModuleFirmwareChannelData& data = m_channelData.at(uartId);

		auto it = data.channelUniqueId.find(lmNumber);
		if (it == data.channelUniqueId.end())
		{
			assert(false);
			return 0;
		}

		return it->second;
	}

	void ModuleFirmwareWriter::setGenericUniqueId(int lmNumber, quint64 genericUniqueId)
	{

		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
		{

			int uartId = it->first;

			ModuleFirmwareChannelData& channelData = it->second;

			if (channelData.binaryDataMap.empty() == true)
			{
				// This case is used for Configuration data (data is stored in frames, no binary data exists)

				if (m_firmwareData.find(uartId) == m_firmwareData.end())
				{
					assert(false);
					return;
				}

				ModuleFirmwareData& data = m_firmwareData[uartId];

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

				if (channelData.channelUniqueId.find(lmNumber) == channelData.channelUniqueId.end())
				{
					assert(false);
					return;
				}

				channelData.channelUniqueId[lmNumber] = genericUniqueId;
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

		quint16 ssKeyValue = m_ssKey << 6;

		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
		{
			int uartId = it->first;

			const ModuleFirmwareChannelData& channelData = it->second;

			if (channelData.binaryDataMap.empty() == true)
			{
				// no channel data supplied
				//
				continue;
			}

			if (eepromFrameCount(uartId) < 3)
			{
				log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, subsystem %1: At least 3 frames needed.").arg(subsysId()));
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

				double fSize = (double)data.size() / eepromFramePayloadSize(uartId);
				fSize = ceil(fSize);
				int size = (int)fSize;

				channelNumbersAndSize.push_back(std::make_pair(channel, size));
			}
			std::sort(channelNumbersAndSize.begin(), channelNumbersAndSize.end(), [](std::pair<int, int> a, std::pair<int, int> b)
			{
				return a.first < b.first;
			});

			// place channel data to frames
			//
			if (m_firmwareData.find(uartId) == m_firmwareData.end())
			{
				assert(false);
				return false;
			}

			ModuleFirmwareData& firmwareData = m_firmwareData[uartId];

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

				if (frame >= eepromFrameCount(uartId))
				{
					log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, SubsystemID %1, LM number %2: data is too big. frame = %3, frameCount = %4").arg(subsysId()).arg(channel).arg(frame).arg(eepromFrameCount(uartId)));
					return false;
				}

				channelStartFrame.push_back(frame);

				// channel data
				//
				const QByteArray& binaryData = channelData.binaryDataMap.at(channel);

				quint64 uniqueId = channelData.channelUniqueId.at(channel);

				// channel service information
				//

				quint8* ptr = firmwareData.frames[frame].data();

				*(quint16*)ptr = qToBigEndian((quint16)0x0001);		//Channel configuration version
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)uartId);	//Data type (configuration)
				ptr += sizeof(quint16);

				if (uniqueId == 0)
				{
					QByteArray bytes = subsysId().toUtf8();

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
						if (index >= eepromFramePayloadSize(uartId))
						{
							// data is bigger than frame - switch to the next frame
							//
							frame++;
							index = 0;
						}

						if (frame >= eepromFrameCount(uartId))
						{
							log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, SubsystemID %1, LM number %2: data is too big. frame = %3, frameCount = %4").arg(subsysId()).arg(channel).arg(frame).arg(eepromFrameCount(uartId)));
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

			*(quint16*)ptr = qToBigEndian((quint16)m_lmDescriptionNumber);	// Description number
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

		return true;
	}

	//
	//
	// ModuleFirmwareCollection
	//
	//

	ModuleFirmwareCollection::ModuleFirmwareCollection():
		m_buildNo(0),
		m_debug(false),
		m_changesetId(0)
	{
	}

	ModuleFirmwareCollection::~ModuleFirmwareCollection()
	{
	}

	void ModuleFirmwareCollection::init(const QString& projectName, const QString& userName, int buildNo, bool debug, int changesetId)
	{
		m_projectName = projectName;
		m_userName = userName;
		m_buildNo = buildNo;
		m_debug = debug;
		m_changesetId = changesetId;
	}

	ModuleFirmwareWriter* ModuleFirmwareCollection::createFirmware(QString caption, QString subsysId, int ssKey, int uartId, QString uartType, int frameSize, int frameCount, int lmDescriptionNumber)
	{
		bool newFirmware = m_firmwares.count(subsysId) == 0;

		ModuleFirmwareWriter& fw = m_firmwares[subsysId];

		if (newFirmware == true || fw.uartExists(uartId) == false)
		{
			fw.init(uartId, uartType, frameSize, frameCount, caption, subsysId, ssKey, lmDescriptionNumber, m_projectName, m_userName,
					m_buildNo, m_debug ? "debug" : "release", m_changesetId);
		}

		return &fw;
	}

	std::map<QString, ModuleFirmwareWriter>& ModuleFirmwareCollection::firmwares()
	{
		return m_firmwares;
	}

	ModuleFirmwareWriter& ModuleFirmwareCollection::firmware(const QString& subsystemID)
	{
static ModuleFirmwareWriter err;

		if (m_firmwares.find(subsystemID) == m_firmwares.end())
		{
			return err;
		}

		return m_firmwares.at(subsystemID);
	}


}
