
#include "../include/ModuleConfiguration.h"
#include "../include/Crc.h"
#include "../include/CUtils.h"
#include <QMap>
#include <QHash>
#include <QQmlEngine>
#include <QtEndian>

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

		return qToLittleEndian(static_cast<quint16>(*(m_frames[frameIndex].data() + offset)));
	}

	quint32 ModuleFirmware::data32(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint32)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return qToLittleEndian(static_cast<quint32>(*(m_frames[frameIndex].data() + offset)));
	}

	bool ModuleFirmware::storeCrc64(int frameIndex, int start, int count, int offset)
    {
        if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint64)) || start + count >= frameSize())
        {
            qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
            return false;
        }

        quint64 result = Crc::crc64(m_frames[frameIndex].data() + start, count);
        setData64(frameIndex, offset, result);

        //qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

        return true;
    }

	bool ModuleFirmware::storeHash64(int frameIndex, int offset, QString dataString)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint64)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		QByteArray bytes = dataString.toUtf8();

		quint64 result = CUtils::calcHash(bytes.data(), bytes.size());
		setData64(frameIndex, offset, result);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

		return true;
	}

	std::vector<quint8> ModuleFirmware::frame(int frameIndex)
    {
        if (frameIndex < 0 || frameIndex >= frameCount())
        {
            Q_ASSERT(false);
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
				*errorMsg = QString("ModuleFirmware::setChannelData error - channel ") + QString::number(channel) + QString(" wrong frameSize!");
			}
			assert(false);
			return false;
		}

		if (this->frameCount() != frameCount)
		{
			if(errorMsg != nullptr)
			{
				*errorMsg = "ModuleFirmware::setChannelData error - channel " + QString::number(channel) + " wrong frameCount!";
			}
			assert(false);
			return false;
		}

		auto it = m_channelData.find(channel);
		if (it != m_channelData.end())
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = "ModuleFirmware::setChannelData error - channel " + QString::number(channel) + " already exists!";
			}
			assert(false);
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

        quint16 ssKeyValue = m_ssKey << 6;

		if (frameCount() < 3)
		{
			if (errorMsg != nullptr)
			{
				*errorMsg = QString("ModuleFirmware::storeChannelData failed: At least 3 frames needed.");
			}
			assert(false);
			return false;
		}

		// sort channel data by growing channel number
		//
		std::vector<int> channelNumbers;
		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
		{
			int channel = it->first;
			channelNumbers.push_back(channel);
		}
		std::sort(channelNumbers.begin(), channelNumbers.end());

		// place channel data to frames
		//
		std::vector<int> channelStartFrame;

		int frame = startDataFrame;

		for (size_t c = 0; c < channelNumbers.size(); c++)
		{
			int channel = channelNumbers[c];

			if (frame >= frameCount())
			{
				if (errorMsg != nullptr)
				{
					*errorMsg = QString("ModuleFirmware::storeChannelData failed: data is too big. Channel = ") +
							QString::number(channel) + ", frame = " + QString::number(frame);
				}
				assert(false);
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

		*(quint16*)ptr = qToBigEndian((quint16)channelNumbers.size());	// Configuration channels quantity
		ptr += sizeof(quint16);

		for (size_t i = 0; i < channelStartFrame.size(); i++)	// Start frames
		{
			*(quint16*)ptr = qToBigEndian((quint16)channelStartFrame[i]);
			ptr += sizeof(quint16);

			//reserved
			ptr += sizeof(quint32);
		}

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
