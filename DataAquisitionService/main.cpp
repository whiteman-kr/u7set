#include <QCoreApplication>
#include "DataAquisitionService.h"

#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visula Leak Detector
	// vld.h includes windows.h wich redefine min/max stl functions
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif


int main(int argc, char *argv[])
{
	DataAquisitionService service(argc, argv);

	return service.exec();
}
