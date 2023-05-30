#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../HardwareLib/DeviceObject.h"
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
											 CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}

DiagDataServiceWorker::DiagDataServiceWorker(const DiagDataServiceWorker* worker) :
	ServiceWorker(worker)
{
}

DiagDataServiceWorker::~DiagDataServiceWorker()
{
}

ServiceWorker* DiagDataServiceWorker::createInstance() const
{
	DiagDataServiceWorker* diagDataServiceWorker = new DiagDataServiceWorker(this);
	return diagDataServiceWorker;
}

void DiagDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	TEST_PTR_RETURN(m_serviceSettings);

	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::DiagDataService, *m_serviceSettings.get());

	serviceInfo.set_settingsxml(xmlString.toStdString());
}

void DiagDataServiceWorker::initCustomCmdLineArgs()
{
	addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	addValueCmdLineArg(CmdLineArg::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
	addValueCmdLineArg(CmdLineArg::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
}

void DiagDataServiceWorker::loadSettings()
{
	DEBUG_LOG_MSG(logger(), QString(tr("Settings from command line or registry:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStr()));
}

void DiagDataServiceWorker::initialize()
{
}

void DiagDataServiceWorker::shutdown()
{
}

