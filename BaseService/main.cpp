#include "../lib/Service.h"
#include "../lib/WUtils.h"


class BaseServiceWorker : public ServiceWorker
{
public:
	BaseServiceWorker(const QString& serviceName, int& argc, char** argv) :
		ServiceWorker(ServiceType::BaseService, serviceName, argc, argv)
	{
	}

	virtual ServiceWorker* createInstance() const override
	{
		BaseServiceWorker* newInstance = new BaseServiceWorker(serviceName(), argc(), argv());
		newInstance->init();
		return newInstance;
	}

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override
	{
		Q_UNUSED(serviceInfo);
	}

	virtual void initCmdLineParser() override
	{
		LOG_CALL();
	}

	virtual void initialize() override
	{
		LOG_CALL();
	}

	virtual void shutdown() override
	{
		LOG_CALL();
	}
};


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0], true);

	LOG_MSG(QString("Run: %1").arg(cmdLine(argc, argv)));

	BaseServiceWorker* baseServiceWorker = new BaseServiceWorker("RPCT Base Service", argc, argv);

	ServiceStarter serviceStarter(baseServiceWorker);

	int result = serviceStarter.exec();

	LOG_MSG(QString("Exit: %1, result = %2").arg(argv[0]).arg(result));

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
