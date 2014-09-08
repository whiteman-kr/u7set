#include "..\include\DeviceObject.h"

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
