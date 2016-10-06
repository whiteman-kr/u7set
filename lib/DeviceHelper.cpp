#include "../lib/DeviceHelper.h"
#include "../lib/LmLimits.h"

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
	Hardware::DeviceObject* deviceObject = getChildDeviceObjectBySuffix(device, suffix, log);

	if (deviceObject == nullptr)
	{
		return nullptr;
	}

	return deviceObject->toController();
}


int DeviceHelper::getAllNativePrimaryDataSize(const Hardware::DeviceModule* lm, Builder::IssueLogger* log)
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

		//  UNCOMMENT after primaryDataSize() implementation
		//
		// size += module->primaryDataSize();
		//

		size += 10;		//  DELETE after primaryDataSize() implementation
	}

	return size;
}


int DeviceHelper::getModulePrimaryDataSize(const Hardware::DeviceModule* lm, int modulePlace, bool* moduleIsFound, Builder::IssueLogger* log)
{
	if (lm == nullptr || log == nullptr || moduleIsFound == nullptr)
	{
		assert(false);
		return 0;
	}

	*moduleIsFound = false;

	if (modulePlace < FIRST_MODULE_PLACE || modulePlace > LAST_MODULE_PLACE)
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

		if (module->place() != modulePlace)
		{
			continue;
		}

		//  UNCOMMENT after primaryDataSize() implementation
		//
		// size = module->primaryDataSize();
		//

		size = 10;				//  DELETE after primaryDataSize() implementation

		*moduleIsFound = true;

		break;
	}

	return size;
}
