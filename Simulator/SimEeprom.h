#pragma once

#include <vector>
#include <QByteArray>
#include <QString>

namespace Hardware
{
	struct ModuleFirmwareData;
}

namespace Sim
{
	enum class UartId
	{
		Undefined = 0,
		Configuration = 258,
		ApplicationLogic = 257,
		Tuning = 260
	};

	struct ChannelServiceFrame
	{
		// Data from parsed [3.1.1.3.2.1	Service Information] frame
		/*
		Table 3 14 Service information structure
		Offset in bytes    ID              Description                            Size in bytes          Note
		+0                 Tun_Ch_Vers 	   Channel configuration version          2
		+2	               Tun_Ch_Dtype	   Data type (tuning)                     2
		+4	               Tun_Ch_ID	   Unique ID*                             8
		+12	               Tun_Ch_DQty	   Tuning values quantity (in frames**)   2
		+14	               Tun_Ch_Res1	   Reserve                                1002
		*/

		quint16 version = 0;
		UartId dataType = UartId::Undefined;
		quint64 uniqueId = 0;
		quint16 frameCount = 0;
	};

	class Eeprom
	{
	public:
		explicit Eeprom(UartId uartId = UartId::Undefined);
		virtual ~Eeprom();

		// Access
		//
	public:
		bool init(const Hardware::ModuleFirmwareData& data);
		bool fill(char fillWith);
		bool reset();					// Set data with FFs
		void clear();					// Clear buffer and reset size

		bool parseAllocationFrame(int maxConfigurationCount);

	public:
		quint8 getByte(int frameIndex, int byteOffset);
		quint16 getWord(int frameIndex, int wordOffset);

		qint32 getSint32(int frameIndex, int wordOffset);
		quint32 getUint32(int frameIndex, int wordOffset);

		quint64 getUint64(int frameIndex, int wordOffset);

		float getFloat(int frameIndex, int wordOffset);
		double getDouble(int frameIndex, int wordOffset);

		template <typename TYPE>
		TYPE getData(int eepromOffset);

		// Properties
		//
	public:
		UartId uartId() const;
		int size() const;
		int frameSize() const;
		int frameCount() const;

		int framePayloadSize() const;

		// Parsed allocation frame (1)
		//
		quint16 subsystemKey() const;
		quint16 buildNo() const;
		quint16 configrationsCount() const;

		int configFrameIndex(int LmNumber) const;
		int configFramesCount(int LmNumber) const;

		// Data here
		//
	private:
		UartId m_uartId = UartId::Undefined;
		QString uartType;
		QByteArray m_data;
		int m_frameSize = 0;
		int m_frameCount = 0;
		int m_framePayloadSize = 0;

		// Parsed Data
		//
	private:
		// Data from parsed allocation frame
		//
		quint16	m_subsystemKey = 0;
		quint16	m_buildNo = 0;
		quint16	m_configrationsCount = 0;
		std::vector<int> m_configFrameIndexes;

		// Service channel frame [3.1.1.3.2.1	Service Information]
		//
		std::vector<ChannelServiceFrame> m_channelServiceInfo;
	};
}

