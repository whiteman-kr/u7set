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


QString cmdLine(int argc, char** argv)
{
	if (argv == nullptr)
	{
		assert(false);
		return "";
	}

	QString cl;

	for(int i = 0; i < argc; i++)
	{
		cl += QString("%1 ").arg(argv[i]);
	}

	return cl.trimmed();
}

