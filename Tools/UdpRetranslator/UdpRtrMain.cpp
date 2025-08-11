#include "UdpRetranslatorApp.h"
#include <CommonLib/u7_vld.h>

int main(int argc, char** argv)
{
	Vld::setVldReportFilterHook();

	int result = 0;

	{
		app.init(argc, argv);

		result = app.run();
	}

	return result;
}

