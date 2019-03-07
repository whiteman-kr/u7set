#pragma once

#include <QtGlobal>

// CRC16 implementation acording to CCITT standards (x^16 + x^12 + x^2 + 1)
//
quint16 calcCrc16(const void* buf, int len);
