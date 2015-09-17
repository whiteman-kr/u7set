#pragma once

enum class SignalType
{
	Analog,
	Discrete,

	Count
};


enum class DataFormat
{
	UnsignedInt,
	SignedInt,
	Float,

	Count
};


const char* const DataFormatStr[] =
{
	"Unsigned Int",
	"Signed Int",
	"Float",
};


enum class ByteOrder
{
	LittleEndian,
	BigEndian,

	Count
};


const char* const ByteOrderStr[] =
{
	"Little Endian",
	"Big Endian",
};

enum class UartID
{
	LmAppLogicUart = 0x101,
	LmConfigUart = 0x102,
	LmServiceUart = 0x103,
	LmTuningUart = 0x104,
	AimUart = 0x203,
	AomUart = 0x303,
	DimUart = 0x403,
	DomUart = 0x503,
	AifmUart = 0x603,
	OcmUart = 0x703

};


#define TO_INT(enumValue) (static_cast<int>(enumValue))
#define ENUM_COUNT(enumName) (static_cast<int>(enumName::Count))

#define C_STR(qstring) qstring.toStdString().c_str()

template <typename EnumType>
EnumType IntToEnum(int value)
{
	return static_cast<EnumType>(value);
}


