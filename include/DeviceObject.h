#pragma once
#include "DbStruct.h"

// Device type, for defining hierrarche, don't save these data to file, can be changed (new level) later
//
enum class DeviceType
{
	Root,
	System,
	Rack,
	Chassis,
	Module,
	Controller,
	DiagSignal
};

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

public:
	virtual void load(const QByteArray& data);
	virtual void save(QByteArray* out_data) const;

    // Properties
    //
public:
	DeviceObject* parent();
	virtual DeviceType deviceType() = 0;

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

	const DbFileInfo& fileInfo() const;
	void setFileInfo(const DbFileInfo& value);

    // Data
    //
protected:
	DeviceObject* m_parent = nullptr;
    std::vector<std::shared_ptr<DeviceObject>> m_children;

	QString m_strId;
	QString m_caption;

	DbFileInfo m_fileInfo;
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

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::Root;
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

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::System;
};


//
//
// DeviceRack
//
//
class DeviceRack : public DeviceObject
{
public:
	DeviceRack();
	virtual ~DeviceRack();

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::Rack;
};


//
//
// DeviceChassis
//
//
class DeviceChassis : public DeviceObject
{
public:
	DeviceChassis();
	virtual ~DeviceChassis();

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::Chassis;
};


//
//
// DeviceModule
//
//
class DeviceModule : public DeviceObject
{
public:
	DeviceModule();
	virtual ~DeviceModule();

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::Module;
};


//
//
// DeviceController
//
//
class DeviceController : public DeviceObject
{
public:
	DeviceController();
	virtual ~DeviceController();

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::Controller;
};


//
//
// DeviceDiagSignal
//
//
class DeviceDiagSignal : public DeviceObject
{
public:
	DeviceDiagSignal();
	virtual ~DeviceDiagSignal();

public:
	virtual DeviceType deviceType() override;

private:
	static const DeviceType m_deviceType = DeviceType::DiagSignal;
};
