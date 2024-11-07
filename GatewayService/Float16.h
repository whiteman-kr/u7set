#pragma once

#include <QtTypes>

#pragma pack(push, 1)

const quint32 FLOAT32_IEE754_HIDDEN_1 = 0x00800000;

union Float32_IEE754
{
	struct
	{
		quint32 mantissa : 23;
		quint32 exponent : 8;
		quint32 sign : 1;
	};

	float float32 = 0;
	quint32 uint32;
};

//
// Non-standart float16 (BG AEC Kozloduy, UIK system)
//
//             16 bits
// --------------------------------
// |  4 bits  | 1 bit |  11 bits  |
// | Exponent | Sign  |  Mantissa |
// --------------------------------
//
union Float16
{
	struct
	{
		quint16 mantissa : 11;
		quint16 sign : 1;
		quint16 exponent : 4;
	};

	quint16 uint16 = 0;
};

inline static const quint16 FLOAT16_MAX_MANTISSA = 0x07FF;
inline static const quint8 FLOAT16_MAX_EXPONENT = 0x0F;
inline static const float FLOAT16_OFFSET = 32768.0f;

#pragma pack(pop)

quint16 encodeFloat16(float f);
float decodeFloat16(const Float16& f16);

//

void print(float f32, const Float16& f16, float f, float of);
void testFloat16();

