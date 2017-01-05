#include "../lib/Service.h"
#include "../lib/WUtils.h"


class BaseServiceWorker : public ServiceWorker
{
public:
	BaseServiceWorker(const QString& serviceName, int& argc, char** argv, int majVersion, int minVersion) :
		ServiceWorker(ServiceType::BaseService, serviceName, argc, argv, majVersion, minVersion)
	{
	}

	virtual ServiceWorker* createInstance() const override
	{
		BaseServiceWorker* newInstance = new BaseServiceWorker(serviceName(), argc(), argv(), majorVersion(), minorVersion());
		newInstance->init();
		return newInstance;
	}

	virtual void initialize() override
	{
		LOG_CALL();
	}

	virtual void shutdown() override
	{
		LOG_CALL();
	}

	virtual void initCmdLineParser() override
	{
		cmdLineParser().addSingleValueOption("id", "Assign EquipmentID of service.", "EQUIPMENT_ID");
	}

	void processCmdLineSettings() override
	{
		CommandLineParser& cp = cmdLineParser();

		if (cp.optionIsSet("id") == true)
		{
			m_settings.setValue("id", cp.optionValue("id"));
		}
	}

	void loadSettings() override
	{
		m_serviceEquipmentID = m_settings.value("id").toString();
	}

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override
	{
		Q_UNUSED(serviceInfo);
	}

private:
	QString m_serviceEquipmentID;
};


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QCoreApplication app(argc, argv);

	INIT_LOGGER(argv[0])

	LOG_MSG(QString("Run: %1").arg(cmdLine(argc, argv)));

	BaseServiceWorker* baseServiceWorker = new BaseServiceWorker("RPCT Base Service", argc, argv, 1, 0);

	ServiceStarter serviceStarter(app, baseServiceWorker);

	int result = serviceStarter.exec();

	LOG_MSG(QString("Exit: %1, result = %2").arg(argv[0]).arg(result));

	google::protobuf::ShutdownProtobufLibrary();

	SHUTDOWN_LOGGER

	return result;
}
