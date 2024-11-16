#pragma once

#include <QtTypes>

namespace Modbus
{
	inline const quint8 FC_READ_COILS = 0x01;					// Read digital outputs and switching outputs
	inline const quint8 FC_READ_DISCRETE_INPUTS = 0x02;			// Read digital inputs and switching operations
	inline const quint8 FC_READ_HOLDING_REGISTERS = 0x03;		// Read holding registers

	inline const int FN03_MAX_REGS_COUNT = 127;

	using RegisterValue = quint16;

	inline const int REGISTER_SIZE_BYTES = sizeof(RegisterValue);

	inline const quint8 ASCII_START_MARKER = ':';
	inline const int ASCII_START_MARKER_LEN = 1;

	inline const quint8 ASCII_END_MARKER_1 = 0x0D;				// CR
	inline const quint8 ASCII_END_MARKER_2 = 0x0A;				// LF
	inline const int ASCII_END_MARKER_LEN = 2;

	inline const int ASCII_DEVICE_ID_LEN = 2;
	inline const int ASCII_FUNCTION_LEN = 2;
	inline const int ASCII_REG_START_ADDR_LEN = 4;
	inline const int ASCII_REG_COUNT_LEN = 4;
	inline const int ASCII_CRC_LEN = 2;
	inline const int ASCII_BYTES_COUNT_LEN = 2;

	static const size_t ASCII_FN03_REQUEST_SIZE =	ASCII_START_MARKER_LEN +	// marker ':'
													ASCII_DEVICE_ID_LEN +		// modbus deviceID 'XX'
													ASCII_FUNCTION_LEN +		// function '03'
													ASCII_REG_START_ADDR_LEN +	// regs start address 'XXXX'
													ASCII_REG_COUNT_LEN +		// regs count 'XXXX'
													ASCII_CRC_LEN +				// CRC 'XX'
													ASCII_END_MARKER_LEN;		// end marker CR+LF

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
		RegisterValue regValues[FN03_MAX_REGS_COUNT];			// registers values
	};

	struct Message
	{
		quint8	modbusDeviceID = 0;
		quint8	functionCode = 0;			// FC_* values

		union
		{
			Fn03_ReadHoldingRegisters_Request fn03Request;
			Fn03_ReadHoldingRegisters_Reply fn03Reply;
		};
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
		Message msg;
	};


#pragma pack(pop)

	quint8 LRC(const quint8* data, size_t dataLength);			// Modbus ASCII mode LRC calculation
	quint16 CRC16(const quint8 *data, size_t dataLength);		// Modbus RTU mode CRC16 calculation
}
