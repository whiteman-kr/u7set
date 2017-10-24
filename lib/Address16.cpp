#include "Address16.h"

bool operator == (const Address16& addr1, const Address16& addr2)
{
	return addr1.offset() == addr2.offset() && addr1.bit() == addr2.bit();
}
