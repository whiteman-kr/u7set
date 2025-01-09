#include <QtTest>
#include "TestAppDataService.h"

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);

	TestAppDataService tads(argc, argv);

	QStringList&& arguments = app.arguments();
	for (int i = arguments.size() - 1; i >= 0; i--)
	{
		if (arguments[i].contains('=') == true)
		{
			arguments.removeAt(i);
		}
	}
	return QTest::qExec(&tads, arguments);
}
