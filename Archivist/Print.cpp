#include "Print.h"

#include "../UtilsLib/WUtils.h"

Print::Print()
{
}

void Print::operator << (const QString& str)
{
	// qDebug() << C_STR(str);
	std::cout << str.toStdString();
}

void Print::newLine()
{
	std::cout << "\n";
}
