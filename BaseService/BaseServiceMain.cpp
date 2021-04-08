#include "../lib/WUtils.h"
#include "../ServiceLib/Service.h"
#include "../OnlineLib/CircularLogger.h"

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
		BaseServiceWorker* newInstance = new BaseServiceWorker(softwareInfo(),
															   serviceName(),
															   argc(), argv(), m_logger);

		newInstance->init();

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
		LOG_MSG(m_logger, QString("%1 = %2").arg("id").arg(equipmentID()));
	}

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override
	{
		Q_UNUSED(serviceInfo);
	}

private:
	std::shared_ptr<CircularLogger> m_logger;
};


int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), Service::getInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::BaseService, "", 1, 0);			// EquipmentID will be set after command line args processing

	BaseServiceWorker baseServiceWorker(si,
										Service::getServiceInstanceName("Base Service", argc, argv),
										argc, argv, logger);

	ServiceStarter serviceStarter(app, baseServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
