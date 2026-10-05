#include "DeviceHelper.h"

#include <HardwareLib/DeviceChassis.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/DeviceController.h>
#include <HardwareLib/Software.h>

#include <CommonLib/ConstStrings.h>
#include <CommonLib/HostAddressPort.h>
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/WUtils.h"

bool DeviceHelper::getIntProperty(std::shared_ptr<Hardware::DeviceObject> device,
								const QString& name,
								qint32* value,
								Builder::IssueLogger* log)
{ 
	return getIntProperty(device.get(), name, value, log); 
}
	
bool DeviceHelper::getIntProperty(const Hardware::DeviceObject* device, const QString& name, qint32* value, Builder::IssueLogger *log)
{
	if (device == nullptr ||
		value == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	QVariant val = device->propertyValue(name);

	if (val.isValid() == false)
	{
		logPropertyNotFoundError(device, name, log);
		return false;
	}

	bool ok = false;

	*value = val.toInt(&ok);

	if (ok == false)
	{
		// Property '%1.%2' conversion error.
		//
		log->errCFG3023(device->equipmentIdTemplate(), name);
		return false;
	}

	return true;
}

bool DeviceHelper::getUIntProperty(std::shared_ptr<Hardware::DeviceObject> device, const QString& name, quint32* value, Builder::IssueLogger* log)
{
	return getUIntProperty(device.get(), name, value, log);
}

bool DeviceHelper::getUIntProperty(const Hardware::DeviceObject* device, const QString& name, quint32* value, Builder::IssueLogger *log)
{
	if (device == nullptr ||
		value == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	QVariant val = device->propertyValue(name);

	if (val.isValid() == false)
	{
		logPropertyNotFoundError(device, name, log);
		return false;
	}

	bool ok = false;

	*value = val.toUInt(&ok);

	if (ok == false)
	{
		// Property '%1.%2' conversion error.
		//
		log->errCFG3023(device->equipmentIdTemplate(), name);
		return false;
	}

	return true;
}

bool DeviceHelper::getStrProperty(std::shared_ptr<Hardware::DeviceObject> device, const QString& name, QString* value, Builder::IssueLogger* log)
{
	return getStrProperty(device.get(), name, value, log);
}

bool DeviceHelper::getStrProperty(const Hardware::DeviceObject* device, const QString& name, QString* value, Builder::IssueLogger* log)
{
	if (device == nullptr ||
		value == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	QVariant val = device->propertyValue(name);

	if (val.isValid() == false)
	{
		logPropertyNotFoundError(device, name, log);
		return false;
	}

	*value = val.toString();

	return true;
}

bool DeviceHelper::getStrListProperty(const Hardware::DeviceObject* device, const QString& name, QStringList* strList, Builder::IssueLogger* log)
{
	if (device == nullptr ||
		strList == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	bool result = true;

	QString str;

	result &= DeviceHelper::getStrProperty(device, name, &str, log);

	str.replace(QChar(QChar::Space), Separator::SEMICOLON);
	str.replace(QChar(QChar::LineFeed), Separator::SEMICOLON);
	str.replace(QChar(QChar::CarriageReturn), Separator::SEMICOLON);
	str.replace(QChar(QChar::Tabulation), Separator::SEMICOLON);
	str.replace(Separator::COMMA, Separator::SEMICOLON);

	*strList = str.split(Separator::SEMICOLON, Qt::SkipEmptyParts);

	return result;
}

bool DeviceHelper::getStrListPropertyAsString(const Hardware::DeviceObject* device, const QString& name, QString* str, Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_RETURN_FALSE(device);
	TEST_PTR_RETURN_FALSE(str);

	QStringList list;

	if (getStrListProperty(device, name, &list, log) == false)
	{
		return false;
	}

	*str = list.join(Separator::SEMICOLON);

	return true;
}

bool DeviceHelper::getBoolProperty(const Hardware::DeviceObject* device, const QString& name, bool* value, Builder::IssueLogger *log)
{
	if (device == nullptr ||
		value == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	QVariant val = device->propertyValue(name);

	if (val.isValid() == false)
	{
		logPropertyNotFoundError(device, name, log);
		return false;
	}

	*value = val.toBool();

	return true;
}

bool DeviceHelper::getIPv4Property(const Hardware::DeviceObject* device,
								   const QString& name,
								   QString* value,
								   bool emptyAllowed,
								   const QString& defaultIp,
								   Builder::IssueLogger *log)
{
	TEST_PTR_RETURN_FALSE(log);

	QHostAddress addr;

	bool res = true;

	if (emptyAllowed == true)
	{
		// defaultIp checking
		//
		addr.setAddress(defaultIp);

		if (res == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(log);			// defaultIp is not valid IPv4 str
			return false;
		}
	}

	res = getStrProperty(device, name, value, log);

	if (res == false)
	{
		return false;
	}

	if (value->isEmpty() == true)
	{
		if (emptyAllowed == false)
		{
			// Property '%1.%2' is empty.
			//
			log->errCFG3022(device->equipmentIdTemplate(), name);
			return false;
		}
		else
		{
			*value = defaultIp;
		}
	}

	res = addr.setAddress(*value);

	if (res == false)
	{
		// Value of property %1.%2 is not valid IPv4 address.
		//
		log->errCFG3026(device->equipmentIdTemplate(), name);
		return false;
	}

	return true;
}

bool DeviceHelper::getIPv4Property(	const Hardware::DeviceObject* device,
									const QString& name,
									QHostAddress* value,
									bool emptyAllowed,
									const QString& defaultIp,
									Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);

	if (value == nullptr)
	{
		LOG_NULLPTR_ERROR(log);
		return false;
	}

	QString ipStr;

	bool res = getIPv4Property(device, name, &ipStr, emptyAllowed, defaultIp, log);

	RETURN_IF_FALSE(res);

	bool result = value->setAddress(ipStr);

	if (result == false)
	{
		// Value of property %1.%2 is not valid IPv4 address.
		//
		log->errCFG3026(device->equipmentIdTemplate(), name);
	}

	return result;
}

bool DeviceHelper::getPortProperty(const Hardware::DeviceObject* device,
								   const QString& name,
								   int* value,
								   bool emptyAllowed,
								   int defaultPort,
								   Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);

	if (emptyAllowed == true)
	{
		// defaultPort checking
		//
		if (defaultPort < Socket::PORT_LOWEST || defaultPort > Socket::PORT_HIGHEST)
		{
			assert(false);
			LOG_INTERNAL_ERROR(log);
			return false;
		}
	}

	QString portStr;

	if (getStrProperty(device, name, &portStr, log) == false)
	{
		return false;
	}

	if (portStr.isEmpty() == true)
	{
		if (emptyAllowed == false)
		{
			// Property '%1.%2' is empty.
			//
			log->errCFG3022(device->equipmentIdTemplate(), name);
			return false;
		}

		*value = defaultPort;
	}
	else
	{
		bool ok = false;

		*value = portStr.toInt(&ok);

		if (ok == false)
		{
			// Property '%1.%2' conversion error.
			//
			log->errCFG3023(device->equipmentIdTemplate(), name);
			return false;
		}
	}

	if (*value < Socket::PORT_LOWEST || *value > Socket::PORT_HIGHEST)
	{
		// Ethernet port number property %1.%2 should be in range 0..65535.
		//
		log->errCFG3027(device->equipmentIdTemplate(), name);
		return false;
	}

	return true;
}

bool DeviceHelper:: getIPv4PortProperty(const Hardware::DeviceObject* device,
									  const QString& ipProperty,
									  const QString& portProperty,
									  HostAddressPort* ipPort,
									  bool emptyAllowed,
									  const QString& defaultIP,
									  int defaultPort,
									  Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);

	if (device == nullptr || ipPort == nullptr)
	{
		LOG_NULLPTR_ERROR(log);
		return false;
	}

	QString ipStr;

	bool result = getIPv4Property(device, ipProperty, &ipStr, emptyAllowed, defaultIP, log);

	RETURN_IF_FALSE(result);

	HostAddressPort ipPortLocal;

	result &= ipPortLocal.setAddress(ipStr);

	if (result == false)
	{
		// Value of property %1.%2 is not valid IPv4 address.
		//
		log->errCFG3026(device->equipmentIdTemplate(), ipProperty);
		return false;
	}

	int port = 0;

	result = getPortProperty(device, portProperty, &port, emptyAllowed, defaultPort, log);

	RETURN_IF_FALSE(result);

	ipPortLocal.setPort(port);

	*ipPort = ipPortLocal;

	return result;
}

bool DeviceHelper::isPropertyExists(const Hardware::DeviceObject* device, const QString& name)
{
	TEST_PTR_RETURN_FALSE(device);

	return device->propertyExists(name);
}

bool DeviceHelper::setIntProperty(std::shared_ptr<Hardware::DeviceObject> device, const QString& name, qint32 value, Builder::IssueLogger* log)
{
	return setIntProperty(device.get(), name, value, log);
}

bool DeviceHelper::setIntProperty(Hardware::DeviceObject* device, const QString& name, qint32 value, Builder::IssueLogger* log)
{
	if (device == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	if (device->propertyExists(name) == false)
	{
		logPropertyNotFoundError(device, name, log);
		return false;
	}

	QVariant v(value);

	bool result = device->setPropertyValue(name, v);

	if (result == false)
	{
		logPropertyWriteError(device, name, log);
	}

	return result;
}

bool DeviceHelper::setUIntProperty(std::shared_ptr<Hardware::DeviceObject> device, const QString& name, quint32 value, Builder::IssueLogger* log)
{
	return setUIntProperty(device.get(), name, value, log);
}

bool DeviceHelper::setUIntProperty(Hardware::DeviceObject* device, const QString& name, quint32 value, Builder::IssueLogger* log)
{
	if (device == nullptr ||
		log == nullptr)
	{
		assert(false);
		return false;
	}

	if (device->propertyExists(name) == false)
	{
		logPropertyNotFoundError(device, name, log);
		return false;
	}

	QVariant v(value);

	bool result = device->setPropertyValue(name, v);

	if (result == false)
	{
		logPropertyWriteError(device, name, log);
	}

	return result;

}

Hardware::DeviceObject* DeviceHelper::getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix)
{
	return getChildDeviceObjectBySuffix(device, suffix, nullptr);
}


Hardware::DeviceObject* DeviceHelper::getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log)
{
	if (device == nullptr)
	{
		assert(false);
		return nullptr;
	}

	int childrenCount = device->childrenCount();

	for(int i = 0; i < childrenCount; i++)
	{
		Hardware::DeviceObject* object = device->child(i).get();

		if (object == nullptr)
		{
			assert(false);
			continue;
		}

		if (object->equipmentIdTemplate().endsWith(suffix) == true)
		{
			return  object;
		}
	}

	if (log != nullptr)
	{
		log->errCFG3014(suffix, device->equipmentIdTemplate());
	}

	return nullptr;
}

Hardware::DeviceController* DeviceHelper::getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix)
{
	return getChildControllerBySuffix(device, suffix, nullptr);
}

Hardware::DeviceController* DeviceHelper::getChildControllerBySuffix(std::shared_ptr<Hardware::DeviceObject> device, const QString& suffix, Builder::IssueLogger* log)
{
	return getChildControllerBySuffix(device.get(), suffix, log);
}

Hardware::DeviceController* DeviceHelper::getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log)
{
	if (device == nullptr)
	{
		assert(false);
		return nullptr;
	}

	Hardware::DeviceObject* deviceObject = getChildDeviceObjectBySuffix(device, suffix, log);

	if (deviceObject == nullptr)
	{
		return nullptr;
	}

	Hardware::DeviceController* deviceController = deviceObject->toController().get();

	if (deviceController != nullptr)
	{
		return deviceController;
	}

	if (log != nullptr)
	{
		log->errCFG3025(suffix, device->equipmentIdTemplate());
	}

	return nullptr;
}

std::vector<Hardware::DeviceController*> DeviceHelper::getChildControllers(const Hardware::DeviceObject* device)
{
	std::vector<Hardware::DeviceController*> controllers;

	if (device == nullptr)
	{
		assert(false);
		return controllers;
	}

	int childrenCount = device->childrenCount();

	for (int i = 0; i < childrenCount; i++)
	{
		Hardware::DeviceObject* object = device->child(i).get();

		if (object == nullptr)
		{
			assert(false);
			continue;
		}

		if (object->isController())
		{
			std::shared_ptr<Hardware::DeviceController> controller = object->toController();

			if (controller != nullptr)
			{
				controllers.push_back(controller.get());
			}
			else
			{
				Q_ASSERT(false);
			}
		}
	}

	return controllers;
}

Hardware::DeviceAppSignal* DeviceHelper::getChildDeviceAppSignalBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log)
{
	if (device == nullptr)
	{
		assert(false);
		return nullptr;
	}

	Hardware::DeviceObject* deviceObject = getChildDeviceObjectBySuffix(device, suffix, log);

	if (deviceObject == nullptr)
	{
		return nullptr;
	}

	Hardware::DeviceAppSignal* deviceAppSignal = deviceObject->toAppSignal().get();

	if (deviceAppSignal != nullptr)
	{
		return deviceAppSignal;
	}

	if (log != nullptr)
	{
		log->errCFG3025(suffix, device->equipmentIdTemplate());
	}

	return nullptr;
}

Hardware::DeviceController* DeviceHelper::getPlatformInterfaceController(const Hardware::DeviceModule* module, Builder::IssueLogger* log)
{
	if (module->isModule() == false)
	{
		assert(false);
		return nullptr;
	}

	return getChildControllerBySuffix(module, EquipmentPropNames::LM_PLATFORM_INTERFACE_CONTROLLER_SUFFIX, log);
}

const Hardware::DeviceModule* DeviceHelper::getModuleOnPlace(std::shared_ptr<Hardware::DeviceModule> lm, int place)
{
	return getModuleOnPlace(lm.get(), place);
}

const Hardware::DeviceModule* DeviceHelper::getModuleOnPlace(const Hardware::DeviceModule* lm, int place)
{
	if (lm == nullptr)
	{
		assert(false);
		return nullptr;
	}

	const Hardware::DeviceChassis* chassis = lm->getParentChassis();

	if (chassis == nullptr)
	{
		assert(false);
		return nullptr;
	}

	int count = chassis->childrenCount();

	for(int i = 0; i < count; i++)
	{
		Hardware::DeviceObject* device = chassis->child(i).get();

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		const Hardware::DeviceModule* module = device->toModule().get();

		if (module == nullptr)
		{
			assert(false);
			continue;
		}

		if (module->place() != place)
		{
			continue;
		}

		return module;
	}

	return nullptr;
}


const Hardware::DeviceModule* DeviceHelper::getLm(const Hardware::DeviceChassis* chassis)
{
	if (chassis == nullptr)
	{
		assert(false);
		return nullptr;
	}

	int count = chassis->childrenCount();

	for(int i = 0; i < count; i++)
	{
		Hardware::DeviceObject* device = chassis->child(i).get();

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		Hardware::DeviceModule* module =  device->toModule().get();

		if (module == nullptr)
		{
			assert(false);
			continue;
		}

		if (module->isLogicModule() == true)
		{
			if (module->place() == LM_PLACE1)
			{
				return 	module;
			}

			assert(false);
			break;
		}
	}

	return nullptr;
}

const Hardware::DeviceModule* DeviceHelper::getLmBvbMso(const Hardware::DeviceChassis* chassis)
{
	if (chassis == nullptr)
	{
		assert(false);
		return nullptr;
	}

	int count = chassis->childrenCount();

	for(int i = 0; i < count; i++)
	{
		Hardware::DeviceObject* device = chassis->child(i).get();

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		Hardware::DeviceModule* module =  device->toModule().get();

		if (module == nullptr)
		{
			assert(false);
			continue;
		}

		if (module->isLogicModule() || module->isBvb() || module->isMso())
		{
			return 	module;
		}
	}

	return nullptr;
}

const Hardware::DeviceModule* DeviceHelper::getAssociatedLm(const Hardware::DeviceObject* object)
{
	//
	// object is under chassis
	//

	if (object == nullptr)
	{
		assert(false);
		return nullptr;
	}

	const Hardware::DeviceChassis* chassis = object->getParentChassis();

	if (chassis == nullptr)
	{
		assert(false);
		return nullptr;
	}

	return getLm(chassis);
}

const Hardware::DeviceModule* DeviceHelper::getAssociatedLmBvbMso(const Hardware::DeviceObject* object)
{
	//
	// object is under chassis
	//

	if (object == nullptr)
	{
		assert(false);
		return nullptr;
	}

	const Hardware::DeviceChassis* chassis = object->getParentChassis();

	if (chassis == nullptr)
	{
		assert(false);
		return nullptr;
	}

	return getLmBvbMso(chassis);
}

std::shared_ptr<Hardware::DeviceModule> DeviceHelper::getParentVduModule(Hardware::DeviceObject* object)
{
	std::shared_ptr<Hardware::DeviceModule> moduleShared = object->getParentModuleShared();

	if (moduleShared->isVdu())
	{
		return moduleShared;
	}

	return nullptr;
}

const Hardware::Software* DeviceHelper::getSoftware(const Hardware::EquipmentSet* equipment, const QString& softwareID)
{
	if (equipment == nullptr)
	{
		assert(false);
		return nullptr;
	}

	const Hardware::DeviceObject* device = equipment->deviceObject(softwareID).get();

	if (device == nullptr)
	{
		return nullptr;
	}

	return device->toSoftware().get();
}

QStringList DeviceHelper::getSoftwareControllersIDs(const Hardware::Software* software)
{
	if (software == nullptr)
	{
		Q_ASSERT(false);
		return QStringList();
	}

	QStringList controllersIDs;

	for(const std::shared_ptr<Hardware::DeviceObject>& child : software->children())
	{
		if (child == nullptr)
		{
			Q_ASSERT(false);
			continue;
		}

		if (child->isController() == true)
		{
			controllersIDs.append(child->equipmentIdTemplate());
		}
	}

	return controllersIDs;
}

void DeviceHelper::getChildDiagSignals(std::shared_ptr<const Hardware::DeviceObject> parent,
									  std::vector<std::shared_ptr<const Hardware::DiagSignal>>* diagSignals)
{
	TEST_PTR_RETURN(parent);
	TEST_PTR_RETURN(diagSignals);

	const std::vector<std::shared_ptr<Hardware::DeviceObject>>& children = parent->children();

	for(const std::shared_ptr<Hardware::DeviceObject>& device : children)
	{
		TEST_PTR_CONTINUE(device);

		if (device->deviceType() == Hardware::DeviceType::DiagSignal)
		{
			std::shared_ptr<const Hardware::DiagSignal> diagSignal = device->toDiagSignal();

			if (diagSignal == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			diagSignals->push_back(diagSignal);
			continue;
		}

		if (device->deviceType() == Hardware::DeviceType::AppSignal)
		{
			continue;
		}

		getChildDiagSignals(device, diagSignals);
	}
}

bool DeviceHelper::isTwoChannelSoftware(const Hardware::DeviceObject* swObject, QStringList* channelsControllersIds)
{
	if (swObject->isSoftware() == false)
	{
		Q_ASSERT(false);
		return false;
	}

	if (channelsControllersIds != nullptr)
	{
		channelsControllersIds->clear();
	}

	for(int ch = CHANNEL_1; ch < 2;  ch++)
	{
		QString suffix = EquipmentPropNames::CONTROLLER_SUFFIX_CH_TEMPLATE.arg(ch + 1);

		Hardware::DeviceObject* controller = getChildControllerBySuffix(swObject, suffix, nullptr);

		if (controller == nullptr || controller->isController() == false)
		{
			if (channelsControllersIds != nullptr)
			{
				channelsControllersIds->clear();
			}

			return false;
		}

		if (channelsControllersIds != nullptr)
		{
			channelsControllersIds->append(controller->equipmentIdTemplate());
		}
	}

	return true;
}

void DeviceHelper::logPropertyNotFoundError(const Hardware::DeviceObject* device, const QString& propertyName, Builder::IssueLogger *log)
{
	if (log != nullptr && device != nullptr)
	{
		log->errCFG3020(device->equipmentIdTemplate(), propertyName);
		return;
	}
}

void DeviceHelper::logPropertyWriteError(const Hardware::DeviceObject* device, const QString& propertyName, Builder::IssueLogger *log)
{
	if (log != nullptr && device != nullptr)
	{
		log->errCFG3019(device->equipmentIdTemplate(), propertyName);
		return;
	}
}


