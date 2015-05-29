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

	Count
};


const char* const DataFormatStr[] =
{
	"Unsignaed Int",
	"Signed Int",
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


#define TO_INT(enumValue) (static_cast<int>(enumValue))
#define ENUM_COUNT(enumName) (static_cast<int>(enumName::Count))

#define C_STR(qstring) qstring.toStdString().c_str()



