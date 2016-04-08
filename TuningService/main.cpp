#include <QCoreApplication>
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


class TuningServiceWorker : public ServiceWorker
{
public:
	TuningServiceWorker() : ServiceWorker(ServiceType::TuningService) {}
	TuningServiceWorker* createInstance() override { return new TuningServiceWorker(); }
};


int main(int argc, char *argv[])
{
	ServiceStarter serviceStarter(argc, argv, "RPCT Tuning Service", new TuningServiceWorker());

	return serviceStarter.exec();
}
