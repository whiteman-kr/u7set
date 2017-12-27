#include "Eeprom.h"
#include <cassert>
#include <QJsonDocument>
#include "../lib/ModuleFirmware.h"

namespace Sim
{

	Eeprom::Eeprom(int uartId) :
		m_uartId(uartId)
	{
	}

	Eeprom::~Eeprom()
	{
	}

	bool Eeprom::init(const Hardware::ModuleFirmwareData& data)
	{
		m_uartId = data.uartId;
		uartType = data.uartType;

		m_frameSize = data.eepromFrameSize;
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
		m_configrationsCount

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
		int to_to;
		return 0;
	}

	quint32 Eeprom::getUint32(int frameIndex, int wordOffset)
	{
		int to_to;
		return 0;
	}

	float Eeprom::getFloat(int frameIndex, int wordOffset)
	{
		int to_to;
		return 0;
	}

	double Eeprom::getDouble(int frameIndex, int wordOffset)
	{
		int to_to;
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
