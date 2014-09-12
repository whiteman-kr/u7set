#pragma once
#include "DbStruct.h"
#include "QUuid"
#include "../include/ProtoSerialization.h"

namespace Hardware
{
	void Init();
	void Shutdwon();

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
	class DeviceObject :
		public QObject,
		public Proto::ObjectSerialization<DeviceObject>
	{
		Q_OBJECT

	protected:
		DeviceObject();
		virtual ~DeviceObject();

		// Serialization
		//
		friend Proto::ObjectSerialization<DeviceObject>;	// for call CreateObject from Proto::ObjectSerialization

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this function only while serialization, as when object is created is not fully initialized
		// and must be read before use
		//
		static DeviceObject* CreateObject(const Proto::Envelope& message);

	public:

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

		QUuid m_uuid;
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

	extern Factory<Hardware::DeviceObject> DeviceObjectFactory;

}
