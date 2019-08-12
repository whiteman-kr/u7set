#include "../lib/WUtils.h"
#include <QString>
#include <cassert>


void swapBytes(const char* src, char* dest, int size)
{
	assert(src != dest);

	dest += size - 1;

	for(int i = 0; i < size; i++)
	{
		*dest-- = *src++;
	}
}



