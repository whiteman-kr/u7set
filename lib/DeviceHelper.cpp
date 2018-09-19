#include "../lib/DeviceHelper.h"
#include "../lib/LmLimits.h"
#include "../lib/WUtils.h"
#include <QHostAddress>


QHash<QString, ModuleRawDataDescription*> DeviceHelper::m_modulesRawDataDescription;

const char* DeviceHelper::LM_PLATFORM_INTERFACE_CONTROLLER_SUFFUX = "_PI";

void DeviceHelper::init()
{
	assert(m_modulesRawDataDescription.count() == 0);
}


void DeviceHelper::shutdown()
{
	for(ModuleRawDataDescription* desc : m_modulesRawDataDescription)
	{
		if (desc != nullptr)
		{
			delete desc;
		}
	}

	m_modulesRawDataDescription.clear();
}


bool DeviceHelper::getIntProperty(const Hardware::DeviceObject* device, const QString& name, int* value, Builder::IssueLogger *log)
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

	if (res == false)
	{
		return false;
	}

	return value->setAddress(ipStr);
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

bool DeviceHelper:: getIpPortProperty(const Hardware::DeviceObject* device,
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

	if (result == false)
	{
		return false;
	}

	int port = 0;

	result = getPortProperty(device, portProperty, &port, emptyAllowed, defaultPort, log);

	if (result == false)
	{
		return false;
	}

	ipPort->setAddress(ipStr);
	ipPort->setPort(port);

	return result;
}

bool DeviceHelper::setIntProperty(Hardware::DeviceObject* device, const QString& name, int value, Builder::IssueLogger* log)
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
		Hardware::DeviceObject* object = device->child(i);

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

	Hardware::DeviceController* deviceController = deviceObject->toController();

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

Hardware::DeviceController* DeviceHelper::getPlatformInterfaceController(const Hardware::DeviceModule* module, Builder::IssueLogger* log)
{
	if (module->isModule() == false)
	{
		assert(false);
		return nullptr;
	}

	return getChildControllerBySuffix(module, LM_PLATFORM_INTERFACE_CONTROLLER_SUFFUX, log);
}

const Hardware::DeviceModule *DeviceHelper::getModuleOnPlace(const Hardware::DeviceModule* lm, int place)
{
	if (lm == nullptr)
	{
		assert(false);
		return nullptr;
	}

	if (place < FIRST_MODULE_PLACE || place > LAST_MODULE_PLACE)
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
		Hardware::DeviceObject* device = chassis->child(i);

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		const Hardware::DeviceModule* module = device->toModule();

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
		Hardware::DeviceObject* device = chassis->child(i);

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		Hardware::DeviceModule* module =  device->toModule();

		if (module == nullptr)
		{
			assert(false);
			continue;
		}

		if (module->isLogicModule() == true)
		{
			if (module->place() == LM1_PLACE)
			{
				return 	module;
			}

			assert(false);
			break;
		}
	}

	return nullptr;
}

const Hardware::DeviceModule* DeviceHelper::getLmOrBvb(const Hardware::DeviceChassis* chassis)
{
	if (chassis == nullptr)
	{
		assert(false);
		return nullptr;
	}

	int count = chassis->childrenCount();

	for(int i = 0; i < count; i++)
	{
		Hardware::DeviceObject* device = chassis->child(i);

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		Hardware::DeviceModule* module =  device->toModule();

		if (module == nullptr)
		{
			assert(false);
			continue;
		}

		if (module->isLogicModule() == true || module->isBvb() == true)
		{
			return 	module;

/*			if (module->place() == LM1_PLACE)
			{
				return 	module;
			}

			assert(false);
			break;*/
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

const Hardware::DeviceModule* DeviceHelper::getAssociatedLmOrBvb(const Hardware::DeviceObject* object)
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

	return getLmOrBvb(chassis);
}

const Hardware::Software* DeviceHelper::getSoftware(const Hardware::EquipmentSet* equipment, const QString& softwareID)
{
	if (equipment == nullptr)
	{
		assert(false);
		return nullptr;
	}

	const Hardware::DeviceObject* device = equipment->deviceObject(softwareID);

	if (device == nullptr)
	{
		return nullptr;
	}

	return device->toSoftware();
}

int DeviceHelper::getAllNativeRawDataSize(const Hardware::DeviceModule* lm, Builder::IssueLogger* log)
{
	if (lm == nullptr || log == nullptr)
	{
		assert(false);
		return 0;
	}

	int size = 0;

	const Hardware::DeviceChassis* chassis = lm->getParentChassis();

	if (chassis == nullptr)
	{
		assert(false);
		return 0;
	}

	int count = chassis->childrenCount();

	for(int i = 0; i < count; i++)
	{
		Hardware::DeviceObject* device = chassis->child(i);

		if (device == nullptr)
		{
			assert(false);
			continue;
		}

		if (device->isModule() == false)
		{
			continue;
		}

		Hardware::DeviceModule* module =  device->toModule();

		if (module == nullptr)
		{
			assert(false);
			continue;
		}

		if (module->hasRawData() == true)
		{
			size += getModuleRawDataSize(module, log);
		}
	}

	return size;
}


int DeviceHelper::getModuleRawDataSize(const Hardware::DeviceModule* lm, int modulePlace, bool* moduleIsFound, Builder::IssueLogger* log)
{
	if (lm == nullptr || log == nullptr || moduleIsFound == nullptr)
	{
		assert(false);
		return 0;
	}

	*moduleIsFound = false;

	const Hardware::DeviceModule* module = getModuleOnPlace(lm, modulePlace);

	if (module == nullptr)
	{
		return 0;
	}

	*moduleIsFound = true;

	int size = 0;

	if (module->hasRawData() == true)
	{
		size = getModuleRawDataSize(module, log);
	}

	return size;
}


int DeviceHelper::getModuleRawDataSize(const Hardware::DeviceModule* module, Builder::IssueLogger* log)
{
	if (module == nullptr || log == nullptr)
	{
		assert(false);
		return false;
	}

	if (module->hasRawData() == false)
	{
		return 0;
	}

	ModuleRawDataDescription* desc = nullptr;

	if (m_modulesRawDataDescription.contains(module->equipmentIdTemplate()))
	{
		desc = m_modulesRawDataDescription.value(module->equipmentIdTemplate());

		if (desc == nullptr)
		{
			return 0;
		}

		return desc->rawDataSize();
	}

	//

	desc = new ModuleRawDataDescription();

	bool result = desc->init(module, log);

	if (result == true)
	{
		m_modulesRawDataDescription.insert(module->equipmentIdTemplate(), desc);

		return desc->rawDataSize();
	}

	// parsing error occured
	// add nullptr-description to prevent repeated parsing
	//
	m_modulesRawDataDescription.insert(module->equipmentIdTemplate(), nullptr);

	return 0;
}



ModuleRawDataDescription* DeviceHelper::getModuleRawDataDescription(const Hardware::DeviceModule* module)
{
	if (module == nullptr)
	{
		assert(false);
		return nullptr;
	}

	return m_modulesRawDataDescription.value(module->equipmentIdTemplate(), nullptr);
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


