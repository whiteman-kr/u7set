#pragma once

#include <QString>

class Print
{
public:
	Print();

	void operator << (const QString& str);
	void newLine();
};

inline static Print print;
