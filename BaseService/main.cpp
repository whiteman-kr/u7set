#include "../lib/Service.h"


class BaseServiceWorker : public ServiceWorker
{
public:
	BaseServiceWorker(int argc, char** argv) :
		ServiceWorker(ServiceType::BaseService, argc, argv)
	{
	}

	BaseServiceWorker(const QStringList& cmdLineArgs) :
		ServiceWorker(ServiceType::BaseService, cmdLineArgs)
	{
	}

	ServiceWorker* createInstance() override
	{
		return new BaseServiceWorker(cmdLineArgs());
	}

	virtual void initialize() override
	{
		LOG_MSG("BaseServiceWorker is initialized");
	}

	virtual void shutdown() override
	{
		LOG_MSG("BaseServiceWorker is finished")
	}
};


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0]);

	BaseServiceWorker* baseServiceWorker = new BaseServiceWorker(argc, argv);

	ServiceStarter serviceStarter(argc, argv, "RPCT Base Service", baseServiceWorker);

	return serviceStarter.exec();
}
