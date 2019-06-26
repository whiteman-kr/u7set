#ifndef SERIALPORTPACKET_H
#define SERIALPORTPACKET_H

#include <qglobal.h>

// ==============================================================================================

// FSC ED AD. Data Protocols and Packages	D8.21.10	Version 3.0	Page 36 of 41

// ==============================================================================================

const quint32			SERIAL_PORT_DATA_SIGN		= 0x424D4C47;

// ==============================================================================================

#pragma pack (1)

struct SerialPortDataHeader
{
	quint32				Signature;
	quint16				Version;
	quint16				TransID;
	quint16				Numerator;
	quint16				DataSize;
	quint32				DataUID;
	quint64				CRC64;
};

#pragma pack ()

const int				SERIAL_PORT_HEADER_SIZE		= sizeof(SerialPortDataHeader);

// ==============================================================================================

const char* const		CommPacketField[] =
{
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Signature"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Version"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Transmission ID"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Numerator"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Data size (bytes)"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Data UID"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Header CRC-64"),
						QT_TRANSLATE_NOOP("SerialPortPacket.h", "Data CRC-64"),
};

const int				COMM_PACKET_FIELD_COUNT	= sizeof(CommPacketField)/sizeof(CommPacketField[0]);

const int				COMM_PACKET_FIELD_SIGN			= 0,
						COMM_PACKET_FIELD_VERSION		= 1,
						COMM_PACKET_FIELD_TRANSID		= 2,
						COMM_PACKET_FIELD_NUMERATOR		= 3,
						COMM_PACKET_FIELD_DATASIZE		= 4,
						COMM_PACKET_FIELD_DATAID		= 5,
						COMM_PACKET_FIELD_HEADER_CRC	= 6,
						COMM_PACKET_FIELD_DATA_CRC		= 7;

// ==============================================================================================

const int				MIN_DATA_SIZE					= 12;		// min size in bytes (4 + 8 ==> DataUniqueID + CRC64)
const int				MAX_DATA_SIZE					= 2560*2;	// max size in bytes

// ==============================================================================================
// big-little endian convertor

// Swap 2 byte, 16 bit values:
//
#define SWAP_2_BYTES(val) \
 ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

// Swap 4 byte, 32 bit values:
//
#define SWAP_4_BYTES(val) \
 ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | \
   (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )

// Swap 8 byte, 64 bit values:
//
#define SWAP_8_BYTES(val) \
 ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
   (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
   (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
   (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )

// ==============================================================================================

#endif // SERIALPORTPACKET_H
