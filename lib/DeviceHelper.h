#pragma once

#include "../lib/DeviceObject.h"
#include "../lib/OutputLog.h"
#include "../u7/Builder/IssueLogger.h"
#include "../u7/Builder/ModulesRawData.h"


class DeviceHelper : public QObject
{
	Q_OBJECT

public:
	struct IntPropertyNameVar
	{
		const char* name = nullptr;
		int* var = nullptr;

		IntPropertyNameVar(const char* n, int* v) : name(n), var(v) {}
	};


	struct StrPropertyNameVar
	{
		const char* name = nullptr;
		QString* var = nullptr;

		StrPropertyNameVar(const char* n, QString* v) : name(n), var(v) {}
	};

	static QString LM_PLATFORM_INTERFACE_CONTROLLER_SUFFUX;

public:
	static void init();
	static void shutdown();

	static bool getIntProperty(const Hardware::DeviceObject* device, const QString& name, int* value, Builder::IssueLogger* log);
	static bool getStrProperty(const Hardware::DeviceObject* device, const QString& name, QString *value, Builder::IssueLogger* log);
	static bool getBoolProperty(const Hardware::DeviceObject* device, const QString& name, bool* value, Builder::IssueLogger* log);

	template<typename T>
	static bool getProperty(const Hardware::DeviceObject* device, const QString& name, T* value, Builder::IssueLogger* log);

	static bool setIntProperty(Hardware::DeviceObject* device, const QString& name, int value, Builder::IssueLogger* log);

	static Hardware::DeviceObject* getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix);
	static Hardware::DeviceObject* getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log);

	static Hardware::DeviceController* getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix);
	static Hardware::DeviceController* getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log);

	static Hardware::DeviceController* getPlatformInterfaceController(const Hardware::DeviceModule* module, Builder::IssueLogger* log);

	static const Hardware::DeviceModule* getModuleOnPlace(const Hardware::DeviceModule* lm, int place);
	static const Hardware::DeviceModule* getLm(const Hardware::DeviceChassis* chassis);
	static const Hardware::DeviceModule* getLmOrBvb(const Hardware::DeviceChassis* chassis);

	static const Hardware::DeviceModule* getAssociatedLm(const Hardware::DeviceObject* object);
	static const Hardware::DeviceModule* getAssociatedLmOrBvb(const Hardware::DeviceObject* object);

	static int getAllNativeRawDataSize(const Hardware::DeviceModule* lm, Builder::IssueLogger* log);
	static int getModuleRawDataSize(const Hardware::DeviceModule* lm, int modulePlace, bool* moduleIsFound, Builder::IssueLogger* log);
	static int getModuleRawDataSize(const Hardware::DeviceModule* module, Builder::IssueLogger* log);

	static ModuleRawDataDescription* getModuleRawDataDescription(const Hardware::DeviceModule* module);

private:
	static void logPropertyNotFoundError(const Hardware::DeviceObject* device, const QString& propertyName, Builder::IssueLogger* log);
	static void logPropertyWriteError(const Hardware::DeviceObject* device, const QString& propertyName, Builder::IssueLogger *log);

private:
	static QHash<QString, ModuleRawDataDescription*> m_modulesRawDataDescription;
};

template<typename T>
bool DeviceHelper::getProperty(const Hardware::DeviceObject* device, const QString& name, T* value, Builder::IssueLogger* log)
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

	if (val.canConvert<T>() == false)
	{
		assert(false);

		// Property '%1.%2' conversion error.
		//
		log->errCFG3023(device->equipmentIdTemplate(), name);

		return false;
	}

	*value = val.value<T>();

	return true;
}
