#include <cmath>
#include <iostream>
#include <QString>

#include "Float16.h"

quint16 encodeFloat16(float f)
{
	Float16 f16;	// initialized by 0

	if (std::isnan(f) || f == 0)
	{
		return f16.uint16;
	}

	bool negative = f < 0? true : false;

	if (negative)
	{
		f = -f;                 // now f is positive
	}

	if (std::isinf(f) || static_cast<quint16>(f) > FLOAT16_MAX_MANTISSA)
	{
		f16.mantissa = FLOAT16_MAX_MANTISSA;
		f16.exponent = FLOAT16_MAX_EXPONENT;
		f16.sign = negative ? 1 : 0;

		return f16.uint16;
	}

    double fd = static_cast<double>(f) * FLOAT16_OFFSET;

    quint32 d = static_cast<quint32>(fd);

	quint16 exponent = 0;

	while(d > 0x7FF)
	{
		d >>= 1;
		exponent++;

		Q_ASSERT(exponent <= FLOAT16_MAX_EXPONENT);
	}

	f16.exponent = exponent;
	f16.mantissa = static_cast<quint16>(d);
	f16.sign = negative ? 1 : 0;

	return f16.uint16;
}

float decodeFloat16(const Float16& f16)
{
	qint32 mantissa = f16.mantissa;

	mantissa <<= f16.exponent;

	float f = static_cast<float>(static_cast<double>(mantissa) / FLOAT16_OFFSET);       // 32768 == 0x8000 == 2^15

	if (f16.sign == 1)
	{
		f = -f;
	}

	return f;
}

void print(float f32, const Float16& f16, float f, float of)
{
	std::cout << "-------------------------------------------------------------\n\n";

	std::cout << QString("float32  = %1\n\n").arg(f32).toStdString();

	std::cout << QString("float16  = 0x%1\n").arg(f16.uint16, 4, 16, QChar('0')).toStdString();
	std::cout << QString("mantissa = %1 (%2b)\n").arg(f16.mantissa).arg(f16.mantissa, 11, 2, QChar('0')).toStdString();
	std::cout << QString("exponent = %1 (0x%2)\n").arg(f16.exponent).arg(f16.exponent, 2, 16, QChar('0')).toStdString();
	std::cout << QString("sign     = %1 (%2)\n\n").arg(f16.sign).arg(f16.sign == 0 ? '+' : '-').toStdString();

	std::cout << QString("restored float32  = %1\n").arg(f).toStdString();
	std::cout << QString("restored float32  = %1 (original)\n\n").arg(of).toStdString();
}

//

int originalEncodeFloat16(float I);
float originalDecodeFloat16(const Float16& f16);

void testFloat16()
{
	Float16 f16;

	f16.exponent = 0;
	f16.mantissa = 1;

	float f = decodeFloat16(f16);

	std::cout << "Min float16 = " << f << "\n";

	//

	f16.exponent = 15;
	f16.mantissa = 0x7ff;

	f = decodeFloat16(f16);

	std::cout << "Max float16 = " << f << "\n\n";

	//

	for(quint16 exp = 0; exp <= FLOAT16_MAX_EXPONENT; exp++)
	{
		for(quint16 man = 1; man <= FLOAT16_MAX_MANTISSA; man++)
		{
			f16.uint16 = 0;

			f16.exponent = exp;
			f16.mantissa = man;
			f16.sign = 0;

			f = decodeFloat16(f16);

			int originalF16 = originalEncodeFloat16(f);

			originalF16 &= 0xFFFF;

			Q_ASSERT(originalF16 <= 0xFFFF);

			Float16 of16;

			of16.uint16 = static_cast<quint16>(originalF16);

			float of = decodeFloat16(of16);

			Q_ASSERT(f == of);
			//Q_ASSERT(f16.uint16 == originalF16);

			QString s = QString("0x%1 %2 == 0x%3 %4\n").
							arg(f16.uint16, 4, 16, QChar('0')).
							arg(f).
							arg(originalF16, 4, 16, QChar('0')).
							arg(of);

			std::cout << s.toStdString();
		}
	}

	//

	for(quint16 exp = 0; exp <= FLOAT16_MAX_EXPONENT; exp++)
	{
		QString s;

		s.append(QString("exp = %1: range ").arg(exp));

		f16.exponent = exp;
		f16.mantissa = 1;
		f16.sign = 0;

		float minf = decodeFloat16(f16);

		s.append(QString("%1 ... ").arg(minf));

		f16.mantissa = FLOAT16_MAX_MANTISSA;

		float maxf = decodeFloat16(f16);

		s.append(QString("%1,  ").arg(maxf));

		double maxErr = 0;
		double omaxErr = 0;

		double range = static_cast<double>(maxf) - static_cast<double>(minf);
		double dr = range / 10000;

		for(int i = 0; i < 10000; i++)
		{
			double d = minf + dr * i;

			f = static_cast<float>(d);

			//

			f16.uint16 = encodeFloat16(f);

			float df = decodeFloat16(f16);

			double err = (std::abs(f - df) / d) * 100;

			if (err > 33)
			{
				int a = 0;
				a++;
			}

			maxErr = std::max(maxErr, err);

			//

			Float16 of16;

			of16.uint16 = originalEncodeFloat16(f) & 0xFFFF;

			float odf = originalDecodeFloat16(of16);

			double oerr = (std::abs(f - odf) / d) * 100;

			if (oerr > 33)
			{
				int a = 0;
				a++;
			}

			omaxErr = std::max(omaxErr, oerr);
		}

		s.append(QString("maxErr = %1, omaxErr = %2\n").arg(maxErr).arg(omaxErr));

		std::cout << s.toStdString();
	}

	std::cout << "\n";

	std::vector<float> floats =
	{
		INFINITY, -INFINITY, NAN, 0, 1.0f, 1.5f,
		80.123f, 15.7896f, 100.0f, 2000.5f, 2046.0f, 2047.0f, 2047.1f, 0.178f, 0.000001f, 0.00003f,
		0.0000305176f,
	};

	for(float fl : floats)
	{
		f16.uint16 = encodeFloat16(fl);

		f = decodeFloat16(f16);
		float of = originalDecodeFloat16(f16);

		print(fl, f16, f, of);
	}
}

// --------------- Original code from UIK developers --------------------

struct _Float_
{
	int R: 12;
	uint M:11;
	uint P:8;
	int S:1;
};

union _FloatU_
{
	struct _Float_  F;
	float V;
};

struct _Format1_
{
	int M: 11;
	uint S: 1;
	uint P: 4;
};

union _Format1U_
{
	struct _Format1_ F;
	int I = 0;						// WhiteMan: added 0 initilization, required to clear high word16 of result
};

int originalEncodeFloat16(float I)
{
	uint a;
	int b;
	union _FloatU_ V;
	union  _Format1U_ F;
	V.V=I;
	a=V.F.M;
	a=a|0x800;
	if(V.F.P==0)
		a=0;
	a=a>>1;
	b=V.F.P;
	b=b+3-125;
	while(b<0)
	{
		a=a>>1; b++;
	}
	F.F.P=b;
	F.F.S=0;
	if(V.F.S)
	{a=0-a;F.F.S=1;}
	F.F.M=a;
	return(F.I);
}

float originalDecodeFloat16(const Float16& f16)
{
	quint16 W = f16.uint16;

	int W1 = W & 0x07FF;
	int W2 = (W & 0xF000) >> 12;
	int S = (W & 0x0800) >> 11;

	float F = (float)W1;

	F = F*(1<<W2)/0x8000;	// 0x8000 == 32768

	if (S)
	{
		F=-F;
	}

	return F;
}

// ---------------------------------------------------------------------------------
