#include "../include/DeviceObject.h"
#include "../include/ProtoSerialization.h"


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
	DeviceObject::DeviceObject()
	{
	}

	DeviceObject::~DeviceObject()
	{
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
		m_strId = Proto::Read(deviceobject.strid());
		m_caption = Proto::Read(deviceobject.caption());

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



	//
	//
	// DeviceRoot
	//
	//
	DeviceRoot::DeviceRoot() :
		DeviceObject()
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
	DeviceSystem::DeviceSystem() :
		DeviceObject()
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
	DeviceRack::DeviceRack() :
		DeviceObject()
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
	DeviceChassis::DeviceChassis() :
		DeviceObject()
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
		Proto::DeviceChassis* chassisMessage =
				message->mutable_deviceobject()->mutable_chassis();

		Q_UNUSED(chassisMessage);
		//chassisMessage ->set_startxdocpt(m_startXDocPt);
		//chassisMessage ->set_startydocpt(m_startYDocPt);

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

		Q_UNUSED(chassisMessage);
		//x = chassisMessage.startxdocpt();
		//y = chassisMessage.startydocpt();

		return true;
	}

	DeviceType DeviceChassis::deviceType() const
	{
		return m_deviceType;
	}

	//
	//
	// DeviceModule
	//
	//
	DeviceModule::DeviceModule() :
		DeviceObject()
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
		Proto::DeviceModule* moduleMessage =
				message->mutable_deviceobject()->mutable_module();

		Q_UNUSED(moduleMessage);
		//moduleMessage->set_startxdocpt(m_startXDocPt);
		//moduleMessage->set_startydocpt(m_startYDocPt);

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

		Q_UNUSED(moduleMessage);
		//x = moduleMessage.startxdocpt();
		//y = moduleMessage.startydocpt();

		return true;
	}

	DeviceType DeviceModule::deviceType() const
	{
		return m_deviceType;
	}


	//
	//
	// DeviceController
	//
	//
	DeviceController::DeviceController() :
		DeviceObject()
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
	DeviceDiagSignal::DeviceDiagSignal() :
		DeviceObject()
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
