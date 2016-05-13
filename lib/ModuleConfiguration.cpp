
#include "../include/ModuleConfiguration.h"
#include "../include/Crc.h"
#include "../include/CUtils.h"
#include <QMap>
#include <QHash>
#include <QQmlEngine>
#include <QtEndian>

QJsVariantList::QJsVariantList(QObject* parent):
    QObject(parent)
{

}

QJsVariantList::~QJsVariantList()
{
}

void QJsVariantList::append(QVariant v)
{
    l.append(v);
}

int QJsVariantList::jsSize()
{
    return l.size();
}

QVariant QJsVariantList::jsAt(int i)
{
    return l.at(i);
}

//-----------------------------------------------------------------------------

namespace Hardware
{

    ModuleFirmware::ModuleFirmware()
	{
	}

	ModuleFirmware::~ModuleFirmware()
	{
	}

	void ModuleFirmware::init(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount, const QString &projectName, const QString &userName, int changesetId)
	{
		m_caption = caption;
		m_subsysId = subsysId;
		m_ssKey = ssKey;
		m_uartId = uartId;
		m_frameSize = frameSize;
		m_projectName = projectName;
		m_userName = userName;
		m_changesetId = changesetId;

		m_frames.clear();
		m_frames.resize(frameCount);

		for (int i = 0; i < frameCount; i++)
		{
			m_frames[i].resize(frameSize);
		}

		m_channelData.clear();

		return;
	}

	bool ModuleFirmware::save(QByteArray& dest, QString* errorMsg)
    {
        if (m_channelData.size() != 0)
        {
            if (storeChannelData(errorMsg) == false)
            {
                return false;
            }
        }

        QJsonObject jObject;

        for (int i = 0; i < frameCount(); i++)
        {
			const std::vector<quint8>& frame = m_frames[i];

            QJsonObject jFrame;

            QJsonArray array;
			for (size_t j = 0; j < frame.size(); j++)
                array.push_back(QJsonValue(frame[j]));

            jFrame.insert("data", array);
            jFrame.insert("frameIndex", i);

            jObject.insert("z_frame_" + QString().number(i), jFrame);
        }

		jObject.insert("userName", m_userName);
		jObject.insert("projectName", m_projectName);
		jObject.insert("caption", caption());
		jObject.insert("subsysId", subsysId());
        jObject.insert("uartId", uartId());
        jObject.insert("frameSize", frameSize());
        jObject.insert("framesCount", frameCount());
		jObject.insert("changesetId", m_changesetId);

		dest = QJsonDocument(jObject).toJson();

		return true;
    }

	bool ModuleFirmware::load(QString fileName)
    {
        m_frames.clear();

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

        /*int configNo = 0;
        QJsonValue jConfigVal = object.value("config" + QString::number(configNo));
        if (jConfigVal.isUndefined() == true || jConfigVal.isObject() == false)
        {
            return false;
        }*/

        //QJsonObject jConfig = jConfigVal.toObject();

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

        /*if (jConfig.value("version").isUndefined() == true)
        {
            return false;
        }
        m_version = (int)jConfig.value("version").toDouble();*/

        if (jConfig.value("uartId").isUndefined() == true)
        {
            return false;
        }
        m_uartId = (int)jConfig.value("uartId").toDouble();

        if (jConfig.value("frameSize").isUndefined() == true)
        {
            return false;
        }
        m_frameSize = (int)jConfig.value("frameSize").toDouble();

		if (jConfig.value("changesetId").isUndefined() == true)
        {
            return false;
        }
		m_changesetId = (int)jConfig.value("changesetId").toDouble();

        /*if (jConfig.value("fileName").isUndefined() == true)
        {
            return false;
        }
        m_fileName = jConfig.value("fileName").toString();*/

        if (jConfig.value("framesCount").isUndefined() == true)
        {
            return false;
        }
        int framesCount = (int)jConfig.value("framesCount").toDouble();

        for (int v = 0; v < framesCount; v++)
        {
            //ConfigDataItem item;

            QJsonValue jFrameVal = jConfig.value("z_frame_" + QString::number(v));
            if (jFrameVal.isUndefined() == true || jFrameVal.isObject() == false)
            {
                assert(false);

                m_frames.clear();
                return false;
            }

            QJsonObject jFrame = jFrameVal.toObject();

            if (jFrame.value("frameIndex").isUndefined() == true)
            {
                assert(false);

                m_frames.clear();
                return false;
            }

            //item.m_index = (int)jFrame.value("frameIndex").toDouble();

            if (jFrame.value("data").isUndefined() == true || jFrame.value("data").isArray() == false)
            {
                assert(false);

                m_frames.clear();
                return false;
            }

            std::vector<quint8> frame;

            QJsonArray array = jFrame.value("data").toArray();
            for (int i = 0; i < array.size(); i++)
            {
                //int v = array[i].toInt();
                //int v = array[i].toInt();
                frame.push_back((int)array[i].toInt());
            }


            m_frames.push_back(frame);
        }

        return true;

    }

	bool ModuleFirmware::isEmpty() const
    {
        return m_frames.size() == 0;
    }

	bool ModuleFirmware::setData8(int frameIndex, int offset, quint8 data)
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

	bool ModuleFirmware::setData16(int frameIndex, int offset, quint16 data)
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

	bool ModuleFirmware::setData32(int frameIndex, int offset, quint32 data)
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

	bool ModuleFirmware::setData64(int frameIndex, int offset, quint64 data)
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

	quint8 ModuleFirmware::data8(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint8)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return static_cast<quint8>(*(m_frames[frameIndex].data() + offset));

	}

	quint16 ModuleFirmware::data16(int frameIndex, int offset)
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

	quint32 ModuleFirmware::data32(int frameIndex, int offset)
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

    QJsVariantList* ModuleFirmware::calcHash64(QString dataString)
    {

        QByteArray bytes = dataString.toUtf8();

        quint64 result = CUtils::calcHash(bytes.data(), bytes.size());

        quint32 h = (result >> 32) & 0xffffffff;
        quint32 l = result & 0xffffffff;

        QJsVariantList* vl = new QJsVariantList(this);
        vl->append(QVariant(h));
        vl->append(QVariant(l));
        return vl;
    }

    QString ModuleFirmware::storeCrc64(int frameIndex, int start, int count, int offset)
    {
        if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint64)) || start + count >= frameSize())
        {
            qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
            return QString();
        }

        quint64 result = Crc::crc64(m_frames[frameIndex].data() + start, count);
        setData64(frameIndex, offset, result);

        //qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

        return QString::number(result);
    }

    QString ModuleFirmware::storeHash64(int frameIndex, int offset, QString dataString)
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

        return QString::number(result);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

	}

    void ModuleFirmware::writeLog(QString logString)
    {
        m_log.append(logString);
    }

	std::vector<quint8> ModuleFirmware::frame(int frameIndex)
    {
        if (frameIndex < 0 || frameIndex >= frameCount())
        {
			assert(frameIndex >= 0 && frameIndex < frameCount());
            return std::vector<quint8>();
        }

        return m_frames[frameIndex];
    }

	bool ModuleFirmware::setChannelData(int channel, int frameSize, int frameCount, const QByteArray& data, QString* errorMsg)
	{
		if (this->frameSize() != frameSize)
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QString("ModuleFirmware::setChannelData error, LM number %1: wrong frameSize (%2), expected %3.").arg(channel).arg(frameSize).arg(this->frameSize());
			}
			else
			{
				assert(this->frameSize() == frameSize);
			}
			return false;
		}

		if (this->frameCount() != frameCount)
		{
			if(errorMsg != nullptr)
			{
				*errorMsg = QString("ModuleFirmware::setChannelData error, LM number %1: wrong frameCount (%2), expected %3.").arg(channel).arg(frameSize).arg(this->frameSize());
			}
			else
			{
				assert(this->frameCount() == frameCount);
			}
			return false;
		}

		auto it = m_channelData.find(channel);
		if (it != m_channelData.end())
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QString("ModuleFirmware::setChannelData error: LM number %1 already exists.").arg(channel);
			}
			else
			{
				assert(it == m_channelData.end());
			}
			return false;
		}

		m_channelData[channel] = data;

		return true;
	}

	bool ModuleFirmware::storeChannelData(QString* errorMsg)
	{
        if (m_channelData.size() == 0)
        {
            // no channel data supplied
            //
            return true;
        }

		const int storageConfigFrame = 1;
		const int startDataFrame = 2;

		const int LMNumber_Min = 1;
		const int LMNumber_Max = 4;

        quint16 ssKeyValue = m_ssKey << 6;

		if (frameCount() < 3)
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QString("ModuleFirmware::storeChannelData error, subsystem %1: At least 3 frames needed.").arg(subsysId());
			}
			else
			{
				assert(frameCount() >= 3);
			}
			return false;
		}

		// sort channel data by growing channel number
		//
        std::vector<std::pair<int, int>> channelNumbersAndSize;
		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
		{
			int channel = it->first;
            double fSize = (double)it->second.size() / frameSize();
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
				if (errorMsg != nullptr)
				{
					*errorMsg = QString("ModuleFirmware::storeChannelData error, LM number %1: data is too big. frame = %2, frameCount = %3").arg(channel).arg(frame).arg(frameCount());
				}
				else
				{
					assert(frame < frameCount());
				}
				return false;
			}

			channelStartFrame.push_back(frame);

			// channel service information
			//
			quint8* ptr = m_frames[frame].data();

			*(quint16*)ptr = qToBigEndian((quint16)0x0001);		//Channel configuration version
			ptr += sizeof(quint16);

			*(quint16*)ptr = qToBigEndian((quint16)uartId());	//Data type (configuration)
			ptr += sizeof(quint16);

			QByteArray bytes = subsysId().toUtf8();

			*(quint64*)ptr = qToBigEndian(CUtils::calcHash(bytes.data(), bytes.size()));
			ptr += sizeof(quint64);

            *(quint16*)ptr = qToBigEndian((quint16)size);           // Frames count
            ptr += sizeof(quint16);

			frame++;

			// channel data
			//

			QByteArray& data = m_channelData[channel];

			// store channel data in frames
			//
			int index = 0;
			for (int i = 0; i < data.size(); i++)
			{
				if (index >= frameSize())
				{
					// data is bigger than frame - switch to the next frame
					//
					frame++;
					index = 0;
				}

				m_frames[frame][index++] = data[i];
			}

			//switch to the next frame
			//
			frame++;
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
				if (errorMsg != nullptr)
				{
					*errorMsg = QString("ModuleFirmware::storeChannelData error, LM number %1: Wrong channel number, expected %2..%3.").arg(channel).arg(LMNumber_Min).arg(LMNumber_Max);
				}
				else
				{
					assert(channel >= LMNumber_Min && channel <= LMNumber_Max);
				}
                return false;
            }

            quint8* ptrChannel = ptrChannelTable + (sizeof(quint16) * 3) *(channel - 1);

            quint16 startFrame = (quint16)channelStartFrame[i];

            *(quint16*)ptrChannel = qToBigEndian(startFrame);
            ptrChannel += sizeof(quint16);

            ptrChannel += sizeof(quint32);
		}

        ptr += (sizeof(quint16) * 3) * 4;

		return true;
	}


	QString ModuleFirmware::caption() const
    {
		return m_caption;
    }

	QString ModuleFirmware::subsysId() const
	{
		return m_subsysId;
	}

	int ModuleFirmware::uartId() const
	{
		return m_uartId;
	}

	quint16 ModuleFirmware::ssKey() const
	{
		return m_ssKey;
	}

	int ModuleFirmware::frameSize() const
	{
		return m_frameSize;
	}

	int ModuleFirmware::frameCount() const
	{
        return static_cast<int>(m_frames.size());
	}

    const QByteArray& ModuleFirmware::log() const
    {
        return m_log;
    }

	ModuleFirmwareCollection::ModuleFirmwareCollection(const QString &projectName, const QString &userName, int changesetId):
		m_projectName(projectName),
		m_userName(userName),
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
			fw.init(caption, subsysId, ssKey, uartId, frameSize, frameCount, m_projectName, m_userName, m_changesetId);
		}

		QQmlEngine::setObjectOwnership(&fw, QQmlEngine::ObjectOwnership::CppOwnership);
		return &fw;
	}

	std::map<QString, ModuleFirmware>& ModuleFirmwareCollection::firmwares()
	{
		return m_firmwares;
	}

}
