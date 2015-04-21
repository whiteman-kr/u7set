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

	int maskPos = 0;	//������� ������� �����
	int strPos = 0;	//������� ������� ������

	while(true)
	{
		if (strPos == strLen && maskPos == maskLen)		//����� �� ����� ������ � ����� ������ �������
		{
			return true;
		}

		if (strPos == strLen)		//����� �� ����� ������ ������ �� �������
		{
			return false;
		}

		if (maskPos == maskLen)	//����� �� ����� ����� - ������ �� �������
		{
			return false;
		}

		QChar m = mask.at(maskPos);

		if ( m == '~' )	//���� � ����� ��������� ������ - ������� ���
		{
			return true;
		}

		if ( m == '?' )	//? - ���������� ������ � ������ �������
		{
			maskPos++;
			strPos++;
			continue;
		}	

		if ( m == '*' ) //* - ���������� ��� �� ������� ������� "_"
		{
			while (str.at(strPos) != '_')
			{
				strPos++;
				if (strPos == strLen)	//����� �� ����� ������ - ������ �������
				{
					if (maskPos == maskLen - 1)
					{
						return true;		//������ ������� ���� ��� ��������� ���������
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

		if (str.at(strPos) != m)		//��������� ������� �� �������
		{
			return false;
		}

		strPos++;
		maskPos++;
	}

	return true;

}

	
