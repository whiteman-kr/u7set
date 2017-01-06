#include "../lib/WUtils.h"
#include <QString>
#include <cassert>


void swapBytes(const char* src, char* dest, int size)
{
	dest += size - 1;

	for(int i = 0; i < size; i++)
	{
		*dest-- = *src++;
	}
}



