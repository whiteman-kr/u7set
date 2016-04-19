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
		logPropertyNotFoundError(name, device->strId(), log);
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
		logPropertyNotFoundError(name, device->strId(), log);
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
		logPropertyNotFoundError(name, device->strId(), log);
		assert(false);
		return false;
	}

	QVariant val = device->propertyValue(name);

	if (val.isValid() == false)
	{
		return false;
	}

	*value = val.toBool();

	return true;
}


void DeviceHelper::logPropertyNotFoundError(const QString& propertyName, const QString& deviceStrID, Builder::IssueLogger *log)
{
	if (log == nullptr)
	{
		LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
				  QString(tr("Property '%1' is not found in device '%2'")).arg(propertyName).arg(deviceStrID));
		assert(false);
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

		if (object->strId().endsWith(suffix) == true)
		{
			return  object;
		}
	}

	if (log != nullptr)
	{
		LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
						   QString("Can't find child object by suffix '%1' in object '%2'").
						   arg(suffix).arg(device->strId()));
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
