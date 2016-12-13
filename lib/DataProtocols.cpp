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
}
