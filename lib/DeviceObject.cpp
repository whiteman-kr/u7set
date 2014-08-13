#include "..\include\DeviceObject.h"

//
//
// DeviceObject
//
//
DeviceObject::DeviceObject() :
	m_parent(nullptr)
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
	return m_children.at(index);
}

void DeviceObject::addChild(std::shared_ptr<DeviceObject> child)
{
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


//
//
// DeviceCase
//
//
DeviceCase::DeviceCase() :
	DeviceObject()
{
}

DeviceCase::~DeviceCase()
{
}

//
//
// DeviceSubblock
//
//
DeviceSubblock::DeviceSubblock() :
	DeviceObject()
{
}

DeviceSubblock::~DeviceSubblock()
{
}


//
//
// DeviceBlock
//
//
DeviceBlock::DeviceBlock() :
    DeviceObject()
{
}

DeviceBlock::~DeviceBlock()
{
}
