#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../lib/DeviceObject.h"
#include "DiagDataService.h"


// -------------------------------------------------------------------------------
//
// DiagDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

DiagDataServiceWorker::DiagDataServiceWorker(const SoftwareInfo& softwareInfo,
											 const QString& serviceName,
											 int& argc,
											 char** argv,
											 std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
	m_logger(logger)
{
}


DiagDataServiceWorker::~DiagDataServiceWorker()
{
}


ServiceWorker* DiagDataServiceWorker::createInstance() const
{
	DiagDataServiceWorker* diagDataServiceWorker = new DiagDataServiceWorker(softwareInfo(), serviceName(), argc(), argv(), m_logger);

	return diagDataServiceWorker;
}


void DiagDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	Q_UNUSED(serviceInfo)
}


void DiagDataServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
	cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
}

void DiagDataServiceWorker::loadSettings()
{
	DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStr()));
}


void DiagDataServiceWorker::initialize()
{
}

void DiagDataServiceWorker::shutdown()
{
}

