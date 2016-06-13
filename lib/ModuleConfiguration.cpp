
#include "../lib/ModuleConfiguration.h"
#include "../lib/Crc.h"
#include "../lib/CUtils.h"
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


}
