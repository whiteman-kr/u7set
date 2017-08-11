#include "../lib/DeviceHelper.h"
#include "../lib/LmLimits.h"


QHash<QString, ModuleRawDataDescription*> DeviceHelper::m_modulesRawDataDescription;
QString DeviceHelper::LM_PLATFORM_INTERFACE_CONTROLLER_SUFFUX = "_PI";


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

	*value = val.toInt();

	return true;
}


bool DeviceHelper::getStrProperty(const Hardware::DeviceObject* device, const QString& name, QString* value, Builder::IssueLogger *log)
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


