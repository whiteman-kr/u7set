
#include "../include/ModuleConfiguration.h"
#include "../include/Crc.h"
#include "../include/CUtils.h"
#include <QMap>
#include <QHash>
#include <QQmlEngine>
#include <QtEndian>

namespace Hardware
{
	ModuleConfFirmware::ModuleConfFirmware()
	{
	}

	ModuleConfFirmware::~ModuleConfFirmware()
	{
	}

	void ModuleConfFirmware::init(QString type, QString subsysId, int uartId, int frameSize, int frameCount, const QString &projectName, const QString &userName, int changesetId)
	{
        m_type = type;
		m_subsysId = subsysId;
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

		return;
	}

	bool ModuleConfFirmware::save(QByteArray& dest) const
    {
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
        jObject.insert("type", type());
		jObject.insert("subsysId", subsysId());
        jObject.insert("uartId", uartId());
        jObject.insert("frameSize", frameSize());
        jObject.insert("framesCount", frameCount());
		jObject.insert("changesetId", m_changesetId);

		dest = QJsonDocument(jObject).toJson();

		return true;
    }

    bool ModuleConfFirmware::load(QString fileName)
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

		if (jConfig.value("type").isUndefined() == true)
        {
            return false;
        }
        m_type = jConfig.value("type").toString();

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

    bool ModuleConfFirmware::isEmpty() const
    {
        return m_frames.size() == 0;
    }

	bool ModuleConfFirmware::setData8(int frameIndex, int offset, quint8 data)
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

	bool ModuleConfFirmware::setData16(int frameIndex, int offset, quint16 data)
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

	bool ModuleConfFirmware::setData32(int frameIndex, int offset, quint32 data)
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

    bool ModuleConfFirmware::setData64(int frameIndex, int offset, quint64 data)
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

	quint8 ModuleConfFirmware::data8(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint8)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return static_cast<quint8>(*(m_frames[frameIndex].data() + offset));

	}

	quint16 ModuleConfFirmware::data16(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint16)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return qToLittleEndian(static_cast<quint16>(*(m_frames[frameIndex].data() + offset)));
	}

	quint32 ModuleConfFirmware::data32(int frameIndex, int offset)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint32)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return qToLittleEndian(static_cast<quint32>(*(m_frames[frameIndex].data() + offset)));
	}

	bool ModuleConfFirmware::storeCrc64(int frameIndex, int start, int count, int offset)
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

	bool ModuleConfFirmware::storeHash64(int frameIndex, int offset, quint16 data)
	{
		if (frameIndex >= static_cast<int>(m_frames.size()) ||
			offset > (int)(frameSize() - sizeof(quint64)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint64 result = CUtils::calcHash(&data, sizeof(data));
		setData64(frameIndex, offset, result);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

		return true;
	}

    std::vector<quint8> ModuleConfFirmware::frame(int frameIndex)
    {
        if (frameIndex < 0 || frameIndex >= frameCount())
        {
            Q_ASSERT(false);
            return std::vector<quint8>();
        }

        return m_frames[frameIndex];
    }


    QString ModuleConfFirmware::type() const
    {
        return m_type;
    }

	QString ModuleConfFirmware::subsysId() const
	{
		return m_subsysId;
	}

	int ModuleConfFirmware::uartId() const
	{
		return m_uartId;
	}

	int ModuleConfFirmware::frameSize() const
	{
		return m_frameSize;
	}

	int ModuleConfFirmware::frameCount() const
	{
        return static_cast<int>(m_frames.size());
	}

	ModuleConfCollection::ModuleConfCollection(const QString &projectName, const QString &userName, int changesetId):
		m_projectName(projectName),
		m_userName(userName),
		m_changesetId(changesetId)
	{
	}

	ModuleConfCollection::~ModuleConfCollection()
	{
	}

	QObject* ModuleConfCollection::jsGet(QString type, QString subsysId, int uartId, int frameSize, int frameCount)
	{
		bool newFirmware = m_firmwares.count(subsysId) == 0;

		ModuleConfFirmware& fw = m_firmwares[subsysId];

		if (newFirmware == true)
		{
			fw.init(type, subsysId, uartId, frameSize, frameCount, m_projectName, m_userName, m_changesetId);
		}

		QQmlEngine::setObjectOwnership(&fw, QQmlEngine::ObjectOwnership::CppOwnership);
		return &fw;
	}

	const std::map<QString, ModuleConfFirmware>& ModuleConfCollection::firmwares() const
	{
		return m_firmwares;
	}

}
