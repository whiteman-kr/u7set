#include <QtEndian>
#include "../lib/DataProtocols.h"
#include "../lib/WUtils.h"


void RupTimeStamp::reverseBytes()
{
	hour = reverseUint16(hour);
	minute = reverseUint16(minute);
	second = reverseUint16(second);
	millisecond = reverseUint16(millisecond);

	day = reverseUint16(day);
	month = reverseUint16(month);
	year = reverseUint16(year);
}


void RupFrameHeader::reverseBytes()
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
