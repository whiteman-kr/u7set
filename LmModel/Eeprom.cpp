#include "Eeprom.h"
#include <cassert>
#include <QJsonDocument>

namespace LmModel
{

	Eeprom::Eeprom(int uartId) :
		m_uartId(uartId)
	{
	}

	Eeprom::~Eeprom()
	{
	}

	bool Eeprom::init(int frameSize, int frameCount, int fillWith)
	{
		m_frameSize = frameSize;
		m_frameCount = frameCount;

		m_data.resize(m_frameSize * m_frameCount);

		bool ok = fill(fillWith);

		return ok;
	}

	bool Eeprom::fill(int fillWith)
	{
		m_data.fill((char)fillWith);
		return true;
	}

	bool Eeprom::reset()
	{
		m_data.fill((char)(0xFF));
		return true;
	}

	void Eeprom::clear()
	{
		m_frameSize = 0;
		m_frameCount = 0;
		m_data.clear();

		return;
	}

	bool Eeprom::loadData(const QByteArray& fileData, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		QJsonDocument document = QJsonDocument::fromJson(fileData);

		if (document.isEmpty() == true ||
			document.isNull() == true ||
			document.isObject() == false)
		{
			return false;
		}

		QJsonObject jConfig = document.object();
		int fileVersion = 1;

		if (jConfig.value("fileVersion").isUndefined() == true)
		{
			fileVersion = 1;	// in old files there is no version information
		}
		else
		{
			fileVersion = jConfig.value("fileVersion").toInt();
		}

		switch (fileVersion)
		{
		case 6:
			return loadVersion6(jConfig, errorMessage);
		default:
			// Unsupported version
			//
			*errorMessage = QString("Unsupported file version, version = %1").arg(fileVersion);
			return false;
		}

		return true;
	}

	bool Eeprom::loadVersion6(const QJsonObject& jConfig, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		// --
		//
		if (jConfig.value("uartId").isUndefined() == true)
		{
			*errorMessage = QString("Undefined UartID");
			return false;
		}

		int loadedUartId = jConfig.value("uartId").toInt();
		if (loadedUartId != m_uartId)
		{
			*errorMessage = QString("Wrong file UartID, expected %1, loaded %2").arg(m_uartId).arg(loadedUartId);
			return false;
		}

		// --
		//
		if (jConfig.value("frameSize").isUndefined() == true)
		{
			*errorMessage = QString("Undefined FrameSize");
			return false;
		}

		m_framePayloadSize = jConfig.value("frameSize").toInt();
		if (m_framePayloadSize > m_frameSize)	// Example: Payload = 1016, frameSize = 1024
		{
			*errorMessage = QString("Frame PayloadSize (frameSize) is wrong.");
			return false;
		}

		// --
		//
		int frameSizeWithCRC = jConfig.value("frameSizeWithCRC").toInt();
		if (frameSizeWithCRC != m_frameSize)
		{
			*errorMessage = QString("Undefined FrameSizeWithCRC");
			return false;
		}

		// Loading frame data
		//
		if (jConfig.value("framesCount").isUndefined() == true)
		{
			return false;
		}
		int framesCount = (int)jConfig.value("framesCount").toDouble();

		if (framesCount != m_frameCount)
		{
			*errorMessage = QString("Wrong framesCount");
			return false;
		}

		QByteArray frameData;
		frameData.resize(m_frameSize);

		quint16* framePtr = (quint16*)frameData.data();

		int frameStringWidth = -1;
		int linesCount = 0;

		for (int frameIndex = 0; frameIndex < framesCount; frameIndex++)
		{
			QString tagName = "z_frame_" + QString::number(frameIndex).rightJustified(4, '0');

			QJsonValue jFrameVal = jConfig.value(tagName);
			if (jFrameVal.isUndefined() == true || jFrameVal.isObject() == false)
			{
				*errorMessage = QString("Tag %1 is not defined").arg(tagName);
				return false;
			}

			QJsonObject jFrame = jFrameVal.toObject();
			if (jFrame.value("frameIndex").isUndefined() == true)
			{
				*errorMessage = QString("Tag frameIndex for %1 is not defined").arg(tagName);
				return false;
			}

			if (frameStringWidth == -1)
			{
				QString firstString = jFrame.value("data0000").toString();

				frameStringWidth = firstString.split(' ').size();
				if (frameStringWidth == 0)
				{
					*errorMessage = QString("Parse data for %1 error").arg(tagName);
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
					*errorMessage = QString("Tag %1 is not defined").arg(stringName);
					return false;
				}

				QString stringValue = v.toString();

				for (QString& s : stringValue.split(' ')) // split takes much time, try to optimize
				{
					bool ok = false;
					quint16 v = s.toUInt(&ok, 16);

					if (ok == false)
					{
						*errorMessage = QString("Conversion %1 error, tag %1").arg(s).arg(stringName);
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

//			if (Crc::checkDataBlockCrc(frameIndex, frameData) == false)
//			{
//				errorCode = QString("File data is corrupt, CRC check error in frame %1.").arg(frameIndex);
//				return false;
//			}

			for (int copyIndex = 0; copyIndex < frameData.size(); copyIndex ++)
			{
				int dataIndex = frameIndex * m_frameSize + copyIndex;
				if (dataIndex >= m_data.size())
				{
					assert(false);
					return false;
				}

				m_data[dataIndex] = frameData[copyIndex];
			}
		}

		return true;
	}

	bool Eeprom::parseAllocationFrame()
	{
		quint16 cfgMarker = getWord(1, 0);
		quint16 cfgVersion = getWord(1, 1);
		quint16	subsystemKey = getWord(1, 2);
		quint16	buildNo = getWord(1, 3);
		quint16	configrationsCount = getWord(1, 7);

		if (cfgMarker != 0xca70)
		{
			// it seems not correct frame
			//
			return false;
		}

		if (cfgVersion != 0x0001)
		{
			// Only version 1 is knows to this parser
			//
			return false;
		}

		m_subsystemKey = subsystemKey;
		m_buildNo = buildNo;
		m_configrationsCount = configrationsCount;

		if (m_configrationsCount > 128)					// Just some reasonable number
		{
			return false;
		}

		m_configFrameIndexes.reserve(m_configrationsCount);

		for (int i = 0; i < m_configrationsCount; i++)
		{
			int wordOffset = 8 + i * 3;
			quint16 startFrameIndex = getWord(1, wordOffset);

			if (startFrameIndex >= frameCount())
			{
				return false;
			}

			m_configFrameIndexes.push_back(startFrameIndex);
		}

		return true;
	}

	quint8 Eeprom::getByte(int frameIndex, int byteOffset)
	{
		int dataIndex = frameSize() * frameIndex + byteOffset;
		assert(dataIndex < m_data.size());
		return m_data[dataIndex];
	}

	quint16 Eeprom::getWord(int frameIndex, int wordOffset)
	{
		if (wordOffset < 0 ||
			wordOffset > frameSize() - 2 ||
			frameIndex < 0  ||
			frameIndex > frameCount())
		{
			assert(false);
			return 0;
		}

		int eepromOffset = frameSize() * frameIndex + wordOffset * 2;

		return getData<quint16>(eepromOffset);
	}

	qint32 Eeprom::getSint32(int frameIndex, int wordOffset)
	{
		return 0;
	}

	quint32 Eeprom::getUint32(int frameIndex, int wordOffset)
	{
		return 0;
	}

	float Eeprom::getFloat(int frameIndex, int wordOffset)
	{
		return 0;
	}

	double Eeprom::getDouble(int frameIndex, int wordOffset)
	{
		return 0;
	}

	template <typename TYPE>
	TYPE Eeprom::getData(int eepromOffset)
	{
		// eepromOffset - in bytes
		//
		if (eepromOffset < 0 || eepromOffset > m_data.size() - sizeof(TYPE))
		{
			assert(eepromOffset >= 0 &&
				   eepromOffset - sizeof(TYPE) <= m_data.size());
			return 0;
		}

		TYPE result = qFromBigEndian<TYPE>(m_data.constData() + eepromOffset);
		return result;
	}

	int Eeprom::uartId() const
	{
		return m_uartId;
	}

	int Eeprom::size() const
	{
		return m_frameSize * m_frameCount;
	}

	int Eeprom::frameSize() const
	{
		return m_frameSize;
	}

	int Eeprom::frameCount() const
	{
		return m_frameCount;
	}

	int Eeprom::framePayloadSize() const
	{
		return m_framePayloadSize;
	}

	quint16 Eeprom::subsystemKey() const
	{
		return m_subsystemKey;
	}

	quint16 Eeprom::buildNo() const
	{
		return m_buildNo;
	}

	quint16 Eeprom::configrationsCount() const
	{
		return m_configrationsCount;
	}

	int Eeprom::configFrameIndex(int configurationNo) const
	{
		if (configurationNo < 0 ||
			configurationNo >= static_cast<int>(m_configFrameIndexes.size()))
		{
			assert(false);
			return -1;
		}

		return m_configFrameIndexes[configurationNo];
	}
}
