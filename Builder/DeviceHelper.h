#pragma once

#include <HardwareLib/DeviceObject.h>
#include <HardwareLib/EquipmentSet.h>

#include "../UtilsLib/OutputLog.h"

#include "IssueLogger.h"

class HostAddressPort;


class DeviceHelper : public QObject
{
	Q_OBJECT
public:
	static constexpr int LM_PLACE1 = 0;
	static constexpr int LM_PLACE2 = 15;

	static constexpr int BVB_PLACE1 = 0;
	static constexpr int BVB_PLACE2 = 13;

	static constexpr int MSO_PLACE1 = 0;
	static constexpr int MSO_PLACE2 = 73;		// MSO can query up to 72 I/O modules


public:
	static bool getIntProperty(const Hardware::DeviceObject* device, const QString& name, qint32* value, Builder::IssueLogger* log);
	static bool getUIntProperty(const Hardware::DeviceObject* device, const QString& name, quint32* value, Builder::IssueLogger *log);
	static bool getStrProperty(const Hardware::DeviceObject* device, const QString& name, QString *value, Builder::IssueLogger* log);
	static bool getStrListProperty(const Hardware::DeviceObject* device, const QString& name, QStringList* strList, Builder::IssueLogger* log);
	static bool getStrListPropertyAsString(const Hardware::DeviceObject* device, const QString& name, QString* str, Builder::IssueLogger* log);
	static bool getBoolProperty(const Hardware::DeviceObject* device, const QString& name, bool* value, Builder::IssueLogger* log);

	template<typename ENUM_TYPE>
	static bool getEnumValueProperty(const Hardware::DeviceObject* device, const QString& name, ENUM_TYPE* value, Builder::IssueLogger* log);

	static bool getIPv4Property(const Hardware::DeviceObject* device,
								const QString& name,
								QString* value,
								bool emptyAllowed,
								const QString& defaultIp,
								Builder::IssueLogger *log);

	static bool getIPv4Property(const Hardware::DeviceObject* device,
								const QString& name,
								QHostAddress* value,
								bool emptyAllowed,
								const QString& defaultIp,
								Builder::IssueLogger *log);

	static bool getPortProperty(const Hardware::DeviceObject* device,
								const QString& name,
								int* value,
								bool emptyAllowed,
								int defaultPort,
								Builder::IssueLogger* log);

	static bool getIPv4PortProperty(const Hardware::DeviceObject* device,
								  const QString& ipProperty,
								  const QString& portProperty,
								  HostAddressPort* ipPort,
								  bool emptyAllowed,
								  const QString& defaultIP,
								  int defaultPort,
								  Builder::IssueLogger* log);

	static bool isPropertyExists(const Hardware::DeviceObject* device, const QString& name);

	template<typename T>
	static bool getProperty(const Hardware::DeviceObject* device, const QString& name, T* value, Builder::IssueLogger* log);

	static bool setIntProperty(Hardware::DeviceObject* device, const QString& name, qint32 value, Builder::IssueLogger* log);
	static bool setUIntProperty(Hardware::DeviceObject* device, const QString& name, quint32 value, Builder::IssueLogger* log);

	static Hardware::DeviceObject* getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix);
	static Hardware::DeviceObject* getChildDeviceObjectBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log);

	static Hardware::DeviceController* getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix);
	static Hardware::DeviceController* getChildControllerBySuffix(const Hardware::DeviceObject* device, const QString& suffix, Builder::IssueLogger* log);

	static Hardware::DeviceController* getPlatformInterfaceController(const Hardware::DeviceModule* module, Builder::IssueLogger* log);

	static const Hardware::DeviceModule* getModuleOnPlace(const Hardware::DeviceModule* lm, int place);
	static const Hardware::DeviceModule* getLm(const Hardware::DeviceChassis* chassis);
	static const Hardware::DeviceModule* getLmBvbMso(const Hardware::DeviceChassis* chassis);

	static const Hardware::DeviceModule* getAssociatedLm(const Hardware::DeviceObject* object);
	static const Hardware::DeviceModule* getAssociatedLmBvbMso(const Hardware::DeviceObject* object);
	static std::shared_ptr<Hardware::DeviceModule> getParentVduModule(Hardware::DeviceObject *object);

	static const Hardware::Software* getSoftware(const Hardware::EquipmentSet* equipment, const QString& softwareID);
	static QStringList getSoftwareControllersIDs(const Hardware::Software* software);

	static void getChildDiagSignals(std::shared_ptr<const Hardware::DeviceObject> parent,
								   std::vector<std::shared_ptr<const Hardware::DiagSignal>>* diagSignals);

	static bool isTwoChannelSoftware(const Hardware::DeviceObject* swObject, QStringList* channelsControllersIds = nullptr);

private:
	static void logPropertyNotFoundError(const Hardware::DeviceObject* device, const QString& propertyName, Builder::IssueLogger* log);
	static void logPropertyWriteError(const Hardware::DeviceObject* device, const QString& propertyName, Builder::IssueLogger *log);

private:
//	static ModulesRawDataDescriptionMap m_modulesRawDataDescription;
};

template<typename ENUM_TYPE>
bool DeviceHelper::getEnumValueProperty(const Hardware::DeviceObject* device, const QString& name, ENUM_TYPE* value, Builder::IssueLogger* log)
{
	static_assert(std::is_enum<ENUM_TYPE>::value == true);

	int intValue = 0;

	bool result = getIntProperty(device, name, &intValue, log);

	if (result == false)
	{
		return false;
	}

	if (E::contains<ENUM_TYPE>(intValue) == false)
	{
		LOG_INTERNAL_ERROR_MSG(log, QString("Unknown enum value of property %1.%2").
							arg(device->equipmentIdTemplate()).arg(name));
		return false;
	}

	*value = static_cast<ENUM_TYPE>(intValue);

	return true;
}

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
