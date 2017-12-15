#include "../lib/Service.h"
#include "../lib/WUtils.h"
#include "../lib/CircularLogger.h"

#include "version.h"


class BaseServiceWorker : public ServiceWorker
{
public:
	BaseServiceWorker(const SoftwareInfo& softwareInfo,
					  const QString& serviceName,
					  int& argc,
					  char** argv,
					  std::shared_ptr<CircularLogger> logger) :
		ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
		m_logger(logger)
	{
	}

	virtual ServiceWorker* createInstance() const override
	{
		BaseServiceWorker* newInstance = new BaseServiceWorker(softwareInfo(), serviceName(), argc(), argv(), m_logger);
		return newInstance;
	}

	virtual void initialize() override
	{
		LOG_CALL(m_logger);
	}

	virtual void shutdown() override
	{
		LOG_CALL(m_logger);
	}

	virtual void initCmdLineParser() override
	{
		cmdLineParser().addSingleValueOption("id", "EquipmentID", "Assign EquipmentID of service.", "EQUIPMENT_ID");
	}

	void loadSettings() override
	{
		m_serviceEquipmentID = getStrSetting("id");

		LOG_MSG(m_logger, QString("%1 = %2").arg("id").arg(m_serviceEquipmentID));
	}

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override
	{
		Q_UNUSED(serviceInfo);
	}

private:
	QString m_serviceEquipmentID;
	std::shared_ptr<CircularLogger> m_logger;
};


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::BaseService, "", 1, 0);			// EquipmentID will be set after command line args processing

	BaseServiceWorker baseServiceWorker(si, "RPCT Base Service", argc, argv, logger);

	ServiceStarter serviceStarter(app, baseServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
