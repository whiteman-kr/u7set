#include "../Builder/ModuleFirmwareWriter.h"
#include <QtEndian>

namespace Hardware
{

ModuleFirmwareWriter::ModuleFirmwareWriter()
{

}


bool ModuleFirmwareWriter::save(QByteArray& dest, Builder::IssueLogger *log)
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

			int diIndex = 0;
			for (auto di : descriptionItems)
			{
				if (di.size() != m_descriptionFields.size())
				{
					qDebug() << "number of data items (" << di.size() << ") must be equal to number of header items (" << m_descriptionFields.size() << ")";
					assert(false);
					return false;
				}

				QJsonObject jDescriptionItem;

				int l = 0;
				for (auto v : di)
				{
					jDescriptionItem.insert(m_descriptionFields[l++], v.toString());
				}

				jDescription.insert("desc" + QString::number(diIndex++).rightJustified(8, '0'), jDescriptionItem);
			}

			if (descriptionItems.empty() == false)
			{
				jObject.insert("z_description_channel_" + QString::number(channel).rightJustified(2, '0'), jDescription);
			}
		}
	}

	// properties
	//

	jObject.insert("userName", m_userName);
	jObject.insert("projectName", m_projectName);
	jObject.insert("caption", caption());
	jObject.insert("subsysId", subsysId());
	jObject.insert("uartId", uartId());
	jObject.insert("frameSize", frameSize());
	jObject.insert("framesCount", frameCount());
	jObject.insert("buildNumber", m_buildNumber);
	jObject.insert("buildConfig", m_buildConfig);
	jObject.insert("changesetId", m_changesetId);
	jObject.insert("fileVersion", fileVersion());

	dest = QJsonDocument(jObject).toJson();

	return true;
}

bool ModuleFirmwareWriter::setChannelData(QString equipmentID, int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& data, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger *log)
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

	ModuleFirmwareData channelData;
	channelData.data = data;
	channelData.uniqueID = uniqueID;

	m_descriptonData[channel] = descriptionData;

	m_channelData[channel] = channelData;

	return true;
}

bool ModuleFirmwareWriter::storeChannelData(Builder::IssueLogger *log)
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

	const int LMNumber_Min = 1;
	const int LMNumber_Max = 12;

	quint16 ssKeyValue = m_ssKey << 6;

	if (frameCount() < 3)
	{
		log->errINT1000(QString("ModuleFirmware::storeChannelData error, subsystem %1: At least 3 frames needed.").arg(subsysId()));
		return false;
	}

	// sort channel data by growing channel number
	//
	std::vector<std::pair<int, int>> channelNumbersAndSize;
	for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
	{
		int channel = it->first;

		ModuleFirmwareData& cd = it->second;

		double fSize = (double)cd.data.size() / frameSize();
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
			log->errINT1000(QString("ModuleFirmware::storeChannelData error, LM number %1: data is too big. frame = %2, frameCount = %3").arg(channel).arg(frame).arg(frameCount()));
			return false;
		}

		channelStartFrame.push_back(frame);

		// channel data
		//
		ModuleFirmwareData& cd = m_channelData[channel];

		// channel service information
		//
		quint8* ptr = m_frames[frame].data();

		*(quint16*)ptr = qToBigEndian((quint16)0x0001);		//Channel configuration version
		ptr += sizeof(quint16);

		*(quint16*)ptr = qToBigEndian((quint16)uartId());	//Data type (configuration)
		ptr += sizeof(quint16);

		if (cd.uniqueID == 0)
		{
			QByteArray bytes = subsysId().toUtf8();

			*(quint64*)ptr = qToBigEndian(CUtils::calcHash(bytes.data(), bytes.size()));
			ptr += sizeof(quint64);
		}
		else
		{
			*(quint64*)ptr = qToBigEndian(cd.uniqueID);
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
			for (int i = 0; i < cd.data.size(); i++)
			{
				if (index >= frameSize())
				{
					// data is bigger than frame - switch to the next frame
					//
					frame++;
					index = 0;
				}

				m_frames[frame][index++] = cd.data[i];
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

	ptr += sizeof(quint64);	//reserved

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
		if (channel < LMNumber_Min || channel > LMNumber_Max)
		{
			log->errINT1000(QString("ModuleFirmware::storeChannelData error, LM number %1: Wrong LM number, expected %2..%3.").arg(channel).arg(LMNumber_Min).arg(LMNumber_Max));
			return false;
		}

		quint8* ptrChannel = ptrChannelTable + (sizeof(quint16) * 3) *(channel - 1);

		quint16 startFrame = (quint16)channelStartFrame[i];

		*(quint16*)ptrChannel = qToBigEndian(startFrame);
		ptrChannel += sizeof(quint16);

		ptrChannel += sizeof(quint32);
	}

	ptr += (sizeof(quint16)  + sizeof(quint32)) * LMNumber_Max;

	return true;
}

//
//
// ModuleFirmwareCollection
//
//

ModuleFirmwareCollection::ModuleFirmwareCollection(const QString &projectName, const QString &userName,
												   int buildNo, bool debug, int changesetId):
	m_projectName(projectName),
	m_userName(userName),
	m_buildNo(buildNo),
	m_debug(debug),
	m_changesetId(changesetId)
{
}

ModuleFirmwareCollection::~ModuleFirmwareCollection()
{
}

QObject* ModuleFirmwareCollection::jsGet(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount)
{
	bool newFirmware = m_firmwares.count(subsysId) == 0;

	ModuleFirmware& fw = m_firmwares[subsysId];

	if (newFirmware == true)
	{
		fw.init(caption, subsysId, ssKey, uartId, frameSize, frameCount, m_projectName, m_userName,
				m_buildNo, m_debug ? "debug" : "release", m_changesetId, QStringList());
	}

	QQmlEngine::setObjectOwnership(&fw, QQmlEngine::ObjectOwnership::CppOwnership);
	return &fw;
}

std::map<QString, ModuleFirmwareWriter> &ModuleFirmwareCollection::firmwares()
{
	return m_firmwares;
}

}
