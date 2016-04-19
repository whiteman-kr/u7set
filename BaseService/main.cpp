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
	BaseServiceWorker(const QString& serviceStrID, const QString& cfgServiceIP1, const QString& cfgServiceIP2) :
		ServiceWorker(ServiceType::BaseService, serviceStrID, cfgServiceIP1, cfgServiceIP2)
	{
	}

	ServiceWorker* createInstance() override
	{
		return new BaseServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2());
	}
};


int main(int argc, char *argv[])
{
	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");

	BaseServiceWorker* baseServiceWorker = new BaseServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2);

	ServiceStarter serviceStarter(argc, argv, "RPCT Base Service", baseServiceWorker);

	return serviceStarter.exec();
}
