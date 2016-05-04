#include <QtEndian>
#include "../include/DataProtocols.h"

void RupFrameHeader::toHostFormat()
{
	frameSize = qFromBigEndian<quint16>(frameSize);
}
