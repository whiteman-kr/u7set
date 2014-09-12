#include "../include/DeviceObject.h"
#include "../include/ProtoSerialization.h"


namespace Hardware
{
	Factory<Hardware::DeviceObject> DeviceObjectFactory;

	void Init()
	{
		Hardware::DeviceObjectFactory.Register<DeviceRoot>();
		Hardware::DeviceObjectFactory.Register<DeviceSystem>();
		Hardware::DeviceObjectFactory.Register<DeviceRack>();
		Hardware::DeviceObjectFactory.Register<DeviceChassis>();
		Hardware::DeviceObjectFactory.Register<DeviceModule>();
		Hardware::DeviceObjectFactory.Register<DeviceController>();
		Hardware::DeviceObjectFactory.Register<DeviceDiagSignal>();
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

	DeviceType DeviceRoot::deviceType()
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
	}

	DeviceType DeviceSystem::deviceType()
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

	DeviceType DeviceRack::deviceType()
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

	DeviceType DeviceChassis::deviceType()
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

	DeviceType DeviceModule::deviceType()
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

	DeviceType DeviceController::deviceType()
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

	DeviceType DeviceDiagSignal::deviceType()
	{
		return m_deviceType;
	}

}
