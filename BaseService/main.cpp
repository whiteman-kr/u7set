#include "../include/BaseService.h"


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
	MainFunctionWorker* mfWorker = new MainFunctionWorker();

	BaseService service(argc, argv, "RPCT Base Service", STP_BASE, mfWorker);

    return service.exec();
}
