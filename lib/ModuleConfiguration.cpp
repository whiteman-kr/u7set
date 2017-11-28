#include "../lib/ModuleConfiguration.h"
#include "../lib/Crc.h"
#include "../lib/CUtils.h"
#include <QMap>
#include <QHash>
#include <QtEndian>

//-----------------------------------------------------------------------------

namespace Hardware
{

    ModuleFirmware::ModuleFirmware()
	{
	}

	ModuleFirmware::~ModuleFirmware()
	{
	}

	void ModuleFirmware::init(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount, int lmDescriptionNumber, const QString &projectName,
							  const QString &userName, int buildNumber, const QString& buildConfig, int changesetId)
	{
		m_caption = caption;
		m_subsysId = subsysId;
		m_ssKey = ssKey;
		m_uartId = uartId;
		m_frameSize = frameSize;
		m_frameSizeWithCRC = frameSize + sizeof(quint64);
		m_lmDescriptionNumber = lmDescriptionNumber;
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
			m_frames[i].resize(m_frameSizeWithCRC);
		}

		return;
	}

	bool ModuleFirmware::loadHeader(QString fileName, QString &errorCode)
	{
		return loadFromFile(fileName, errorCode, false);
	}

	bool ModuleFirmware::load(QString fileName, QString &errorCode)
	{
		return loadFromFile(fileName, errorCode, true);
	}

	bool ModuleFirmware::isEmpty() const
	{
		return m_frames.size() == 0;
	}

	int ModuleFirmware::frameCount() const
	{
		return static_cast<int>(m_frames.size());
	}

	const std::vector<quint8> ModuleFirmware::frame(int frameIndex) const
	{
		if (frameIndex < 0 || frameIndex >= frameCount())
		{
			assert(frameIndex >= 0 && frameIndex < frameCount());
			return std::vector<quint8>();
		}

		return m_frames[frameIndex];
	}

	int ModuleFirmware::fileVersion() const
	{
		return m_fileVersion;
	}

	int ModuleFirmware::maxFileVersion() const
	{
		return m_maxFileVersion;
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

	int ModuleFirmware::frameSizeWithCRC() const
	{
		return m_frameSizeWithCRC;
	}

	int ModuleFirmware::changesetId() const
	{
		return m_changesetId;
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

	int ModuleFirmware::lmDescriptionNumber() const
	{
		return m_lmDescriptionNumber;
	}

	bool ModuleFirmware::loadFromFile(QString fileName, QString& errorCode, bool readDataFrames)
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
		case 4:
			return load_version2_3_4(jConfig, readDataFrames);
		case 5:
		case 6:
			return load_version5_6(jConfig, readDataFrames, errorCode);
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

		m_frameSizeWithCRC = m_frameSize + sizeof(quint64);

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

			QJsonArray array = jFrame.value("data").toArray();

			std::vector<quint8> frame;

			frame.resize(array.size() + sizeof(quint64));

			for (int i = 0; i < array.size(); i++)
			{
				frame[i] = (int)array[i].toInt();
			}

			// Count CRC

			Crc::setDataBlockCrc(v, frame.data(), (int)frame.size());

			m_frames.push_back(frame);
		}

		return true;

	}

	bool ModuleFirmware::load_version2_3_4(const QJsonObject& jConfig, bool readDataFrames)
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

		m_frameSizeWithCRC = m_frameSize + sizeof(quint64);

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
		frameVec.resize(m_frameSizeWithCRC);

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

				for (QString& s : stringValue.split(' '))
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

			// Count CRC

			Crc::setDataBlockCrc(v, frameVec.data(), (int)frameVec.size());

			m_frames.push_back(frameVec);
		}

		return true;

	}

	bool ModuleFirmware::load_version5_6(const QJsonObject& jConfig, bool readDataFrames, QString& errorCode)
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

		m_frameSizeWithCRC = (int)jConfig.value("frameSizeWithCRC").toDouble();

		if (m_frameSizeWithCRC <= m_frameSize)
		{
			assert(false);
			m_frameSizeWithCRC = m_frameSize + sizeof(quint64);
		}

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

		if (jConfig.value("lmDescriptionNumber").isUndefined() == true)
		{
			m_lmDescriptionNumber = 0;
		}
		else
		{
			m_lmDescriptionNumber = (int)jConfig.value("lmDescriptionNumber").toDouble();
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
		frameVec.resize(m_frameSizeWithCRC);

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

					if (dataPos >= m_frameSizeWithCRC / sizeof(quint16))
					{
						assert(false);
						break;
					}

					dataPos++;

					*ptr++ = qToBigEndian(v);
				}
			}

			if (Crc::checkDataBlockCrc(v, frameVec) == false)
			{
				errorCode = tr("File data is corrupt, CRC check error in frame %1.").arg(v);
				return false;
			}

			m_frames.push_back(frameVec);
		}

		return true;
	}

}
