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

	void ModuleFirmware::init(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount, const QString &projectName,
							  const QString &userName, int buildNumber, const QString& buildConfig, int changesetId, int descriptionFieldsVersion, const QStringList& descriptionFields)
	{
		m_caption = caption;
		m_subsysId = subsysId;
		m_ssKey = ssKey;
		m_uartId = uartId;
		m_frameSize = frameSize;
		m_projectName = projectName;
		m_userName = userName;
		m_buildNumber = buildNumber;
		m_buildConfig = buildConfig;
		m_changesetId = changesetId;
		m_fileVersion = maxFileVersion();

		m_frames.clear();
		m_frames.resize(frameCount);

		for (int i = 0; i < frameCount; i++)
		{
			m_frames[i].resize(frameSize);
		}

		m_channelData.clear();

		m_descriptionFieldsVersion = descriptionFieldsVersion;
		m_descriptionFields = descriptionFields;

		return;
	}

	bool ModuleFirmware::load(QString fileName, QString& errorCode, bool readDataFrames)
    {
		errorCode.clear();
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

		if (jConfig.value("fileVersion").isUndefined() == true)
		{
			m_fileVersion = 1;	// in old files there is no version information
		}
		else
		{
			m_fileVersion = (int)jConfig.value("fileVersion").toInt();
		}

		switch (m_fileVersion)
		{
		case 1:
			return load_version1(jConfig, readDataFrames);
		case 2:
		case 3:
			return load_version2_3(jConfig, readDataFrames);
		default:
			errorCode = tr("This file version is not supported. Max supported version is %1.").arg(maxFileVersion());
			return false;
		}

	}

	bool ModuleFirmware::load_version1(const QJsonObject &jConfig, bool readDataFrames)
	{
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

		//

		if (readDataFrames == false)
		{
			return true;
		}


		if (jConfig.value("framesCount").isUndefined() == true)
        {
            return false;
        }
        int framesCount = (int)jConfig.value("framesCount").toDouble();

        for (int v = 0; v < framesCount; v++)
        {

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
				frame.push_back((int)array[i].toInt());
            }


            m_frames.push_back(frame);
        }

        return true;

    }

	bool ModuleFirmware::load_version2_3(const QJsonObject& jConfig, bool readDataFrames)
	{
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

		if (jConfig.value("buildConfig").isUndefined() == true)
		{
			m_buildConfig.clear();
		}
		else
		{
			m_buildConfig = jConfig.value("buildConfig").toString();
		}

		if (jConfig.value("buildNumber").isUndefined() == true)
		{
			m_buildNumber = 0;
		}
		else
		{
			m_buildNumber = (int)jConfig.value("buildNumber").toDouble();
		}

		if (jConfig.value("changesetId").isUndefined() == true)
		{
			return false;
		}
		m_changesetId = (int)jConfig.value("changesetId").toDouble();

		//

		if (readDataFrames == false)
		{
			return true;
		}

		if (jConfig.value("framesCount").isUndefined() == true)
		{
			return false;
		}
		int framesCount = (int)jConfig.value("framesCount").toDouble();

		std::vector<quint8> frameVec;
		frameVec.resize(m_frameSize);

		quint16* framePtr = (quint16*)frameVec.data();

		int frameStringWidth = -1;
		int linesCount = 0;

		for (int v = 0; v < framesCount; v++)
		{

			QJsonValue jFrameVal = jConfig.value("z_frame_" + QString::number(v).rightJustified(4, '0'));
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

			if (frameStringWidth == -1)
			{
				QString firstString = jFrame.value("data0000").toString();

				frameStringWidth = firstString.split(' ').size();
				if (frameStringWidth == 0)
				{
					assert(false);

					m_frames.clear();
					return false;
				}

				linesCount = ceil((float)m_frameSize / 2 / frameStringWidth);
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

					m_frames.clear();
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

						m_frames.clear();
						return false;
					}

					if (dataPos >= m_frameSize / sizeof(quint16))
					{
						assert(false);
						break;
					}

					dataPos++;

					*ptr++ = qToBigEndian(v);
				}
			}

			m_frames.push_back(frameVec);
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

		return QString::number(result, 16);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

	}

	quint32 ModuleFirmware::calcCrc32(int frameIndex, int start, int count)
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

    void ModuleFirmware::writeLog(QString logString)
    {
        m_log.append(logString);
    }

	void ModuleFirmware::jsSetDescriptionFields(int descriptionVersion, QString fields)
	{
		m_descriptionFieldsVersion = descriptionVersion;
		m_descriptionFields = fields.split(';');
	}

	void ModuleFirmware::setDescriptionFields(int descriptionVersion, const QStringList& fields)
	{
		m_descriptionFieldsVersion = descriptionVersion;
		m_descriptionFields = fields;
	}

	void ModuleFirmware::jsAddDescription(int channel, QString descriptionCSV)
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

	std::vector<quint8> ModuleFirmware::frame(int frameIndex)
    {
        if (frameIndex < 0 || frameIndex >= frameCount())
        {
			assert(frameIndex >= 0 && frameIndex < frameCount());
            return std::vector<quint8>();
        }

        return m_frames[frameIndex];
    }

	quint64 ModuleFirmware::uniqueID(int lmNumber)
	{
		auto it = m_dataUniqueIDMap.find(lmNumber);
		if (it == m_dataUniqueIDMap.end())
		{
			assert(false);
			return 0;
		}

		return it->second;
	}

	void ModuleFirmware::jsSetUniqueID(int lmNumber, quint64 uniqueID)
	{
		m_dataUniqueIDMap[lmNumber] = uniqueID;
	}


	void ModuleFirmware::setGenericUniqueId(int lmNumber, quint64 genericUniqueId)
	{
		if (m_channelData.empty() == false)
		{
			if (m_channelData.find(lmNumber) == m_channelData.end())
			{
				assert(false);
				return;
			}

			m_channelData[lmNumber].uniqueID = genericUniqueId;
		}
		else
		{
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

	int ModuleFirmware::changesetId() const
	{
		return m_changesetId;
	}

	int ModuleFirmware::fileVersion() const
	{
		return m_fileVersion;
	}

	int ModuleFirmware::maxFileVersion() const
	{
		return m_maxFileVersion;
	}

	QString ModuleFirmware::projectName() const
	{
		return m_projectName;
	}

	QString ModuleFirmware::userName() const
	{
		return m_userName;
	}

	int ModuleFirmware::buildNumber() const
	{
		return m_buildNumber;
	}

	QString ModuleFirmware::buildConfig() const
	{
		return m_buildConfig;
	}


    const QByteArray& ModuleFirmware::log() const
    {
        return m_log;
    }


}
