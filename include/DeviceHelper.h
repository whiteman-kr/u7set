#pragma once

#include "../include/DeviceObject.h"
#include "../include/OutputLog.h"
#include "../u7/Builder/IssueLogger.h"


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


private:
	static void logPropertyNotFoundError(const QString& propertyName, const QString& deviceStrID, Builder::IssueLogger* log);

public:
	static bool getIntProperty(const Hardware::DeviceObject* device, const QString& name, int* value, Builder::IssueLogger* log);
	static bool getStrProperty(const Hardware::DeviceObject* device, const QString& name, QString *value, Builder::IssueLogger* log);
	static bool getBoolProperty(const Hardware::DeviceObject* device, const QString& name, bool* value, Builder::IssueLogger* log);

	static Hardware::DeviceObject* getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix);
	static Hardware::DeviceObject* getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log);

	static Hardware::DeviceController *getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix);
	static Hardware::DeviceController *getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log);
};
