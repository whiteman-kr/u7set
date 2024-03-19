#pragma once

namespace Modbus
{
	inline const quint8 FC_READ_COILS = 0x01;					// Read digital outputs and switching outputs
	inline const quint8 FC_READ_DISCRETE_INPUTS = 0x02;			// Read digital inputs and switching operations
	inline const quint8 FC_READ_HOLDING_REGISTERS = 0x03;		// Read holding registers

	inline const int FN03_MAX_REGS_COUNT = 127;

	using RegisterValue = quint16;

	inline const int REGISTER_SIZE_BYTES = sizeof(RegisterValue);

#pragma pack(push, 1)

	struct Fn03_ReadHoldingRegisters_Request
	{
		quint16 regsStartAddr;				// in BE
		quint16 regsCount;					// in BE

		void reverseBytes();
	};

	struct Fn03_ReadHoldingRegisters_Reply
	{
		quint8 bytesCount;										// real filled size in bytes
		RegisterValue regValues[FN03_MAX_REGS_COUNT];					// registers values
	};

	struct TcpHeader
	{
		quint16 transactionID = 0;
		quint16 protocolID = 0;				// always 0 for Modbus-TCP
		quint16 length = 0;					// length in bytes == sizeof(slaveID) + sizeof(functionCode) + sizeof(modbusData)

		void reverseBytes();
	};

	struct TcpFrame
	{
		TcpHeader header;

		quint8	modbusDeviceID = 0;
		quint8	functionCode = 0;			// FC_* values

		union
		{
			Fn03_ReadHoldingRegisters_Request fn03Request;
			Fn03_ReadHoldingRegisters_Reply fn03Reply;
		};

		void reverseBytes();
	};

#pragma pack(pop)

	quint8 LRC (const quint8* data, int dataLength);		// Modbus ASCII mode LRC calculation
	quint16 CRC16 (const quint8 *data, int dataLength);		// Modbus RTU mode CRC16 calculation

}
