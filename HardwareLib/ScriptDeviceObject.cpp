#ifndef HARDWARE_LIB_DOMAIN
#error Don't include this file in the project! Link HardwareLib instead.
#endif

#include "ScriptDeviceObject.h"
#include <QHostAddress>
#include "DeviceObject.h"

namespace Hardware
{
	ScriptDeviceObject::ScriptDeviceObject(std::shared_ptr<DeviceObject> deviceObject, QObject* parent /*= nullptr*/) :
		QObject(parent),
		m_deviceObject(deviceObject)
	{
		assert(deviceObject);
	}

	std::shared_ptr<const DeviceObject> ScriptDeviceObject::deviceObject() const
	{
		return m_deviceObject;
	}

	std::shared_ptr<DeviceObject> ScriptDeviceObject::deviceObject()
	{
		return m_deviceObject;
	}

	QJSValue ScriptDeviceObject::parent()
	{
		if (m_deviceObject->hasParent() == false)
		{
			return {};
		}

		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		return engine->newQObject(new ScriptDeviceObject(m_deviceObject->parent()));
	}

	QJSValue ScriptDeviceObject::child(int index)
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (index < 0 || index >= m_deviceObject->childrenCount())
		{
			engine->throwError(tr("ScriptDeviceObject %1 does not have child with index %2")
							   .arg(m_deviceObject->equipmentId())
							   .arg(index));
			return {};
		}

		return engine->newQObject(new ScriptDeviceObject(m_deviceObject->child(index)));
	}

	QJSValue ScriptDeviceObject::childByEquipmentId(QString id)
	{
		auto child = m_deviceObject->childByEquipmentId(id);
		if (child == nullptr)
		{
			return {};
		}

		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		return engine->newQObject(new ScriptDeviceObject(child));
	}

	QJSValue ScriptDeviceObject::toSystem()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isSystem() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceSystem from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto s = new ScriptDeviceSystem(m_deviceObject->toSystem());
		assert(s);

		return engine->newQObject(s);
	}

	QJSValue ScriptDeviceObject::toRack()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isRack() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceRack from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto r = new ScriptDeviceRack(m_deviceObject->toRack());
		assert(r);

		return engine->newQObject(r);
	}

	QJSValue ScriptDeviceObject::toChassis()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isChassis() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceChassis from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto ch = new ScriptDeviceChassis(m_deviceObject->toChassis());
		assert(ch);

		return engine->newQObject(ch);
	}

	QJSValue ScriptDeviceObject::toModule()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isModule() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceModule from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto m = new ScriptDeviceModule(m_deviceObject->toModule());
		assert(m);

		return engine->newQObject(m);
	}

	QJSValue ScriptDeviceObject::toController()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isController() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceController from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto c = new ScriptDeviceController(m_deviceObject->toController());
		assert(c);

		return engine->newQObject(c);
	}

	QJSValue ScriptDeviceObject::toWokstation()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isWorkstation() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceWorkstation from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto ws = new ScriptDeviceWorkstation(m_deviceObject->toWorkstation());
		assert(ws);

		return engine->newQObject(ws);
	}

	QJSValue ScriptDeviceObject::toSoftware()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isSoftware() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceSoftware from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto sw = new ScriptDeviceSoftware(m_deviceObject->toSoftware());
		assert(sw);

		return engine->newQObject(sw);
	}

	QJSValue ScriptDeviceObject::toAppSignal()
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return {};
		}

		if (m_deviceObject->isAppSignal() == false)
		{
			engine->throwError(tr("Cannot construct ScriptDeviceAppSignal from %1 (%2)").arg(m_deviceObject->equipmentId()).arg(m_deviceObject->metaObject()->className()));
			return {};
		}

		auto as = new ScriptDeviceAppSignal(m_deviceObject->toAppSignal());
		assert(as);

		return engine->newQObject(as);
	}

	bool ScriptDeviceObject::isRoot() const
	{
		return m_deviceObject->isRoot();
	}

	bool ScriptDeviceObject::isSystem() const
	{
		return m_deviceObject->isSystem();
	}

	bool ScriptDeviceObject::isRack() const
	{
		return m_deviceObject->isRack();
	}

	bool ScriptDeviceObject::isChassis() const
	{
		return m_deviceObject->isChassis();
	}

	bool ScriptDeviceObject::isModule() const
	{
		return m_deviceObject->isModule();
	}

	bool ScriptDeviceObject::isController() const
	{
		return m_deviceObject->isController();
	}

	bool ScriptDeviceObject::isWorkstation() const
	{
		return m_deviceObject->isWorkstation();
	}

	bool ScriptDeviceObject::isSoftware() const
	{
		return m_deviceObject->isSoftware();
	}

	bool ScriptDeviceObject::isAppSignal() const
	{
		return m_deviceObject->isAppSignal();
	}

	QVariant ScriptDeviceObject::propertyValue(const QString& caption) const
	{
		return m_deviceObject->propertyValue(caption);
	}

	int ScriptDeviceObject::propertyInt(const QString& caption) const
	{
		return propertyValue(caption).toInt();
	}

	bool ScriptDeviceObject::propertyBool(const QString& caption) const
	{
		return propertyValue(caption).toBool();
	}

	QString ScriptDeviceObject::propertyString(const QString& caption) const
	{
		return propertyValue(caption).toString();
	}

	quint32 ScriptDeviceObject::propertyIP(const QString& caption) const
	{
		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			assert(engine);
			return 0;
		}

		QString s = propertyString(caption);
		bool ok = false;
		quint32 result = QHostAddress{s}.toIPv4Address(&ok);

		if (ok == false)
		{
			engine->throwError(tr("Device %1 cannot convert property %2 to IP address (value %3).")
							   .arg(m_deviceObject->equipmentId())
							   .arg(caption)
							   .arg(s));
			return 0;
		}

		return result;
	}

	QString ScriptDeviceObject::equipmentId() const
	{
		return m_deviceObject->equipmentId();
	}

	QString ScriptDeviceObject::caption() const
	{
		return m_deviceObject->caption();
	}

	QUuid ScriptDeviceObject::uuid() const
	{
		return m_deviceObject->uuid();
	}

	int ScriptDeviceObject::childrenCount() const
	{
		return m_deviceObject->childrenCount();
	}

	int ScriptDeviceObject::deviceType() const
	{
		return static_cast<int>(m_deviceObject->deviceType());
	}

	int ScriptDeviceObject::place() const
	{
		return m_deviceObject->place();
	}

	//
	// System
	//
	ScriptDeviceSystem::ScriptDeviceSystem(std::shared_ptr<DeviceSystem> deviceSystem, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceSystem, parent)
	{
		assert(deviceSystem);
	}

	const Hardware::DeviceSystem* ScriptDeviceSystem::system() const
	{
		const Hardware::DeviceSystem* result = m_deviceObject->toSystem().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceSystem.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::DeviceSystem* ScriptDeviceSystem::system()
	{
		Hardware::DeviceSystem* result = m_deviceObject->toSystem().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceSystem.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	//
	// Rack
	//
	ScriptDeviceRack::ScriptDeviceRack(std::shared_ptr<DeviceRack> deviceRack, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceRack, parent)
	{
		assert(deviceRack);
	}

	const Hardware::DeviceRack* ScriptDeviceRack::rack() const
	{
		const Hardware::DeviceRack* result = m_deviceObject->toRack().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceRack.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::DeviceRack* ScriptDeviceRack::rack()
	{
		Hardware::DeviceRack* result = m_deviceObject->toRack().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceRack.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}


	//
	// Chassis
	//
	ScriptDeviceChassis::ScriptDeviceChassis(std::shared_ptr<DeviceChassis> deviceChassis, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceChassis, parent)
	{
		assert(deviceChassis);
	}

	const Hardware::DeviceChassis* ScriptDeviceChassis::chassis() const
	{
		const Hardware::DeviceChassis* result = m_deviceObject->toChassis().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceChassis.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::DeviceChassis* ScriptDeviceChassis::chassis()
	{
		Hardware::DeviceChassis* result = m_deviceObject->toChassis().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceChassis.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}


	//
	// Module
	//
	ScriptDeviceModule::ScriptDeviceModule(std::shared_ptr<DeviceModule> deviceModule, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceModule, parent)
	{
		assert(deviceModule);
	}

	const Hardware::DeviceModule* ScriptDeviceModule::module() const
	{
		const Hardware::DeviceModule* result = m_deviceObject->toModule().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceModule.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::DeviceModule* ScriptDeviceModule::module()
	{
		Hardware::DeviceModule* result = m_deviceObject->toModule().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceModule.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	int ScriptDeviceModule::moduleFamily() const
	{
		auto m = module();
		return m ? static_cast<int>(m->moduleFamily()) : -1;
	}

	int ScriptDeviceModule::customModuleFamily() const
	{
		auto m = module();
		return m ? static_cast<int>(m->customModuleFamily()) : -1;
	}

	int ScriptDeviceModule::moduleVersion() const
	{
		auto m = module();
		return m ? m->moduleVersion() : -1;
	}


	//
	// Controller
	//
	ScriptDeviceController::ScriptDeviceController(std::shared_ptr<DeviceController> deviceController, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceController, parent)
	{
		assert(deviceController);
	}

	const Hardware::DeviceController* ScriptDeviceController::controller() const
	{
		const Hardware::DeviceController* result = m_deviceObject->toController().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceController.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::DeviceController* ScriptDeviceController::controller()
	{
		Hardware::DeviceController* result = m_deviceObject->toController().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceController.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}


	//
	// Workstation
	//
	ScriptDeviceWorkstation::ScriptDeviceWorkstation(std::shared_ptr<Hardware::Workstation> deviceWorkstation, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceWorkstation, parent)
	{
		assert(deviceWorkstation);
	}

	const Hardware::Workstation* ScriptDeviceWorkstation::workstation() const
	{
		const Hardware::Workstation* result = m_deviceObject->toWorkstation().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceWorkstation.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::Workstation* ScriptDeviceWorkstation::workstation()
	{
		Hardware::Workstation* result = m_deviceObject->toWorkstation().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceWorkstation.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}


	//
	// Software
	//
	ScriptDeviceSoftware::ScriptDeviceSoftware(std::shared_ptr<Software> deviceSoftware, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceSoftware, parent)
	{
		assert(deviceSoftware);
	}

	const Hardware::Software* ScriptDeviceSoftware::software() const
	{
		const Hardware::Software* result = m_deviceObject->toSoftware().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceSoftware.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::Software* ScriptDeviceSoftware::software()
	{
		Hardware::Software* result = m_deviceObject->toSoftware().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceSoftware.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	int ScriptDeviceSoftware::softwareType() const
	{
		const Software* s = software();
		return s ? s->softwareType() : -1;
	}


	//
	// AppSignal
	//
	ScriptDeviceAppSignal::ScriptDeviceAppSignal(std::shared_ptr<DeviceAppSignal> deviceAppSignal, QObject* parent /*= nullptr*/) :
		ScriptDeviceObject(deviceAppSignal, parent)
	{
		assert(deviceAppSignal);
	}

	const Hardware::DeviceAppSignal* ScriptDeviceAppSignal::appSignal() const
	{
		const Hardware::DeviceAppSignal* result = m_deviceObject->toAppSignal().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceAppSignal.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}

	Hardware::DeviceAppSignal* ScriptDeviceAppSignal::appSignal()
	{
		Hardware::DeviceAppSignal* result = m_deviceObject->toAppSignal().get();
		if (result == nullptr)
		{
			assert(result);

			QJSEngine* engine = qjsEngine(this);
			if (engine == nullptr)
			{
				assert(engine);
				return result;
			}

			engine->throwError(tr("Device %1 cannot be converted to ScriptDeviceAppSignal.").arg(m_deviceObject->equipmentId()));
		}

		return result;
	}


}
