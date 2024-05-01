#pragma once
#include <memory>
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


	/// @brief The ScriptDeviceObject class represents a device object in the script.
	/// @details Instances of this class allow access to the device object properties and its children.
	/// 
	class ScriptDeviceObject : public QObject
	{
		Q_OBJECT

		/// @brief Device object identifier.
		Q_PROPERTY(QString equipmentId READ equipmentId)
		
		/// @brief Device object caption.
		Q_PROPERTY(QString caption READ caption)

		/// @brief Device object UUID.
		Q_PROPERTY(QUuid uuid READ uuid)

		/// @brief Device object type.
		Q_PROPERTY(int deviceType READ deviceType)

		/// @brief Device object place.
		Q_PROPERTY(int place READ place)

		/// @brief Number of children.
		Q_PROPERTY(int childrenCount READ childrenCount)

	public:
		ScriptDeviceObject(const std::shared_ptr<DeviceObject>& deviceObject, QObject* parent = nullptr);

		std::shared_ptr<const DeviceObject> deviceObject() const;
		std::shared_ptr<DeviceObject> deviceObject();

	public slots:
		/// @brief Returns the parent object of the device object.
		QJSValue parent();

		/// @brief Returns the child object at the specified index.
		QJSValue child(int index);

		/// @brief Returns the child object with the specified equipmentId.
		QJSValue childByEquipmentId(QString id);

		/// @brief Cast object to ScriptDeviceSystem, return null if object is not DeviceSystem.
		QJSValue toSystem();

		/// @brief Cast object to ScriptDeviceRack, return null if object is not DeviceRack.
		QJSValue toRack();

		/// @brief Cast object to ScriptDeviceChassis, return null if object is not DeviceChassis.
		QJSValue toChassis();

		/// @brief Cast object to ScriptDeviceModule, return null if object is not DeviceModule.
		QJSValue toModule();

		/// @brief Cast object to ScriptDeviceController, return null if object is not DeviceController.
		QJSValue toController();

		/// @brief Cast object to ScriptDeviceWorkstation, return null if object is not Workstation.
		QJSValue toWorkstation();

		/// @brief Cast object to ScriptDeviceSoftware, return null if object is not Software.
		QJSValue toSoftware();

		/// @brief Cast object to ScriptDeviceAppSignal, return null if object is not AppSignal.
		QJSValue toAppSignal();
		

		/// @brief Returns true if the object is a ScriptDeviceRoot object.
		bool isRoot() const;

		/// @brief Returns true if the object is a ScriptDeviceSystem object.
		bool isSystem() const;

		/// @brief Returns true if the object is a ScriptDeviceRack object.
		bool isRack() const;

		/// @brief Returns true if the object is a ScriptDeviceChassis object.
		bool isChassis() const;

		/// @brief Returns true if the object is a ScriptDeviceModule object.
		bool isModule() const;

		/// @brief Returns true if the object is a ScriptDeviceController object.
		bool isController() const;

		/// @brief Returns true if the object is a ScriptWorkstation object.
		bool isWorkstation() const;

		/// @brief Returns true if the object is a ScriptSoftware object.
		bool isSoftware() const;

		/// @brief Returns true if the object is a ScriptAppSignal object.
		bool isAppSignal() const;

		/// @brief Returns the value of the specified property.
		QVariant propertyValue(const QString& caption) const;

		int propertyInt(const QString& caption) const;
		bool propertyBool(const QString& caption) const;
		QString propertyString(const QString& caption) const;
		quint32 propertyIP(const QString& caption) const;

	public:
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
		Q_PROPERTY(int customModuleFamily READ customModuleFamily)
		Q_PROPERTY(int moduleVersion READ moduleVersion)

	public:
		ScriptDeviceModule(std::shared_ptr<DeviceModule> deviceModule, QObject* parent = nullptr);

	private:
		const Hardware::DeviceModule* module() const;
		Hardware::DeviceModule* module();

		int moduleFamily() const;
		int customModuleFamily() const;
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
		ScriptDeviceAppSignal(const std::shared_ptr<DeviceAppSignal>& deviceAppSignal, QObject* parent = nullptr);

	private:
		const Hardware::DeviceAppSignal* appSignal() const;
		Hardware::DeviceAppSignal* appSignal();
	};

}
