#include "../include/DeviceHelper.h"

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
