#pragma once
#include <memory>
#include <QJSEngine>
#include <QUuid>

namespace Hardware
{
	class DeviceObject;
	class DeviceRoot;
	class DeviceSystem;
	class DeviceRack;
	class DeviceChassis;
	class DeviceModule;
	class DeviceController;
	class Workstation;
	class Software;
	class DeviceAppSignal;
	class DeviceDiagSignal;


	//
	// DeviceObject script wrapper
	//
	class ScriptDeviceObject : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString equipmentId READ equipmentId)
		Q_PROPERTY(QString caption READ caption)
		Q_PROPERTY(QUuid uuid READ uuid)
		Q_PROPERTY(int deviceType READ deviceType)
		Q_PROPERTY(int place READ place)
		Q_PROPERTY(int childrenCount READ childrenCount)

	public:
		ScriptDeviceObject(std::shared_ptr<DeviceObject> deviceObject, QObject* parent = nullptr);

		std::shared_ptr<const DeviceObject> deviceObject() const;
		std::shared_ptr<DeviceObject> deviceObject();

	public slots:
		QJSValue parent();
		QJSValue child(int index);
		QJSValue childByEquipmentId(QString id);

		QJSValue toSystem();
		QJSValue toRack();
		QJSValue toChassis();
		QJSValue toModule();
		QJSValue toController();
		QJSValue toWokstation();
		QJSValue toSoftware();
		QJSValue toAppSignal();

		bool isRoot() const;
		bool isSystem() const;
		bool isRack() const;
		bool isChassis() const;
		bool isModule() const;
		bool isController() const;
		bool isWorkstation() const;
		bool isSoftware() const;
		bool isAppSignal() const;

		// Properties
		//
		QVariant propertyValue(const QString& caption) const;

		int propertyInt(const QString& caption) const;
		bool propertyBool(const QString& caption) const;
		QString propertyString(const QString& caption) const;
		quint32 propertyIP(const QString& caption) const;

	private:
		QString equipmentId() const;
		QString caption() const;
		QUuid uuid() const;

		int childrenCount() const;

		int deviceType() const;
		int place() const;

	protected:
		std::shared_ptr<DeviceObject> m_deviceObject;
	};


	//
	// System
	//
	class ScriptDeviceSystem : public ScriptDeviceObject
	{
		Q_OBJECT

	public:
		ScriptDeviceSystem(std::shared_ptr<DeviceSystem> deviceSystem, QObject* parent = nullptr);

	private:
		const Hardware::DeviceSystem* system() const;
		Hardware::DeviceSystem* system();
	};


	//
	// Rack
	//
	class ScriptDeviceRack : public ScriptDeviceObject
	{
		Q_OBJECT

	public:
		ScriptDeviceRack(std::shared_ptr<DeviceRack> deviceRack, QObject* parent = nullptr);

	private:
		const Hardware::DeviceRack* rack() const;
		Hardware::DeviceRack* rack();
	};

	//
	// Chassis
	//
	class ScriptDeviceChassis : public ScriptDeviceObject
	{
		Q_OBJECT

	public:
		ScriptDeviceChassis(std::shared_ptr<DeviceChassis> deviceChassis, QObject* parent = nullptr);

	private:
		const Hardware::DeviceChassis* chassis() const;
		Hardware::DeviceChassis* chassis();
	};


	//
	// Module
	//
	class ScriptDeviceModule : public ScriptDeviceObject
	{
		Q_OBJECT

		Q_PROPERTY(int moduleFamily READ moduleFamily)
		Q_PROPERTY(int moduleVersion READ moduleVersion)

	public:
		ScriptDeviceModule(std::shared_ptr<DeviceModule> deviceModule, QObject* parent = nullptr);

	private:
		const Hardware::DeviceModule* module() const;
		Hardware::DeviceModule* module();

		int moduleFamily() const;
		int moduleVersion() const;
	};


	//
	// Controller
	//
	class ScriptDeviceController : public ScriptDeviceObject
	{
		Q_OBJECT

	public:
		ScriptDeviceController(std::shared_ptr<DeviceController> deviceController, QObject* parent = nullptr);

	private:
		const Hardware::DeviceController* controller() const;
		Hardware::DeviceController* controller();
	};


	//
	// Workstation
	//
	class ScriptDeviceWorkstation : public ScriptDeviceObject
	{
		Q_OBJECT

	public:
		ScriptDeviceWorkstation(std::shared_ptr<Workstation> deviceWorkstation, QObject* parent = nullptr);

	private:
		const Hardware::Workstation* workstation() const;
		Hardware::Workstation* workstation();
	};


	//
	// Software
	//
	class ScriptDeviceSoftware : public ScriptDeviceObject
	{
		Q_OBJECT

		Q_PROPERTY(int softwareType READ softwareType)

	public:
		ScriptDeviceSoftware(std::shared_ptr<Software> deviceSoftware, QObject* parent = nullptr);

	private:
		const Hardware::Software* software() const;
		Hardware::Software* software();

		int softwareType() const;
	};


	//
	// AppSignal
	//
	class ScriptDeviceAppSignal : public ScriptDeviceObject
	{
		Q_OBJECT

	public:
		ScriptDeviceAppSignal(std::shared_ptr<DeviceAppSignal> deviceAppSignal, QObject* parent = nullptr);

	private:
		const Hardware::DeviceAppSignal* appSignal() const;
		Hardware::DeviceAppSignal* appSignal();
	};

}
