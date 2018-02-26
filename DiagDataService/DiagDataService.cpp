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

	cp.addSingleValueOption("id", "EquipmentID", "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", "CfgServiceIP1", "IP-addres of first Configuration Service.", "");
	cp.addSingleValueOption("cfgip2", "CfgServiceIP2", "IP-addres of second Configuration Service.", "");
}

void DiagDataServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting("EquipmentID");

	m_cfgServiceIP1Str = getStrSetting("CfgServiceIP1");

	m_cfgServiceIP1 = HostAddressPort(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_REQUEST);

	m_cfgServiceIP2Str = getStrSetting("CfgServiceIP2");

	m_cfgServiceIP2 = HostAddressPort(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg("EquipmentID").arg(m_equipmentID));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2 (%3)")).arg("CfgServiceIP1").arg(m_cfgServiceIP1Str).arg(m_cfgServiceIP1.addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2 (%3)")).arg("CfgServiceIP2").arg(m_cfgServiceIP2Str).arg(m_cfgServiceIP2.addressPortStr()));
}


void DiagDataServiceWorker::initialize()
{
}

void DiagDataServiceWorker::shutdown()
{
}

