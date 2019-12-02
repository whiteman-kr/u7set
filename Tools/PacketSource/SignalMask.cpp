#include "SignalMask.h"

#define LEN 256

// -------------------------------------------------------------------------------------------------------------------

unsigned long dp_stars[LEN];

// -------------------------------------------------------------------------------------------------------------------

int fmask1(const char* mask, const char* str, unsigned long deep)
{
	unsigned long *dp_el;
	if(deep == 0)
	{
		return 0; // Owerflow 32 stars
	}

	for( ; ; str++)
	{
			switch(*mask++)
			{
			case '*' :

				dp_el = dp_stars + (unsigned char) str; //-V542

				if(!(*dp_el & deep) && fmask1(mask, str, deep << 1))
				{
					return 1;
				}

				*dp_el |= deep;
				mask--;

			case '?' :

				if(*str == 0)
				{
					return 0;
				}
				continue;

			case 0 :

				return *str == 0;

			default :

				if(mask[-1] != *str)
				{
					return 0;
				}

				continue;
		} // switch
	} // for
} // Function fmask1

// -------------------------------------------------------------------------------------------------------------------

bool processSignalMask(const QString& Mask, const QString& StrID)
{
	if (Mask.isEmpty() == true || StrID.isEmpty() == true)
	{
		return false;
	}

	unsigned int MaskLen = static_cast<unsigned int>(Mask.length()) + 1;
	unsigned int StrIDLen = static_cast<unsigned int>(StrID.length()) + 1;

	char* UpMask = new char[MaskLen];
	char* UpStrID = new char[StrIDLen];

	memcpy(UpMask, Mask.toLocal8Bit().data(), MaskLen);
	memcpy(UpStrID, StrID.toLocal8Bit().data(), StrIDLen);

	memset(dp_stars, 0, LEN * sizeof(unsigned long));
	int Result = fmask1(UpMask, UpStrID, 1);

	delete[] UpMask;
	delete[] UpStrID;

	return Result;
}

// -------------------------------------------------------------------------------------------------------------------


