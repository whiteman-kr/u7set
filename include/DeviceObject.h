#ifndef DEVICEOBJECT_H
#define DEVICEOBJECT_H

//
//
// DeviceObject
//
//
class DeviceObject
{
protected:
	DeviceObject();
	virtual ~DeviceObject();

    // Properties
    //
public:
	DeviceObject* parent();

    // Children care
    //
	int childrenCount() const;

	DeviceObject* child(int index) const;

	int childIndex(DeviceObject* child) const;

	std::shared_ptr<DeviceObject> childSharedPtr(int index);

	void addChild(std::shared_ptr<DeviceObject> child);
	void deleteAllChildren();

    // Props
    //
	const QString& strId() const;
	void setStrId(const QString& value);

	const QString& caption() const;
	void setCaption(const QString& value);

    // Data
    //
protected:
    DeviceObject* m_parent;
    std::vector<std::shared_ptr<DeviceObject>> m_children;

	QString m_strId;
    QString m_caption;
};


//
//
// DeviceRoot
//
//
class DeviceRoot : public DeviceObject
{
public:
    DeviceRoot();
	virtual ~DeviceRoot();
};


//
//
// DeviceSystem
//
//
class DeviceSystem : public DeviceObject
{
public:
    DeviceSystem();
    virtual ~DeviceSystem();
};


//
//
// DeviceCase
//
//
class DeviceCase : public DeviceObject
{
public:
    DeviceCase();
    virtual ~DeviceCase();
};


//
//
// DeviceSubblock
//
//
class DeviceSubblock : public DeviceObject
{
public:
    DeviceSubblock();
    virtual ~DeviceSubblock();
};


//
//
// DeviceBlock
//
//
class DeviceBlock : public DeviceObject
{
public:
    DeviceBlock();
    virtual ~DeviceBlock();
};


#endif // DEVICEOBJECT_H
