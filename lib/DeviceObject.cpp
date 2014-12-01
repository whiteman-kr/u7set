#include "../include/DeviceObject.h"
#include "../include/ProtoSerialization.h"
#include <QDynamicPropertyChangeEvent>


namespace Hardware
{
	//const static wchar_t* DeviceObjectExtensions = {L".hroot"}, {L".hsystem"};

	Factory<Hardware::DeviceObject> DeviceObjectFactory;

	void Init()
	{
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRoot>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceSystem>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRack>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceChassis>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceModule>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceController>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceDiagSignal>();
	}

	void Shutdwon()
	{
	}

	//
	//
	// DeviceObject
	//
	//
	DeviceObject::DeviceObject(bool preset /*= false*/) :
		m_preset(preset)
	{
	}

	DeviceObject::~DeviceObject()
	{
	}

	DeviceObject* DeviceObject::fromDbFile(const DbFile& file)
	{
		DeviceObject* object = DeviceObject::Create(file.data());

		if (object == nullptr)
		{
			assert(object != nullptr);
			return nullptr;
		}

		object->setFileInfo(file);
		return object;
	}

	bool DeviceObject::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		Proto::DeviceObject* pMutableDeviceObject = message->mutable_deviceobject();

		Proto::Write(pMutableDeviceObject->mutable_uuid(), m_uuid);
		Proto::Write(pMutableDeviceObject->mutable_strid(), m_strId);
		Proto::Write(pMutableDeviceObject->mutable_caption(), m_caption);

		pMutableDeviceObject->set_place(m_place);

		if (m_childRestriction.isEmpty() == false)
		{
			Proto::Write(pMutableDeviceObject->mutable_childrestriction(), m_childRestriction);
		}

		if (m_preset == true)
		{
			pMutableDeviceObject->set_preset(m_preset);

			pMutableDeviceObject->set_presetroot(m_presetRoot);
			Proto::Write(pMutableDeviceObject->mutable_presetname(), m_presetName);
		}

		return true;
	}

	bool DeviceObject::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		const Proto::DeviceObject& deviceobject = message.deviceobject();

		m_uuid = Proto::Read(deviceobject.uuid());
		Proto::Read(deviceobject.strid(), &m_strId);
		Proto::Read(deviceobject.caption(), &m_caption);
		m_place = deviceobject.place();

		if (deviceobject.has_childrestriction() == true)
		{
			Proto::Read(deviceobject.childrestriction(), &m_childRestriction);
		}
		else
		{
			m_childRestriction.clear();
		}

		if (deviceobject.has_preset() == true && deviceobject.preset() == true)
		{
			m_preset = deviceobject.preset();

			if (deviceobject.has_presetroot() == true)
			{
				m_presetRoot = deviceobject.presetroot();
			}
			else
			{
				assert(deviceobject.has_presetroot() == true);
			}

			if (deviceobject.has_presetname() == true)
			{
				Proto::Read(deviceobject.presetname(), &m_presetName);
			}
			else
			{
				assert(deviceobject.has_presetname());
			}
		}

		return true;
	}

	DeviceObject* DeviceObject::CreateObject(const Proto::Envelope& message)
	{
		// This func can create only one instance
		//
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		DeviceObject* pDeviceObject = DeviceObjectFactory.Create(classNameHash);

		if (pDeviceObject == nullptr)
		{
			assert(pDeviceObject);
			return nullptr;
		}

		pDeviceObject->LoadData(message);

		return pDeviceObject;
	}

	DeviceObject* DeviceObject::parent()
	{
		return m_parent;
	}

	DeviceType DeviceObject::deviceType() const
	{
		assert(false);
		return DeviceType::Root;
	}

	QString DeviceObject::fileExtension() const
	{
		int index = static_cast<int>(deviceType());
		assert(index >= 0 && index < sizeof(DeviceObjectExtensions) / sizeof(DeviceObjectExtensions[0]));

		QString result = QString::fromWCharArray(DeviceObjectExtensions[index]);
		return result;
	}

	QString DeviceObject::fileExtension(DeviceType device)
	{
		QString result = QString::fromWCharArray(DeviceObjectExtensions[static_cast<int>(device)]);
		return result;
	}

	int DeviceObject::childrenCount() const
	{
		return static_cast<int>(m_children.size());
	}

	DeviceObject* DeviceObject::child(int index) const
	{
		return m_children.at(index).get();
	}

	int DeviceObject::childIndex(DeviceObject* child) const
	{
		auto fr = std::find_if(m_children.begin(), m_children.end(),
							   [child](const std::shared_ptr<DeviceObject>& v)
		{
			return v.get() == child;
		});

		if (fr == m_children.end())
		{
			return -1;
		}

		return std::distance(m_children.begin(), fr);
	}

	std::shared_ptr<DeviceObject> DeviceObject::childSharedPtr(int index)
	{
		std::shared_ptr<DeviceObject> sp = m_children.at(index);
		return sp;
	}

	void DeviceObject::addChild(std::shared_ptr<DeviceObject> child)
	{
		if (deviceType() >= child->deviceType())
		{
			assert(deviceType() < child->deviceType());
			return;
		}

		child->m_parent = this;
		m_children.push_back(child);
	}

	void DeviceObject::deleteChild(DeviceObject* child)
	{
		auto found = std::find_if(m_children.begin(), m_children.end(), [child](decltype(m_children)::const_reference c)
			{
				return c.get() == child;
			});

		if (found == m_children.end())
		{
			assert(found != m_children.end());
			return;
		}

		m_children.erase(found);
		return;
	}

	void DeviceObject::deleteAllChildren()
	{
		m_children.clear();
	}

	bool DeviceObject::checkChild(DeviceObject* child, QString* errorMessage)
	{
		if (child == nullptr ||
			errorMessage == nullptr)
		{
			assert(child);
			assert(errorMessage);
			return false;
		}

		// Check device level
		//
		if (deviceType() > child->deviceType())
		{
			*errorMessage = tr("Childer device level must be lower that perents.");
			return false;
		}

		// Assume that an empty script is true. It will allow to save memory for modules, controllers...
		//
		if (m_childRestriction.isEmpty() == true)
		{
			return true;
		}

		// Create a copy of child, will be deleted by jsEngine on destroying.
		// It is because QJSEngine::newQObject makes value with JavaEngine ownership
		// and it cannot be changed (((
		// In QQmlEngine (derived from QJSEngine) it can be changed by calling QQmlEngine::setObjectOwnership(???, QQmlEngine::CppOwnership);
		// also, QScriptEngine::QtOwnership can help.
		// Hopefully in future Qt releases it might be changed, that ownership can be chnged from QJSEngine or QJSValue.
		//

		//QByteArray data;
		//child->Save(data);

		//DeviceObject* childCopy = DeviceObject::Create(data);
		//data.clear();

		// Run m_childRestriction script
		//
		QJSEngine jsEngine;

		//QJSValue arg = jsEngine.newQObject(childCopy);
		QJSValue arg = jsEngine.newQObject(child);
		QQmlEngine::setObjectOwnership(child, QQmlEngine::CppOwnership);

		QJSValue function = jsEngine.evaluate(m_childRestriction);
		QJSValue result = function.call(QJSValueList() << arg);

		if (result.isError() == true)
		{
			*errorMessage = tr("Script error:").arg(result.toString());
			return false;
		}

		bool boolResult = result.toBool();
		if (boolResult == false)
		{
			*errorMessage = tr("DeviceObject is not allowed.");
		}

		return boolResult;
	}

	void DeviceObject::sortChildrenByPlace()
	{
		std::sort(std::begin(m_children), std::end(m_children),
			[](const std::shared_ptr<DeviceObject>& o1, const std::shared_ptr<DeviceObject>& o2)
			{
				return o1->m_place < o2->m_place;
			});

		return;
	}

	const QString& DeviceObject::strId() const
	{
		return m_strId;
	}

	void DeviceObject::setStrId(const QString& value)
	{
		m_strId = value;
	}

	const QString& DeviceObject::caption() const
	{
		return m_caption;
	}

	void DeviceObject::setCaption(const QString& value)
	{
		m_caption = value;
	}

	DbFileInfo& DeviceObject::fileInfo()
	{
		return m_fileInfo;
	}

	const DbFileInfo& DeviceObject::fileInfo() const
	{
		return m_fileInfo;
	}

	void DeviceObject::setFileInfo(const DbFileInfo& value)
	{
		m_fileInfo = value;
	}

	const QString& DeviceObject::childRestriction() const
	{
		return m_childRestriction;
	}

	void DeviceObject::setChildRestriction(const QString& value)
	{
		m_childRestriction = value;
	}

	int DeviceObject::place() const
	{
		return m_place;
	}

	void DeviceObject::setPlace(int value)
	{
		m_place = value;
	}

	bool DeviceObject::preset() const
	{
		return m_preset;
	}

	bool DeviceObject::presetRoot() const
	{
		assert(m_preset == true);
		return m_presetRoot;
	}

	void DeviceObject::setPresetRoot(bool value)
	{
		m_presetRoot = value;
	}

	const QString& DeviceObject::presetName() const
	{
		assert(m_preset == true);
		return m_presetName;
	}

	void DeviceObject::setPresetName(const QString& value)
	{
		m_presetName = value;
	}

	//
	//
	// DeviceRoot
	//
	//
	DeviceRoot::DeviceRoot(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceRoot::~DeviceRoot()
	{
	}

	DeviceType DeviceRoot::deviceType() const
	{
		return m_deviceType;
	}


	//
	//
	// DeviceSystem
	//
	//
	DeviceSystem::DeviceSystem(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceSystem::~DeviceSystem()
	{
		qDebug() << Q_FUNC_INFO;
	}

	bool DeviceSystem::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceSystem* systemMessage =
				message->mutable_deviceobject()->mutable_system();

		Q_UNUSED(systemMessage);
		//systemMessage->set_startxdocpt(m_startXDocPt);
		//systemMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceSystem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_system() == false)
		{
			assert(message.deviceobject().has_system());
			return false;
		}

		const Proto::DeviceSystem& systemMessage = message.deviceobject().system();

		Q_UNUSED(systemMessage);
		//m_startXDocPt = systemMessage.startxdocpt();
		//m_startYDocPt = systemMessage.startydocpt();

		return true;
	}

	DeviceType DeviceSystem::deviceType() const
	{
		return m_deviceType;
	}


	//
	//
	// DeviceRack
	//
	//
	DeviceRack::DeviceRack(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceRack::~DeviceRack()
	{
	}

	bool DeviceRack::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceRack* rackMessage =
				message->mutable_deviceobject()->mutable_rack();

		Q_UNUSED(rackMessage);
		//rackMessage->set_startxdocpt(m_startXDocPt);
		//rackMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceRack::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_rack() == false)
		{
			assert(message.deviceobject().has_rack());
			return false;
		}

		const Proto::DeviceRack& rackMessage = message.deviceobject().rack();

		Q_UNUSED(rackMessage);
		//x = rackMessage.startxdocpt();
		//y = rackMessage.startydocpt();

		return true;
	}

	DeviceType DeviceRack::deviceType() const
	{
		return m_deviceType;
	}

	//
	//
	// DeviceChassis
	//
	//
	DeviceChassis::DeviceChassis(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceChassis::~DeviceChassis()
	{
	}

	bool DeviceChassis::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceChassis* chassisMessage = message->mutable_deviceobject()->mutable_chassis();

		chassisMessage->set_type(m_type);

		return true;
	}

	bool DeviceChassis::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_chassis() == false)
		{
			assert(message.deviceobject().has_chassis());
			return false;
		}

		const Proto::DeviceChassis& chassisMessage = message.deviceobject().chassis();

		m_type =  chassisMessage.type();

		return true;
	}

	DeviceType DeviceChassis::deviceType() const
	{
		return m_deviceType;
	}

	int DeviceChassis::type() const
	{
		return m_type;
	}

	void DeviceChassis::setType(int value)
	{
		m_type = value;
	}

	//
	//
	// DeviceModule
	//
	//
	DeviceModule::DeviceModule(bool preset /*= false*/) :
		DeviceObject(preset)
	{

	}

	DeviceModule::~DeviceModule()
	{
	}

	bool DeviceModule::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceModule* moduleMessage = message->mutable_deviceobject()->mutable_module();

		moduleMessage->set_type(m_type);

		if (m_moduleConfiguration.hasConfiguration() == true)
		{
			m_moduleConfiguration.save(moduleMessage->mutable_module_configuration());
		}

		return true;
	}

	bool DeviceModule::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_module() == false)
		{
			assert(message.deviceobject().has_module());
			return false;
		}

		const Proto::DeviceModule& moduleMessage = message.deviceobject().module();

		m_type =  moduleMessage.type();

		if (moduleMessage.has_module_configuration() == true)
		{
			m_moduleConfiguration.load(moduleMessage.module_configuration());

			// Set configuration user properties to Qt meta system
			//
			m_moduleConfiguration.addUserPropertiesToObject(this);
		}

		return true;
	}

	DeviceType DeviceModule::deviceType() const
	{
		return m_deviceType;
	}

	bool DeviceModule::event(QEvent* e)
	{
		if (e->type() == QEvent::DynamicPropertyChange)
		{
			// Configuration property was changed
			//
			 QDynamicPropertyChangeEvent* d = dynamic_cast<QDynamicPropertyChangeEvent*>(e);
			 assert(d != nullptr);

			QString propertyName = d->propertyName();

			QVariant value = this->property(propertyName.toStdString().c_str());

			if (value.isValid() == true)
			{
				m_moduleConfiguration.setUserProperty(propertyName, value);
			}

			// Accept event
			//
			return true;
		}

		// Event was not recognized
		//
		return false;
	}

	int DeviceModule::type() const
	{
		return m_type;
	}

	void DeviceModule::setType(int value)
	{
		m_type = value;
	}

	QString DeviceModule::configurationStruct() const
	{
		QString s = QString::fromStdString(m_moduleConfiguration.structDescription());
		return s;
	}

	void DeviceModule::setConfigurationStruct(const QString& value)
	{
		m_moduleConfiguration.setHasConfiguration(true);
		m_moduleConfiguration.setStructDescription(value.toStdString());

		m_moduleConfiguration.readStructure(value.toStdString().data());
		m_moduleConfiguration.addUserPropertiesToObject(this);
	}

	//
	//
	// DeviceController
	//
	//
	DeviceController::DeviceController(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceController::~DeviceController()
	{
	}

	bool DeviceController::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceController* controllerMessage =
				message->mutable_deviceobject()->mutable_controller();

		Q_UNUSED(controllerMessage);
		//controllerMessage->set_startxdocpt(m_startXDocPt);
		//controllerMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceController::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_controller() == false)
		{
			assert(message.deviceobject().has_controller());
			return false;
		}

		const Proto::DeviceController& controllerMessage = message.deviceobject().controller();

		Q_UNUSED(controllerMessage);
		//x = controllerMessage.startxdocpt();
		//y = controllerMessage.startydocpt();

		return true;
	}

	DeviceType DeviceController::deviceType() const
	{
		return m_deviceType;
	}

	//
	//
	// DeviceDiagSignal
	//
	//
	DeviceDiagSignal::DeviceDiagSignal(bool preset /*= false*/) :
		DeviceObject(preset)
	{
	}

	DeviceDiagSignal::~DeviceDiagSignal()
	{
	}

	bool DeviceDiagSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = DeviceObject::SaveData(message);
		if (result == false || message->has_deviceobject() == false)
		{
			assert(result);
			assert(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceDiagSignal* signalMessage =
				message->mutable_deviceobject()->mutable_diagsignal();

		Q_UNUSED(signalMessage);
		//signalMessage->set_startxdocpt(m_startXDocPt);
		//signalMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceDiagSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			assert(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_diagsignal() == false)
		{
			assert(message.deviceobject().has_diagsignal());
			return false;
		}

		const Proto::DeviceDiagSignal& signalMessage = message.deviceobject().diagsignal();

		Q_UNUSED(signalMessage);
		//x = signalMessage.startxdocpt();
		//y = signalMessage.startydocpt();

		return true;
	}

	DeviceType DeviceDiagSignal::deviceType() const
	{
		return m_deviceType;
	}

}
