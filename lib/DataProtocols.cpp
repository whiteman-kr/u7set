#include <QtEndian>
#include "../lib/DataProtocols.h"
#include "../lib/WUtils.h"

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
		CRC64 = reverseUint64(Crc::crc64(&header, ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));
	}

	bool Frame::checkCRC64()
	{
		quint64 calculatedCRC = reverseUint64(Crc::crc64(&header, ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));

		return CRC64 == calculatedCRC;
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
		assert(static_cast<FotipV2::OpCode>(header.operationCode) == FotipV2::OpCode::Write);

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
					unsignedIntValue = reverseUint32(unsignedIntValue);
				}

				return QString("%1").arg(unsignedIntValue);
			}

		default:
			assert(false);
			return QString("Unknown FotipV2::DataType");
		}
	}
}

void RupFotipV2::calcCRC64()
{
	CRC64 = reverseUint64(Crc::crc64(&rupHeader, ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));
}

bool RupFotipV2::checkCRC64()
{
	quint64 calculatedCRC = reverseUint64(Crc::crc64(&rupHeader, ENTIRE_UDP_SIZE - sizeof(quint64 /*CRC64*/ )));

	return CRC64 == calculatedCRC;
}

