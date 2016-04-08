#include "../include/Service.h"


#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visual Leak Detector
    // vld.h includes windows.h wich redefine min/max stl functions
    #ifdef min
        #undef min
    #endif
    #ifdef max
        #undef max
    #endif
#endif


class BaseServiceWorker : public ServiceWorker
{
public:
	BaseServiceWorker() : ServiceWorker(ServiceType::BaseService) {}
	ServiceWorker* createInstance() override { return new BaseServiceWorker(); }
};


int main(int argc, char *argv[])
{
	ServiceStarter serviceStarter(argc, argv, "RPCT Base Service", new BaseServiceWorker());

	return serviceStarter.exec();
}
