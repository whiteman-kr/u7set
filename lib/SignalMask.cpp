#include <QString>
#include "../include/SignalMask.h"

//-------------------------------------------------------------------------------------------------------------
bool processDiagSignalMask (const QString& mask, const QString& str)
{
	if (mask.isEmpty())
	{
		return true;
	}

	int maskLen = mask.length();
	int strLen = str.length();

	int maskPos = 0;	//текущая позиция маски
	int strPos = 0;	//текущая позиция строки

	while(true)
	{
		if (strPos == strLen && maskPos == maskLen)		//дошли до конца строки и маски строки совпали
		{
			return true;
		}

		if (strPos == strLen)		//дошли до конца строки строки не совпали
		{
			return false;
		}

		if (maskPos == maskLen)	//дошли до конца маски - строки не совпали
		{
			return false;
		}

		QChar m = mask.at(maskPos);

		if ( m == '~' )	//если в маске встретили тильду - вернуть ТРУ
		{
			return true;
		}

		if ( m == '?' )	//? - пропустить символ и начать сначала
		{
			maskPos++;
			strPos++;
			continue;
		}	

		if ( m == '*' ) //* - пропустить все до первого символа "_"
		{
			while (str.at(strPos) != '_')
			{
				strPos++;
				if (strPos == strLen)	//дошли до конца строки - строки совпали
				{
					if (maskPos == maskLen - 1)
					{
						return true;		//строки совпали если это последняя звездочка
					}
					else
					{
						return false;
					}
				}
			}

			maskPos++;
			continue;
		}

		if (str.at(strPos) != m)		//очередные символы не совпали
		{
			return false;
		}

		strPos++;
		maskPos++;
	}

	return true;

}

	
