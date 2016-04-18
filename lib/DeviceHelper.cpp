#include "../include/DeviceHelper.h"

bool DeviceHelper::getIntProperty(const Hardware::DeviceObject* device, const QString& name, int* value, OutputLog* log)
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


bool DeviceHelper::getStrProperty(const Hardware::DeviceObject* device, const QString& name, QString* value, OutputLog* log)
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


bool DeviceHelper::getBoolProperty(const Hardware::DeviceObject* device, const QString& name, bool* value, OutputLog* log)
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


void DeviceHelper::logPropertyNotFoundError(const QString& propertyName, const QString& deviceStrID, OutputLog* log)
{
	if (log == nullptr)
	{
		LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
				  QString(tr("Property '%1' is not found in device '%2'")).arg(propertyName).arg(deviceStrID));
		assert(false);
		return;
	}
}
