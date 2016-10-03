#include "../lib/Service.h"


class BaseServiceWorker : public ServiceWorker
{
public:
	BaseServiceWorker(const QString& serviceStrID, const QString& cfgServiceIP1, const QString& cfgServiceIP2) :
		ServiceWorker(ServiceType::BaseService, serviceStrID, cfgServiceIP1, cfgServiceIP2, "")
	{
	}

	ServiceWorker* createInstance() override
	{
		return new BaseServiceWorker(serviceEquipmentID(), cfgServiceIP1(), cfgServiceIP2());
	}
};


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QString serviceStrID = ServiceStarter::getCommandLineKeyValue(argc, argv, "id");
	QString cfgServiceIP1 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip1");
	QString cfgServiceIP2 = ServiceStarter::getCommandLineKeyValue(argc, argv, "cfgip2");

	BaseServiceWorker* baseServiceWorker = new BaseServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2);

	ServiceStarter serviceStarter(argc, argv, "RPCT Base Service", baseServiceWorker);

	return serviceStarter.exec();
}
