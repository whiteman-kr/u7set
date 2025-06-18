#include "SimEeprom.h"

#include <HardwareLib/ModuleFirmware.h>
#include <UtilsLib/Crc.h>

namespace
{
	template<typename TYPE>
	TYPE getData(const QByteArray& data, int eepromOffset)
	{
		// eepromOffset - in bytes
		//
		if (eepromOffset < 0 || eepromOffset > static_cast<int>(data.size() - sizeof(TYPE)))
		{
			assert(eepromOffset >= 0 && static_cast<qsizetype>(eepromOffset - sizeof(TYPE)) <= data.size());
			return 0;
		}

		TYPE result = qFromBigEndian<TYPE>(data.constData() + eepromOffset);
		return result;
	}
} // namespace

namespace Sim
{
	Eeprom::Eeprom(UartId uartId) :
		m_uartId(uartId)
	{
	}

	bool Eeprom::init(const Hardware::ModuleFirmwareData& data)
	{
		m_uartId = static_cast<UartId>(data.uartId);
		uartType = data.uartType;

		m_frameSize = data.eepromFrameSize;
		m_framePayloadSize = data.eepromFramePayloadSize;
		m_frameCount = static_cast<int>(data.frames.size());

		m_data = data.toByteArray();

		if (m_data.size() != m_frameSize * m_frameCount)
		{
			assert(m_data.size() == m_frameSize * m_frameCount);
			return false;
		}

		return true;
	}

	bool Eeprom::fill(char fillWith)
	{
		m_data.fill(fillWith);
		return true;
	}

	bool Eeprom::reset()
	{
		m_data.fill(static_cast<char>(0xFF));
		return true;
	}

	void Eeprom::clear()
	{
		m_frameSize = 0;
		m_frameCount = 0;
		m_data.clear();

		return;
	}

	bool Eeprom::parseAllocationFrame(int maxConfigurationCount)
	{
		quint16 cfgMarker = getWord(1, 0);
		quint16 cfgVersion = getWord(1, 1);
		quint16 subsystemKey = getWord(1, 2);
		quint16 buildNo = getWord(1, 3);
		quint16 configurationCount = getWord(1, 7);

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
		m_configurationCount = configurationCount;

		if (m_configurationCount > maxConfigurationCount)
		{
			return false;
		}

		m_channelServiceInfo.clear();
		m_channelServiceInfo.reserve(maxConfigurationCount);

		m_configFrameIndexes.reserve(maxConfigurationCount);

		for (int i = 0; i < maxConfigurationCount; i++)
		{
			int wordOffset = 8 + i * 3;
			quint16 startFrameIndex = getWord(1, wordOffset);

			if (startFrameIndex >= frameCount())
			{
				return false;
			}

			// If startFrameIndex is 0 then this configuration is not exists, it is ok, we can have gaps in LmNumber
			//

			// +1 to start frame index, to the payload
			//
			m_configFrameIndexes.push_back(startFrameIndex == 0 ? startFrameIndex : startFrameIndex + 1);

			// --
			//
			ChannelServiceFrame& csf = m_channelServiceInfo.emplace_back();

			if (startFrameIndex != 0)
			{
				csf.version = getWord(startFrameIndex, 0);
				csf.dataType = static_cast<UartId>(getWord(startFrameIndex, 1));
				csf.uniqueId = getUint64(startFrameIndex, 2);
				csf.frameCount = getWord(startFrameIndex, 6);
			}
		}

		return true;
	}

	quint8 Eeprom::getByte(int frameIndex, int byteOffset)
	{
		int dataIndex = frameSize() * frameIndex + byteOffset;
		assert(dataIndex < m_data.size());
		return std::as_const(m_data)[dataIndex];
	}

	quint16 Eeprom::getWord(int frameIndex, int wordOffset)
	{
		if (wordOffset < 0 || wordOffset > frameSize() - 2 || frameIndex < 0 || frameIndex > frameCount())
		{
			assert(false);
			return 0;
		}

		int eepromOffset = frameSize() * frameIndex + wordOffset * 2;

		return getData<quint16>(m_data, eepromOffset);
	}

	qint32 Eeprom::getSint32(int /*frameIndex*/, int /*wordOffset*/)
	{
		assert(false); // To Do
		return 0;
	}

	quint32 Eeprom::getUint32(int /*frameIndex*/, int /*wordOffset*/)
	{
		assert(false); // To Do
		return 0;
	}

	quint64 Eeprom::getUint64(int frameIndex, int wordOffset)
	{
		if (wordOffset < 0 || wordOffset > frameSize() - 9 || frameIndex < 0 || frameIndex > frameCount())
		{
			assert(false);
			return 0;
		}

		int eepromOffset = frameSize() * frameIndex + wordOffset * 2;

		return getData<quint64>(m_data, eepromOffset);
	}

	float Eeprom::getFloat(int /*frameIndex*/, int /*wordOffset*/)
	{
		assert(false); // To Do
		return 0;
	}

	double Eeprom::getDouble(int /*frameIndex*/, int /*wordOffset*/)
	{
		assert(false); // To Do
		return 0;
	}

	UartId Eeprom::uartId() const
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

	quint16 Eeprom::configurationCount() const
	{
		return m_configurationCount;
	}

	int Eeprom::configFrameIndex(int LmNumber) const
	{
		// LmNumber is 1-based
		//
		LmNumber--;

		if (LmNumber < 0 || LmNumber >= static_cast<int>(m_configFrameIndexes.size()))
		{
			assert(false);
			return 0; // Configuration cannot start form frame 0
		}

		return m_configFrameIndexes[LmNumber];
	}

	int Eeprom::configFramesCount(int LmNumber) const
	{
		// LmNumber is 1-based
		//
		LmNumber--;

		if (LmNumber < 0 || LmNumber >= static_cast<int>(m_channelServiceInfo.size()))
		{
			assert(false);
			return 0; // Configuration cannot start form frame 0
		}

		return m_channelServiceInfo[LmNumber].frameCount;
	}

	quint32 Eeprom::crc32(bool excludeBuildNo) const
	{
		// Assume that build no always in frame 1, offset 3 in 16-words, takes 16 bit.
		//

		if (excludeBuildNo == false)
		{
			return CRC32(m_data.constData(), m_data.size());
		}

		std::vector<char> dataWithMaskedBuildNo{m_data.cbegin(), m_data.cend()};

		// checkBuildNo - just for debugging, we can observe it.
		//
		try
		{
			size_t buildNoOffsetInBytes = frameSize() * 1 /*frameIndex*/ + 3 /*wordOffset*/ * 2;

			[[maybe_unused]] quint16 checkBuildNo =
				(dataWithMaskedBuildNo.at(buildNoOffsetInBytes + 0) << 8) | dataWithMaskedBuildNo.at(buildNoOffsetInBytes + 1);

			// Mask BuildNo.
			//
			dataWithMaskedBuildNo.at(buildNoOffsetInBytes + 0) = 0;
			dataWithMaskedBuildNo.at(buildNoOffsetInBytes + 1) = 0;

			// Mask CRC64 of the 1st frame.
			//
			size_t crcOffset = frameSize() * 2 - 8;
			dataWithMaskedBuildNo.at(crcOffset + 0) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 1) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 2) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 3) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 4) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 5) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 6) = 0;
			dataWithMaskedBuildNo.at(crcOffset + 7) = 0;
		}
		catch (const std::out_of_range&)
		{
			assert(false);
			return 0;
		}

		return CRC32(dataWithMaskedBuildNo.data(), dataWithMaskedBuildNo.size());
	}
} // namespace Sim
