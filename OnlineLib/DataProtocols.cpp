#ifndef ONLINE_LIB_DOMAIN
#error Don't include this file in the project! Link OnlineLib instead.
#endif

#include <QtEndian>

#include "DataProtocols.h"
#include "../UtilsLib/WUtils.h"
#include "../lib/ConstStrings.h"

namespace Rup
{
	void TimeStamp::reverseBytes()
	{
		hour = reverseUint16(hour);
		minute = reverseUint16(minute);
		second = reverseUint16(second);
		millisecond = reverseUint16(millisecond);

		day = reverseUint16(day);
		month = reverseUint16(month);
		year = reverseUint16(year);
	}

	void TimeStamp::setDateTime(const QDateTime& dateTime)
	{
		QDate date = dateTime.date();

		year = static_cast<quint16>(date.year());
		month = static_cast<quint16>(date.month());
		day = static_cast<quint16>(date.day());

		QTime time = dateTime.time();

		hour = static_cast<quint16>(time.hour());
		minute = static_cast<quint16>(time.minute());
		second = static_cast<quint16>(time.second());
		millisecond = static_cast<quint16>(time.msec());
	}

	qint64 TimeStamp::toInt64(bool reverseBytes) const
	{
		quint16 _hour = hour;
		quint16 _minute = minute;
		quint16 _second = second;
		quint16 _millisecond = millisecond;

		quint16 _day = day;
		quint16 _month = month;
		quint16 _year = year;

		if (reverseBytes == true)
		{
			_hour = reverseUint16(_hour);
			_minute = reverseUint16(_minute);
			_second = reverseUint16(_second);
			_millisecond = reverseUint16(_millisecond);

			_day = reverseUint16(_day);
			_month = reverseUint16(_month);
			_year = reverseUint16(_year);
		}

		// if any asserts is failed, first of all check bytes order!
		//
		Q_ASSERT(_hour >= 0 && _hour <= 23);
		Q_ASSERT(_minute >= 0 && _minute <= 59);
		Q_ASSERT(_second >= 0 && _second <= 59);
		Q_ASSERT(_millisecond >= 0 && _millisecond <= 999);

		Q_ASSERT(_day >= 1 && _day <= 31);
		Q_ASSERT(_month >= 1 && _month <= 12);
		Q_ASSERT(_year >= 1970);

		QDateTime dt;

		dt.setTimeSpec(Qt::UTC);

		dt.setDate(QDate(_year, _month, _day));
		dt.setTime(QTime(_hour, _minute, _second, _millisecond));

		return dt.toMSecsSinceEpoch();
	}

	void Header::reverseBytes()
	{
		frameSize = reverseUint16(frameSize);
		protocolVersion = reverseUint16(protocolVersion);

		flags.all = reverseUint16(flags.all);

		dataId = reverseUint32(dataId);
		moduleType = reverseUint16(moduleType);
		numerator = reverseUint16(numerator);
		framesQuantity = reverseUint16(framesQuantity);
		frameNumber = reverseUint16(frameNumber);

		timeStamp.reverseBytes();
	}

	void Frame::calcCRC64()
	{
		CRC64 = reverseUint64(Crc::crc64(&header, Socket::ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));
	}

	bool Frame::checkCRC64()
	{
		quint64 calculatedCRC = reverseUint64(Crc::crc64(&header, Socket::ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));

		return CRC64 == calculatedCRC;
	}

	void Frame::dumpData()
	{
		QString s;

		for(quint16 i = 0; i < sizeof(data); i++)
		{
			QString v;

			if ((i % 16) == 0)
			{
				s += QString("%1").arg(static_cast<unsigned int>(i), 4, 16, Latin1Char::ZERO);
			}

			s += QString("%1").arg(static_cast<unsigned int>(data[i]), 2, 16, Latin1Char::ZERO);;

			if (i > 0 && (i % 7) == 0)
			{
				s += " ";
			}

			if (i > 0 && (i % 15) == 0)
			{
				qDebug() << s;

				s.clear();
			}
		}
	}
}


namespace FotipV2
{
	void Header::reverseBytes()
	{
		protocolVersion = reverseUint16(protocolVersion);
		uniqueId = reverseUint64(uniqueId);

		subsystemKey.wordVaue = reverseUint16(subsystemKey.wordVaue);

		operationCode = reverseUint16(operationCode);

		flags.all = reverseUint16(flags.all);

		startAddressW = reverseUint32(startAddressW);
		fotipFrameSizeB = reverseUint16(fotipFrameSizeB);
		romSizeB = reverseUint32(romSizeB);
		romFrameSizeB = reverseUint16(romFrameSizeB);
		dataType = reverseUint16(dataType);
		offsetInFrameW = reverseUint32(offsetInFrameW);
	}


	QString Frame::valueStr(bool reverseValue)
	{
		Q_ASSERT(static_cast<FotipV2::OpCode>(header.operationCode) == FotipV2::OpCode::Write);

		switch(static_cast<FotipV2::DataType>(header.dataType))
		{
		case FotipV2::DataType::AnalogFloat:
			{
				float floatValue = write.analogFloatValue;

				if (reverseValue == true)
				{
					floatValue = reverseFloat(floatValue);
				}

				return QString("%1").arg(static_cast<double>(floatValue));
			}

		case FotipV2::DataType::AnalogSignedInt:
			{
				qint32 signedIntValue = write.analogSignedIntValue;

				if (reverseValue == true)
				{
					signedIntValue = reverseInt32(signedIntValue);
				}

				return QString("%1").arg(signedIntValue);
			}

		case FotipV2::DataType::Discrete:
			{
				quint32 unsignedIntValue = write.discreteValue;

				if (reverseValue == true)
				{
					unsignedIntValue = (reverseUint32(unsignedIntValue) == 0 ? 0 : 1);
				}

				return QString("%1").arg(unsignedIntValue);
			}

		default:
			assert(false);
			return QString("Unknown FotipV2::DataType");
		}
	}

	bool Frame::isDiscreteData()
	{
		Q_ASSERT(static_cast<FotipV2::OpCode>(header.operationCode) == FotipV2::OpCode::Write);

		return static_cast<FotipV2::DataType>(header.dataType) == FotipV2::DataType::Discrete;
	}
}

void RupFotipV2::calcCRC64()
{
	CRC64 = reverseUint64(Crc::crc64(&rupHeader, Socket::ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));
}

bool RupFotipV2::checkCRC64()
{
	quint64 calculatedCRC = reverseUint64(Crc::crc64(&rupHeader, Socket::ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));

	return CRC64 == calculatedCRC;
}

