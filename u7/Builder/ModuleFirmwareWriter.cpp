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


	void ModuleFirmwareWriter::setDescriptionFields(int descriptionVersion, const QStringList& fields)
	{
		m_descriptionFieldsVersion = descriptionVersion;
		m_descriptionFields = fields;
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

		// Count CRC64 for all frames

		for (int i = 0; i < frameCount(); i++)
		{
			std::vector<quint8>& frame = m_frames[i];

			Crc::setDataBlockCrc(i, frame.data(), (int)frame.size());
		}

		// Save all frames to file

		const int frameStringWidth = 16;

		QJsonObject jObject;

		for (int i = 0; i < frameCount(); i++)
		{
			const std::vector<quint8>& frame = m_frames[i];

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


			jObject.insert("z_frame_" + QString().number(i).rightJustified(4, '0'), jFrame);
		}

		//description
		//

		if (m_descriptionFields.empty() == false && m_descriptonData.empty() == false)
		{

			for (auto channelDescription : m_descriptonData)
			{
				QJsonObject jDescription;

				int channel = channelDescription.first;
				const std::vector<QVariantList>& descriptionItems = channelDescription.second;

				// Description header string

				QString descriptionHeaderString = "Version;" + CsvFile::getCsvString(m_descriptionFields, true);

				jDescription.insert("desc fields", descriptionHeaderString);

				// Description strings

				int diIndex = 0;
				for (auto di : descriptionItems)
				{
					if (di.size() != m_descriptionFields.size())
					{
						qDebug() << "number of data items (" << di.size() << ") must be equal to number of header items (" << m_descriptionFields.size() << ")";
						assert(false);
						return false;
					}

					QString descriptionString = QString("%1;%2").arg(m_descriptionFieldsVersion).arg(CsvFile::getCsvString(di, true));

					jDescription.insert("desc" + QString::number(diIndex++).rightJustified(8, '0'), descriptionString);
				}

				if (descriptionItems.empty() == false)
				{
					jObject.insert("z_description_channel_" + QString::number(channel).rightJustified(2, '0'), jDescription);
				}
			}
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
		jObject.insert("uartId", uartId());
		jObject.insert("frameSize", frameSize());
		jObject.insert("frameSizeWithCRC", frameSizeWithCRC());
		jObject.insert("framesCount", frameCount());
		jObject.insert("buildNumber", m_buildNumber);
		jObject.insert("buildConfig", m_buildConfig);
		jObject.insert("changesetId", m_changesetId);
		jObject.insert("fileVersion", fileVersion());
		jObject.insert("buildSoftware", m_buildSoftware);
		jObject.insert("buildTime", m_buildTime);

		dest = QJsonDocument(jObject).toJson();

		return true;
	}

	bool ModuleFirmwareWriter::setChannelData(QString equipmentID, int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& data, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (this->frameSize() != frameSize)
		{
			log->errINT1000(QString("ModuleFirmware::setChannelData error, LM number %1: wrong frameSize (%2), expected %3.").arg(channel).arg(frameSize).arg(this->frameSize()));
			return false;
		}

		if (this->frameCount() != frameCount)
		{
			log->errINT1000(QString("ModuleFirmware::setChannelData error, LM number %1: wrong frameCount (%2), expected %3.").arg(channel).arg(frameSize).arg(this->frameSize()));
			return false;
		}

		auto it = m_channelData.find(channel);
		if (it != m_channelData.end())
		{
			log->errCFG3003(channel, equipmentID);
			return false;
		}

		m_channelUniqueId[channel] = uniqueID;

		m_descriptonData[channel] = descriptionData;

		m_channelData[channel] = data;

		return true;
	}

	bool ModuleFirmwareWriter::setData8(int frameIndex, int offset, quint8 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint8* ptr = static_cast<quint8*>(m_frames[frameIndex].data() + offset);
		*ptr = data;

		return true;
	}

	bool ModuleFirmwareWriter::setData16(int frameIndex, int offset, quint16 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint16 dataBE= qToBigEndian(data);

		quint16* ptr = reinterpret_cast<quint16*>(m_frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	bool ModuleFirmwareWriter::setData32(int frameIndex, int offset, quint32 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint32 dataBE = qToBigEndian(data);

		quint32* ptr = reinterpret_cast<quint32*>(m_frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	bool ModuleFirmwareWriter::setData64(int frameIndex, int offset, quint64 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint64 dataBE = qToBigEndian(data);

		quint64* ptr = reinterpret_cast<quint64*>(m_frames[frameIndex].data() + offset);
		*ptr = dataBE;

		return true;
	}

	quint8 ModuleFirmwareWriter::data8(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(quint8)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return static_cast<quint8>(*(m_frames[frameIndex].data() + offset));

	}

	quint16 ModuleFirmwareWriter::data16(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(quint16)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = m_frames[frameIndex];

		quint16 data = *(reinterpret_cast<quint16*>(frameData.data() + offset));
		return qFromBigEndian(data);
	}

	quint32 ModuleFirmwareWriter::data32(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(quint32)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = m_frames[frameIndex];

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
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(quint64)) || start + count > frameSize())
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString();
		}

		quint64 result = Crc::crc64(m_frames[frameIndex].data() + start, count);
		setData64(frameIndex, offset, result);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

		return QString::number(result, 16);
	}

	QString ModuleFirmwareWriter::storeHash64(int frameIndex, int offset, QString dataString)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				offset > (int)(frameSize() - sizeof(quint64)))
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
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
				start + count > frameSize())
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		quint64 result = Crc::crc64(m_frames[frameIndex].data() + start, count);

		return result & 0xffffffff;
	}

	void ModuleFirmwareWriter::jsSetDescriptionFields(int descriptionVersion, QString fields)
	{
		m_descriptionFieldsVersion = descriptionVersion;
		m_descriptionFields = fields.split(';');
	}

	void ModuleFirmwareWriter::jsAddDescription(int channel, QString descriptionCSV)
	{
		QStringList l = descriptionCSV.split(';');

		QVariantList v;
		for (auto s : l)
		{
			v.append(s);
		}

		auto cd = m_descriptonData.find(channel);
		if (cd == m_descriptonData.end())
		{
			std::vector<QVariantList> descriptonData;
			descriptonData.push_back(v);

			m_descriptonData[channel] = descriptonData;

		}
		else
		{
			std::vector<QVariantList>& descriptonData = cd->second;
			descriptonData.push_back(v);
		}

	}

	void ModuleFirmwareWriter::jsSetUniqueID(int lmNumber, quint64 uniqueID)
	{
		m_channelUniqueId[lmNumber] = uniqueID;
	}


	void ModuleFirmwareWriter::writeLog(QString logString)
	{
		m_scriptLog.append(logString);
	}

	const QByteArray& ModuleFirmwareWriter::scriptLog() const
	{
		return m_scriptLog;
	}

	quint64 ModuleFirmwareWriter::uniqueID(int lmNumber)
	{
		auto it = m_channelUniqueId.find(lmNumber);
		if (it == m_channelUniqueId.end())
		{
			assert(false);
			return 0;
		}

		return it->second;
	}

	void ModuleFirmwareWriter::setGenericUniqueId(int lmNumber, quint64 genericUniqueId)
	{
		if (m_channelData.empty() == false)
		{
			// This case is used for Application and Tuning data (data is stored in Channel Data)

			if (m_channelData.find(lmNumber) == m_channelData.end())
			{
				assert(false);
				return;
			}

			m_channelUniqueId[lmNumber] = genericUniqueId;
		}
		else
		{
			// This case is used for Configuration data (data is stored in frames)

			const int ConfigDataStartFrame = 2;

			const int LMNumberConfigFrameCount = 19;

			int frameNumber = ConfigDataStartFrame + LMNumberConfigFrameCount * (lmNumber - 1);

			const int UniqueIdOffset = 4;

			quint64 uidBE = qToBigEndian(genericUniqueId);

			if (frameNumber < 0 || frameNumber >= (int)m_frames.size())
			{
				assert(false);
				return;
			}

			std::vector<quint8>& frame = m_frames[frameNumber];

			quint8* pData = frame.data();

			quint64* pUniqueIdPtr = (quint64*)(pData + UniqueIdOffset);

			*pUniqueIdPtr = uidBE;
		}
	}

	bool ModuleFirmwareWriter::storeChannelData(Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (m_channelData.size() == 0)
		{
			// no channel data supplied
			//
			return true;
		}

		const int storageConfigFrame = 1;
		const int startDataFrame = 2;

		quint16 ssKeyValue = m_ssKey << 6;

		if (frameCount() < 3)
		{
			log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, subsystem %1: At least 3 frames needed.").arg(subsysId()));
			return false;
		}

		const int LMNumber_Min = 1;
		const int LMNumber_Max = 64;

		// sort channel data by growing channel number
		//
		std::vector<std::pair<int, int>> channelNumbersAndSize;
		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
		{
			int channel = it->first;

			if (channel < LMNumber_Min || channel > LMNumber_Max)
			{
				log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, LM number %1: Wrong LM number, expected %2..%3.").arg(channel).arg(LMNumber_Min).arg(LMNumber_Max));
				return false;
			}

			const QByteArray& channelData = it->second;

			double fSize = (double)channelData.size() / frameSize();
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

			if (frame >= frameCount())
			{
				log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, SubsystemID %1, LM number %2: data is too big. frame = %3, frameCount = %4").arg(subsysId()).arg(channel).arg(frame).arg(frameCount()));
				return false;
			}

			channelStartFrame.push_back(frame);

			// channel data
			//
			const QByteArray& channelData = m_channelData[channel];

			quint64 uniqueId = m_channelUniqueId[channel];

			// channel service information
			//
			quint8* ptr = m_frames[frame].data();

			*(quint16*)ptr = qToBigEndian((quint16)0x0001);		//Channel configuration version
			ptr += sizeof(quint16);

			*(quint16*)ptr = qToBigEndian((quint16)uartId());	//Data type (configuration)
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
				for (int i = 0; i < channelData.size(); i++)
				{
					if (index >= frameSize())
					{
						// data is bigger than frame - switch to the next frame
						//
						frame++;
						index = 0;
					}

					if (frame >= frameCount())
					{
						log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, SubsystemID %1, LM number %2: data is too big. frame = %3, frameCount = %4").arg(subsysId()).arg(channel).arg(frame).arg(frameCount()));
						return false;
					}

					m_frames[frame][index++] = channelData[i];
				}

				//switch to the next frame
				//
				frame++;
			}
		}


		// fill storage config frame
		//
		quint8* ptr = m_frames[storageConfigFrame].data();

		*(quint16*)ptr = qToBigEndian((quint16)0xCA70);	// Configuration reference mark
		ptr += sizeof(quint16);

		*(quint16*)ptr = qToBigEndian((quint16)0x0001);	// Configuration structure version
		ptr += sizeof(quint16);

		*(quint16*)ptr = qToBigEndian((quint16)ssKeyValue);	// Subsystem key
		ptr += sizeof(quint16);

		*(quint16*)ptr = qToBigEndian((quint16)m_buildNumber);	// Build number
		ptr += sizeof(quint16);

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

	ModuleFirmwareWriter* ModuleFirmwareCollection::get(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount)
	{
		bool newFirmware = m_firmwares.count(subsysId) == 0;

		ModuleFirmwareWriter& fw = m_firmwares[subsysId];

		if (newFirmware == true)
		{
			fw.init(caption, subsysId, ssKey, uartId, frameSize, frameCount, m_projectName, m_userName,
					m_buildNo, m_debug ? "debug" : "release", m_changesetId);
		}

		return &fw;
	}

	QObject* ModuleFirmwareCollection::jsGet(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount)
	{
		ModuleFirmwareWriter* fw = get(caption, subsysId, ssKey, uartId, frameSize, frameCount);

		QQmlEngine::setObjectOwnership(fw, QQmlEngine::ObjectOwnership::CppOwnership);

		return fw;
	}

	quint64 ModuleFirmwareCollection::getFirmwareUniqueId(const QString &subsystemID, int lmNumber)
	{
		if (m_firmwares.count(subsystemID) == 0)
		{
			assert(false);
			return 0;
		}

		ModuleFirmwareWriter& fw = m_firmwares[subsystemID];

		return fw.uniqueID(lmNumber);
	}

	void ModuleFirmwareCollection::setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId)
	{
		if (m_firmwares.count(subsystemID) == 0)
		{
			assert(false);
			return;
		}

		ModuleFirmwareWriter& fw = m_firmwares[subsystemID];

		fw.setGenericUniqueId(lmNumber, genericUniqueId);
	}

	std::map<QString, ModuleFirmwareWriter>& ModuleFirmwareCollection::firmwares()
	{
		return m_firmwares;
	}


}
