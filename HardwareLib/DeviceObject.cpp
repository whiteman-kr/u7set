#ifndef HARDWARE_LIB_DOMAIN
#error Don't include this file in the project! Link HardwareLib instead.
#endif

#include "DeviceObject.h"
#include "ScriptDeviceObject.h"
#include "../lib/ConstStrings.h"
#include "../Proto/ProtoSerialization.h"
#include <utility>
#include <QJSEngine>
#include <QQmlEngine>
#include <QDebug>
#include <QFile>
#include <QMetaObject>
#include <QMetaProperty>
#include <QXmlStreamReader>
#include <QFile>
#include <QMetaProperty>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

namespace Hardware
{
	const std::array<QString, 10> DeviceObjectExtensions =
		{
			".hrt",			// Root
			".hsm",			// System
			".hrk",			// Rack
			".hcs",			// Chassis
			".hmd",			// Module
			".hws",			// Workstation
			".hsw",			// Software
			".hcr",			// Controller
			".hds",			// AppSignal
			".hdds",		// DiagSignal
		};

	extern const std::array<QString, 10> DeviceTypeNames =
		{
			"Root",			// Root
			"System",		// System
			"Rack",			// Rack
			"Chassis",		// Chassis
			"Module",		// Module
			"Workstation",	// Workstation
			"Software",		// Software
			"Controller",	// Controller
			"AppSignal",	// Signal
			"DiagSignal",	// Signal
		};

	Factory<DeviceObject> DeviceObjectFactory;

	void init()
	{
		qDebug() << "Hardware::init";

		// --
		//
		static bool firstRun = false;
		if (firstRun)
		{
			Q_ASSERT(false);
			Hardware::DeviceObjectFactory.clear();
		}

		firstRun = true;

		//--
		//
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRoot>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceSystem>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRack>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceChassis>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceModule>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceController>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceAppSignal>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceAppSignal>("DeviceSignal");		// DeviceAppSignal used to be DeviceSignal, so create fabrica for DeviceSignal too
		Hardware::DeviceObjectFactory.Register<Hardware::Workstation>();
		Hardware::DeviceObjectFactory.Register<Hardware::Software>();

		return;
	}


	void shutdown()
	{
		qDebug() << "Hardware::Shutdown";

		DeviceObject::PrintRefCounter();

		return;
	}

	//
	//
	// PropertyNames
	//
	//
	const QString PropertyNames::fileId = "FileID";
	const QString PropertyNames::uuid = "Uuid";
	const QString PropertyNames::equipmentIdTemplate = "EquipmentIDTemplate";
	const QString PropertyNames::equipmentId = "EquipmentID";
	const QString PropertyNames::caption = "Caption";
	const QString PropertyNames::childRestriction = "ChildRestriction";
	const QString PropertyNames::place = "Place";
	const QString PropertyNames::specificProperties = "SpecificProperties";
	const QString PropertyNames::signalSpecificProperties = "SignalSpecificProperties";
	const QString PropertyNames::preset = "Preset";
	const QString PropertyNames::presetRoot = "PresetRoot";
	const QString PropertyNames::presetName = "PresetName";
	const QString PropertyNames::presetObjectUuid = "PresetObjectUuid";

	const QString PropertyNames::lmDescriptionFile = "LmDescriptionFile";
	const QString PropertyNames::lmNumber = "LMNumber";
	const QString PropertyNames::lmSubsystemChannel = "SubsystemChannel";
	const QString PropertyNames::lmSubsystemID = "SubsystemID";

	const QString PropertyNames::type = "Type";
	const QString PropertyNames::function = "Function";
	const QString PropertyNames::byteOrder = "ByteOrder";
	const QString PropertyNames::format = "Format";
	const QString PropertyNames::memoryArea = "MemoryArea";
	const QString PropertyNames::size = "Size";

	const QString PropertyNames::valueOffset = "ValueOffset";
	const QString PropertyNames::valueBit = "ValueBit";
	const QString PropertyNames::validitySignalId = "ValiditySiganlID";

	const QString PropertyNames::appSignalDataFormat = "AppAnalogSignalFormat";

	const QString PropertyNames::appSignalBusTypeId = "BusTypeID";

	const QString PropertyNames::categoryAppSignal = "AppSignal";

	//
	//
	// DeviceObject
	//
	//
	DeviceObject::DeviceObject(DeviceType deviceType, bool preset /*= false*/, QObject* parent /*= nullptr*/) noexcept :
		PropertyObject(parent),
		m_deviceType(deviceType),
		m_preset(preset)
	{
		auto uuidProp = ADD_PROPERTY_GETTER(QUuid, PropertyNames::uuid, true, DeviceObject::uuid);
		uuidProp->setExpert(true);

		auto equipmentIdTemplateProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::equipmentIdTemplate, true, DeviceObject::equipmentIdTemplate, DeviceObject::setEquipmentIdTemplate);
		equipmentIdTemplateProp->setValidator(QLatin1String("^[a-zA-Z0-9#$_()]*$"));

		auto equipmentIdProp = ADD_PROPERTY_GETTER(QString, PropertyNames::equipmentId, true, DeviceObject::equipmentId);
		equipmentIdProp->setReadOnly(true);

		auto captionProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::caption, true, DeviceObject::caption, DeviceObject::setCaption);

		auto childRestrProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::childRestriction, true, DeviceObject::childRestriction, DeviceObject::setChildRestriction);
		childRestrProp->setExpert(true);
		childRestrProp->setIsScript(true);

		ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::place, true, DeviceObject::place, DeviceObject::setPlace);

		auto specificProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::specificProperties, true, DeviceObject::specificProperties, DeviceObject::setSpecificProperties);
		specificProp->setExpert(true);
		specificProp->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);

		auto presetProp = ADD_PROPERTY_GETTER(bool, PropertyNames::preset, true, DeviceObject::preset);
		presetProp->setExpert(true);

		auto presetRootProp = ADD_PROPERTY_GETTER(bool, PropertyNames::presetRoot, true, DeviceObject::presetRoot);
		presetRootProp->setExpert(true);

		if (preset == true)
		{
			auto presetNameProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::presetName, true, DeviceObject::presetName, DeviceObject::setPresetName);
			presetNameProp->setExpert(true);
		}

		auto presetObjectUuidProp = ADD_PROPERTY_GETTER(QUuid, PropertyNames::presetObjectUuid, true, DeviceObject::presetObjectUuid);
		presetObjectUuidProp->setExpert(true);

		captionProp->setUpdateFromPreset(true);
		childRestrProp->setUpdateFromPreset(true);
		specificProp->setUpdateFromPreset(true);

		return;
	}

	void DeviceObject::dump(bool dumpProps, bool dumpTree, QString* out, int nesting /*= 0*/) const
	{
		auto print = [out](const QString& str, int nesting) mutable
		{
			QString r = QString("\n%1%2").arg("", nesting * 4).arg(str);

			if (out == nullptr)
			{
				qDebug().noquote() << r;
			}
			else
			{
				*out += r;
			}
		};

		print(equipmentId() + " " + metaObject()->className(), nesting);

		if (dumpProps == true)
		{
			auto props = properties();

			// Sort properties by caption
			//
			std::sort(props.begin(), props.end(), [](const auto& p1, const auto& p2) { return p1->caption() < p2->caption(); });

			for (const auto& p : props)
			{
				print(QStringLiteral("|") + p->caption() + ": " + p->value().toString(), nesting);
			}
		}

		if (dumpTree == true)
		{
			// Sort children by place + caption
			//
			auto children = m_children;
			std::sort(children.begin(), children.end(),
			[](const auto& ch1, const auto& ch2)
			{
				if (ch1->place() != ch2->place())
				{
					return ch1->place() < ch2->place();

				}
				else
				{
					return ch1->caption() < ch2->caption();
				}
			});

			for (const auto& child : children)
			{
				child->dump(dumpProps, true, out, nesting + 1);
			}
		}

		return;
	}

	std::shared_ptr<const DeviceObject> DeviceObject::sharedPtr() const
	{
		return shared_from_this();
	}

	std::shared_ptr<DeviceObject> DeviceObject::sharedPtr()
	{
		return shared_from_this();
	}

	bool DeviceObject::SaveData(Proto::Envelope* message) const
	{
		bool ok = SaveData(message, false);
		return ok;
	}

	bool DeviceObject::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);

		Proto::DeviceObject* mutableDeviceObject = message->mutable_deviceobject();

		Proto::Write(mutableDeviceObject->mutable_uuid(), m_uuid);
		Proto::Write(mutableDeviceObject->mutable_equipmentid(), m_equipmentId);
		Proto::Write(mutableDeviceObject->mutable_caption(), m_caption);

		mutableDeviceObject->set_place(m_place);
		mutableDeviceObject->set_childcounthint(static_cast<int>(m_children.size()));

		if (m_childRestriction.isEmpty() == false)
		{
			Proto::Write(mutableDeviceObject->mutable_childrestriction(), m_childRestriction);
		}

		if (m_specificPropertiesStruct.isEmpty() == false)
		{
			mutableDeviceObject->set_specific_properties_struct(m_specificPropertiesStruct.toStdString());
		}

		// Save specific properties' values
		//
		std::vector<std::shared_ptr<Property>> props = this->properties();

		for (const auto& p : props)
		{
			if (p->specific() == true)
			{
				::Proto::Property* protoProp = mutableDeviceObject->mutable_properties()->Add();
				Proto::saveProperty(protoProp, p);
			}
		}

		// --
		//
		if (m_preset == true)
		{
			mutableDeviceObject->set_preset(m_preset);

			mutableDeviceObject->set_presetroot(m_presetRoot);
			Proto::Write(mutableDeviceObject->mutable_presetname(), m_presetName);
			Proto::Write(mutableDeviceObject->mutable_presetobjectuuid(), m_presetObjectUuid);
		}

		// Save children if it is necessary (can be in serialization for the clipboard)
		//
		if (saveTree == true)
		{
			for (const std::shared_ptr<DeviceObject>& child : m_children)
			{
				::Proto::Envelope* childMessage = mutableDeviceObject->add_children();
				Q_ASSERT(childMessage);

				child->SaveData(childMessage, saveTree);
			}
		}

		return true;
	}

	bool DeviceObject::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return false;
		}

		const Proto::DeviceObject& deviceobject = message.deviceobject();

		m_uuid = Proto::Read(deviceobject.uuid());
		Q_ASSERT(m_uuid.isNull() == false);

		Proto::Read(deviceobject.equipmentid(), &m_equipmentId);
		Proto::Read(deviceobject.caption(), &m_caption);
		m_place = deviceobject.place();

		size_t childCountHint = deviceobject.childcounthint();
		if (childCountHint != 0)
		{
			m_children.reserve(childCountHint);
		}

		if (deviceobject.has_childrestriction() == true)
		{
			Proto::Read(deviceobject.childrestriction(), &m_childRestriction);
		}
		else
		{
			m_childRestriction.clear();
		}

		if (deviceobject.has_specific_properties_struct() == true)
		{
			m_specificPropertiesStruct = QString::fromStdString(deviceobject.specific_properties_struct());
			parseSpecificPropertiesStruct(m_specificPropertiesStruct);
		}
		else
		{
			m_specificPropertiesStruct.clear();
		}

		// Load specific properties' values. They are already exists after calling parseSpecificPropertiesStruct()
		//
		std::vector<std::shared_ptr<Property>> specificProps = PropertyObject::specificProperties();

		for (const ::Proto::Property& p :  deviceobject.properties())
		{
			auto it = std::find_if(specificProps.begin(), specificProps.end(),
				[p](std::shared_ptr<Property>& dp)
				{
					return dp->caption().toStdString() == p.name();
				});

			if (it == specificProps.end())
			{
				qDebug() << "ERROR: Can't find property " << p.name().c_str() << " in" << m_equipmentId;
			}
			else
			{
				std::shared_ptr<Property>& property = *it;

				Q_ASSERT(property->specific() == true);	// it's suppose to be specific property;

				bool loadOk = Proto::loadProperty(p, property);

				Q_UNUSED(loadOk);
				Q_ASSERT(loadOk);
			}
		}

		// --
		//
		if (deviceobject.has_preset() == true && deviceobject.preset() == true)
		{
			m_preset = deviceobject.preset();

			if (m_preset == true)
			{
				auto presetNameProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::presetName, true, DeviceObject::presetName, DeviceObject::setPresetName);
				presetNameProp->setExpert(true);
			}

			if (deviceobject.has_presetroot() == true)
			{
				m_presetRoot = deviceobject.presetroot();
			}
			else
			{
				Q_ASSERT(deviceobject.has_presetroot());
			}

			if (deviceobject.has_presetname() == true)
			{
				Proto::Read(deviceobject.presetname(), &m_presetName);
			}
			else
			{
				Q_ASSERT(deviceobject.has_presetname());
			}

			if (deviceobject.has_presetobjectuuid() == true)
			{
				m_presetObjectUuid = Proto::Read(deviceobject.presetobjectuuid());
			}
			else
			{
				Q_ASSERT(deviceobject.has_presetobjectuuid());
			}
		}

		// Load children if all tree was saved
		//
		if (this->isRack() == true && deviceobject.children_size() > 0)
		{
			// Multithread reading
			//
			std::vector<QFuture<std::shared_ptr<DeviceObject>>> threadFuncs;
			threadFuncs.reserve(deviceobject.children_size());

			for (int childIndex = 0; childIndex < deviceobject.children_size(); childIndex++)
			{
				const ::Proto::Envelope& childMessage = deviceobject.children(childIndex);

				QFuture<std::shared_ptr<DeviceObject>> f = QtConcurrent::run(DeviceObject::CreateObject, childMessage);

				threadFuncs.push_back(f);
			}

			for (QFuture<std::shared_ptr<DeviceObject>>& f : threadFuncs)
			{
				std::shared_ptr<DeviceObject> child = f.result();

				if (child == nullptr)
				{
					Q_ASSERT(child);
					continue;
				}

				addChild(child);
			}
		}
		else
		{
			for (int childIndex = 0; childIndex < deviceobject.children_size(); childIndex++)
			{
				const ::Proto::Envelope& childMessage = deviceobject.children(childIndex);

				std::shared_ptr<DeviceObject> child(DeviceObject::Create(childMessage));

				if (child == nullptr)
				{
					Q_ASSERT(child);
					continue;
				}

				addChild(child);
			}
		}

		return true;
	}

	std::shared_ptr<DeviceObject> DeviceObject::CreateObject(const Proto::Envelope& message)
	{
		// This func can create only one instance
		//
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<DeviceObject> deviceObject = DeviceObjectFactory.Create(classNameHash);

		if (deviceObject == nullptr)
		{
			Q_ASSERT(deviceObject);
			return deviceObject;
		}

		deviceObject->LoadData(message);

		return deviceObject;
	}

	bool DeviceObject::SaveObjectTree(Proto::Envelope* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		try
		{
			bool ok = this->SaveData(message, true);
			return ok;
		}
		catch (...)
		{
			Q_ASSERT(false);
			return false;
		}
	}

	void DeviceObject::expandEquipmentId()
	{
		// The same procedure is done in expandEquipmentId, keep it in mind if add any new macroses
		//
		if (hasParent() == true)
		{
			m_equipmentId.replace(QLatin1String("$(PARENT)"), parent()->equipmentIdTemplate(), Qt::CaseInsensitive);
		}

		m_equipmentId.replace(QLatin1String("$(PLACE)"), QString::number(place()).rightJustified(2, '0'), Qt::CaseInsensitive);

		for (auto& child : m_children)
		{
			child->expandEquipmentId();
		}

		return;
	}

	// Get all signals, including signals from child items
	//
	std::vector<std::shared_ptr<DeviceAppSignal>> DeviceObject::getAllAppSignals() const
	{
		std::vector<std::shared_ptr<DeviceAppSignal>> deviceSignals;
		deviceSignals.reserve(128);

		getAllAppSignalsRecursive(&deviceSignals);

		return deviceSignals;
	}

	bool DeviceObject::event(QEvent* /*e*/)
	{
		// Event was not recognized
		//
		return false;
	}



	// Get all signals, including signals from child items
	//
	void DeviceObject::getAllAppSignalsRecursive(std::vector<std::shared_ptr<DeviceAppSignal>>* deviceSignals) const
	{
		if (deviceSignals == nullptr)
		{
			Q_ASSERT(deviceSignals);
			return;
		}

		for (const std::shared_ptr<DeviceObject>& child : m_children)
		{
			if (child->deviceType() == DeviceType::AppSignal)
			{
				deviceSignals->push_back(std::dynamic_pointer_cast<DeviceAppSignal>(child));
				Q_ASSERT(dynamic_cast<DeviceAppSignal*>(deviceSignals->back().get()) != nullptr);
			}
			else
			{
				child->getAllAppSignalsRecursive(deviceSignals);
			}
		}

		return;
	}

	bool DeviceObject::hasParent() const noexcept
	{
		return m_parent.expired() == false;
	}

	std::shared_ptr<DeviceObject> DeviceObject::parent() noexcept
	{
		return m_parent.lock();
	}

	const std::shared_ptr<DeviceObject> DeviceObject::parent() const noexcept
	{
		return m_parent.lock();
	}

	DeviceType DeviceObject::deviceType() const noexcept
	{
		return m_deviceType;
	}

	bool DeviceObject::isRoot() const noexcept
	{
		return deviceType() == DeviceType::Root;
	}

	bool DeviceObject::isSystem() const noexcept
	{
		return deviceType() == DeviceType::System;
	}

	bool DeviceObject::isRack() const noexcept
	{
		return deviceType() == DeviceType::Rack;
	}

	bool DeviceObject::isChassis() const noexcept
	{
		return deviceType() == DeviceType::Chassis;
	}

	bool DeviceObject::isModule() const noexcept
	{
		return deviceType() == DeviceType::Module;
	}

	bool DeviceObject::isController() const noexcept
	{
		return deviceType() == DeviceType::Controller;
	}

	bool DeviceObject::isWorkstation() const noexcept
	{
		return deviceType() == DeviceType::Workstation;
	}

	bool DeviceObject::isSoftware() const noexcept
	{
		return deviceType() == DeviceType::Software;
	}

	bool DeviceObject::isAppSignal() const noexcept
	{
		return deviceType() == DeviceType::AppSignal;
	}

	std::shared_ptr<const DeviceRoot> DeviceObject::toRoot() const noexcept
	{
		Q_ASSERT(isRoot());
		return toType<DeviceRoot>();
	}

	std::shared_ptr<Hardware::DeviceRoot> DeviceObject::toRoot() noexcept
	{
		return toType<DeviceRoot>();
	}

	std::shared_ptr<const DeviceSystem> DeviceObject::toSystem() const noexcept
	{
		return toType<DeviceSystem>();
	}

	std::shared_ptr<DeviceSystem> DeviceObject::toSystem() noexcept
	{
		return toType<DeviceSystem>();
	}

	std::shared_ptr<const DeviceRack> DeviceObject::toRack() const noexcept
	{
		return toType<DeviceRack>();
	}

	std::shared_ptr<DeviceRack> DeviceObject::toRack() noexcept
	{
		return toType<DeviceRack>();
	}

	std::shared_ptr<const DeviceChassis> DeviceObject::toChassis() const noexcept
	{
		return toType<DeviceChassis>();
	}

	std::shared_ptr<DeviceChassis> DeviceObject::toChassis() noexcept
	{
		return toType<DeviceChassis>();
	}

	std::shared_ptr<const DeviceModule> DeviceObject::toModule() const noexcept
	{
		return toType<const DeviceModule>();
	}

	std::shared_ptr<DeviceModule> DeviceObject::toModule() noexcept
	{
		return toType<DeviceModule>();
	}

	std::shared_ptr<const DeviceController> DeviceObject::toController() const noexcept
	{
		return toType<const DeviceController>();
	}

	std::shared_ptr<DeviceController> DeviceObject::toController() noexcept
	{
		return toType<DeviceController>();
	}

	std::shared_ptr<const DeviceAppSignal> DeviceObject::toAppSignal() const noexcept
	{
		return toType<const DeviceAppSignal>();
	}

	std::shared_ptr<DeviceAppSignal> DeviceObject::toAppSignal() noexcept
	{
		return toType<DeviceAppSignal>();
	}

	std::shared_ptr<const Workstation> DeviceObject::toWorkstation() const noexcept
	{
		return toType<const Workstation>();
	}

	std::shared_ptr<Workstation> DeviceObject::toWorkstation() noexcept
	{
		return toType<Workstation>();
	}

	std::shared_ptr<const Software> DeviceObject::toSoftware() const noexcept
	{
		return toType<const Software>();
	}

	std::shared_ptr<Software> DeviceObject::toSoftware() noexcept
	{
		return toType<Software>();
	}

	const Hardware::DeviceController* DeviceObject::getParentController() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isController())
			{
				return deviceObject->toController().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceModule* DeviceObject::getParentModule() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isModule())
			{
				return deviceObject->toModule().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceChassis* DeviceObject::getParentChassis() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isChassis())
			{
				return deviceObject->toChassis().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceRack* DeviceObject::getParentRack() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isRack())
			{
				return deviceObject->toRack().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceSystem* DeviceObject::getParentSystem() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isSystem())
			{
				return deviceObject->toSystem().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::DeviceRoot* DeviceObject::getParentRoot() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isRoot())
			{
				return deviceObject->toRoot().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	QString DeviceObject::fileExtension() const
	{
		static_assert(std::size(Hardware::DeviceObjectExtensions) == static_cast<size_t>(DeviceType::DeviceTypeCount));
		static_assert(std::size(Hardware::DeviceTypeNames) == static_cast<size_t>(DeviceType::DeviceTypeCount));

		size_t index = static_cast<size_t>(deviceType());
		Q_ASSERT(index < std::size(Hardware::DeviceObjectExtensions));

		QString result = Hardware::DeviceObjectExtensions[index];
		return result;
	}

	QString DeviceObject::fileExtension(DeviceType device)
	{
		QString result = Hardware::DeviceObjectExtensions[static_cast<size_t>(device)];
		return result;
	}

	void DeviceObject::setExpertToProperty(const QString& property, bool expert)
	{
		// If property is not created yet, do not set expert to it
		//
		if (propertyExists(property, false) == false)
		{
			return;
		}

		std::shared_ptr<Property> prop = propertyByCaption(property);

		if (prop != nullptr)
		{
			prop->setExpert(expert);
		}
		else
		{
			Q_ASSERT(prop);
		}

		return;
	}

	int DeviceObject::childrenCount() const
	{
		return static_cast<int>(m_children.size());
	}

	int DeviceObject::childIndex(const std::shared_ptr<const DeviceObject>& child) const
	{
		// Manual search for an index is 1.6 times faster than std::find
		//
		int result = -1;

		for (size_t i = 0, childCount = m_children.size(); i < childCount; i++)
		{
			if (m_children[i] == child)
			{
				result = static_cast<int>(i);
				break;
			}
		}

		return result;
	}

	[[nodiscard]] const std::vector<std::shared_ptr<DeviceObject>>& DeviceObject::children() const
	{
		return m_children;
	}

	const std::shared_ptr<DeviceObject>& DeviceObject::child(int index) const
	{
		return m_children.at(index);
	}

	std::shared_ptr<DeviceObject> DeviceObject::child(const QUuid& uuid) const
	{
		std::shared_ptr<DeviceObject> result;

		for (const std::shared_ptr<DeviceObject>& child : m_children)
		{
			if (child->uuid() == uuid)
			{
				result = child;
				break;
			}
		}

		return result;
	}

	std::shared_ptr<DeviceObject> DeviceObject::childByPresetUuid(const QUuid& presetObjectUuid) const
	{
		std::shared_ptr<DeviceObject> result;

		for (const std::shared_ptr<DeviceObject>& child : m_children)
		{
			if (child->presetObjectUuid() == presetObjectUuid)
			{
				result = child;
				break;
			}
		}

		return result;
	}

	std::shared_ptr<DeviceObject> DeviceObject::childByEquipmentId(const QString& id)
	{
		if (equipmentId() == id)
		{
			return this->shared_from_this();
		}

		for (const auto& child : m_children)
		{
			auto r = child->childByEquipmentId(id);
			if (r != nullptr)
			{
				return r;
			}
		}

		return {};
	}

	bool DeviceObject::canAddChild(const DeviceType childType) const
	{
		if (childType == DeviceType::Software &&
			deviceType() != DeviceType::Workstation &&
			deviceType() != DeviceType::Root)
		{
			return false;
		}

		if (deviceType() == DeviceType::Software)
		{
			return false;
		}

		if (deviceType() == DeviceType::Workstation)
		{
			return childType == DeviceType::Software;
		}

		if (deviceType() >= childType)
		{
			return false;
		}

		if (childType == DeviceType::Workstation &&
			deviceType() > DeviceType::Chassis)
		{
			return false;
		}

		return true;
	}

	void DeviceObject::addChild(const std::shared_ptr<Hardware::DeviceObject>& child)
	{
		if (child == nullptr)
		{
			Q_ASSERT(child);
			return;
		}

		if (canAddChild(child->deviceType()) == false)
		{
			Q_ASSERT(canAddChild(child->deviceType()));
			return;
		}

		child->m_parent = shared_from_this();
		m_children.push_back(child);

		return;
	}

	void DeviceObject::deleteChild(std::shared_ptr<DeviceObject> child)
	{
		auto found = std::find_if(m_children.begin(), m_children.end(), [child](decltype(m_children)::const_reference c)
			{
				return c == child;
			});

		if (found == m_children.end())
		{
			Q_ASSERT(found != m_children.end());
			return;
		}

		m_children.erase(found);
		return;
	}

	void DeviceObject::deleteAllChildren()
	{
		m_children.clear();
	}

	bool DeviceObject::checkChild(std::shared_ptr<DeviceObject> child, QString* errorMessage)
	{
		if (child == nullptr ||
			errorMessage == nullptr)
		{
			Q_ASSERT(child);
			Q_ASSERT(errorMessage);
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

		// Run m_childRestriction script
		//
		QJSEngine engine;

		QJSValue function = engine.evaluate(m_childRestriction);
		if (function.isError())
		{
			qDebug() << "Script evaluate error at line " << function.property("lineNumber").toInt();
			qDebug() << "\tClass: " << metaObject()->className();
			qDebug() << "\tStack: " << function.property("stack").toString();
			qDebug() << "\tMessage: " << function.toString();

			*errorMessage += tr("DeviceObject::childRestriction script evaluation error, object %1, error %2, line %3")
							 .arg(equipmentId())
							 .arg(function.toString())
							 .arg(function.property("lineNumber").toInt());
			return false;
		}

		QJSValue arg = engine.newQObject(new ScriptDeviceObject(child));
		QJSValue result = function.call(QJSValueList() << arg);
		if (result.isError() == true)
		{
			*errorMessage = tr("Script error: ").arg(result.toString());
			return false;
		}

		bool boolResult = result.toBool();
		if (boolResult == false)
		{
			*errorMessage = tr("DeviceObject is not allowed.");
		}

		return boolResult;
	}

	QString DeviceObject::replaceEngeneeringToEngineering(const QString& data)
	{
		QString result = data;

		result.replace(QLatin1String("engeneering"), QLatin1String("engineering"), Qt::CaseSensitive);
		result.replace(QLatin1String("Engeneering"), QLatin1String("Engineering"), Qt::CaseSensitive);

		return result;
	}

	QUuid DeviceObject::uuid() const
	{
		return m_uuid;
	}

	void DeviceObject::setUuid(QUuid value)
	{
		m_uuid = value;
	}

	QString DeviceObject::equipmentIdTemplate() const
	{
		return m_equipmentId;
	}

	void DeviceObject::setEquipmentIdTemplate(const QString& value)
	{
		if (m_equipmentId != value)
		{
			m_equipmentId = value;
			m_equipmentId.replace(QRegExp(QStringLiteral("[^a-zA-Z0-9#$_()]")), QStringLiteral("#"));
		}
	}

	QString DeviceObject::equipmentId() const
	{
		if (equipmentIdTemplate().contains(QChar('$')) == false)
		{
			// m_equipmentId does not have any marcos variables, just return it
			//
			return equipmentIdTemplate();
		}

		std::array<std::pair<const DeviceObject*, QString>, static_cast<size_t>(DeviceType::DeviceTypeCount)> devices;
		size_t deviceCount = 0;

		const DeviceObject* d = this;
		while (d != nullptr)
		{
			devices[deviceCount++] = std::make_pair(d, d->equipmentIdTemplate());
			d = d->parent().get();
		}

		// !WARNING!
		// The same procedure is done in expandEquipmentId, keep it in mind if add any new macroses
		//
		QString parentId;
		size_t deviceIndex = deviceCount;

		while (deviceIndex > 0)
		{
			deviceIndex--;

			const DeviceObject* device = devices[deviceIndex].first;
			QString equipId = devices[deviceIndex].second;

			if (device->hasParent() == true)
			{
				equipId.replace(QLatin1String("$(PARENT)"), parentId, Qt::CaseInsensitive);
			}

			equipId.replace(QLatin1String("$(PLACE)"), QString::number(device->place()).rightJustified(2, '0'), Qt::CaseInsensitive);

			parentId = equipId;
		}

		QString thisId = parentId;
		return thisId;
	}

	QString DeviceObject::caption() const
	{
		return m_caption;
	}

	void DeviceObject::setCaption(QString value)
	{
		if (m_caption != value)
		{
			m_caption = value;
			m_caption.replace(QChar::LineFeed, " ");
		}
	}

	QString DeviceObject::childRestriction() const
	{
		return m_childRestriction;
	}

	void DeviceObject::setChildRestriction(QString value)
	{
		m_childRestriction = value;
	}

	QString DeviceObject::specificProperties() const
	{
		return m_specificPropertiesStruct;
	}

	void DeviceObject::setSpecificProperties(QString value)
	{
		if (m_specificPropertiesStruct != value)
		{
			m_specificPropertiesStruct = value;
			parseSpecificPropertiesStruct(m_specificPropertiesStruct);
		}
	}

	int DeviceObject::place() const
	{
		return m_place;
	}

	void DeviceObject::setPlace(int value)
	{
		m_place = value;
	}

	// JSON short description, uuid, equipmentId, caption, place, etc
	//
	QString DeviceObject::details() const
	{
		QString captionEscaped = caption();
		captionEscaped.replace(QLatin1String("'"), QLatin1String("''"));
		captionEscaped.replace(QLatin1String("\""), QLatin1String("\\\""));

		QString json = QString(
R"DELIM({
	"Uuid" : "%1",
	"EquipmentID" : "%2",
	"Caption" : "%3",
	"Place" : %4,
	"Type" : "%5"
})DELIM")
			.arg(uuid().toString())
			.arg(equipmentIdTemplate())
			.arg(captionEscaped)
			.arg(place())
			.arg(fileExtension());

		return json;
	}

	bool DeviceObject::preset() const
	{
		return m_preset;
	}

	bool DeviceObject::presetRoot() const
	{
		return m_presetRoot;
	}

	void DeviceObject::setPresetRoot(bool value)
	{
		m_presetRoot = value;
	}

	QString DeviceObject::presetName() const
	{
		return m_presetName;
	}

	void DeviceObject::setPresetName(QString value)
	{
		m_presetName = value;
	}

	QUuid DeviceObject::presetObjectUuid() const
	{
		return m_presetObjectUuid;
	}

	void DeviceObject::setPresetObjectUuid(QUuid value)
	{
		m_presetObjectUuid = value;
	}


	DbFileInfo* DeviceObject::data()
	{
		return m_data.get();
	}

	const DbFileInfo* DeviceObject::data() const
	{
		return m_data.get();
	}

	void DeviceObject::setData(std::shared_ptr<DbFileInfo> data)
	{
		m_data = std::move(data);
		return;
	}

	//
	//
	// DeviceRoot
	//
	//
	DeviceRoot::DeviceRoot(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Root, preset, parent)
	{
	}

	//
	//
	// DeviceSystem
	//
	//
	DeviceSystem::DeviceSystem(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::System, preset, parent)
	{
		//qDebug() << "DeviceRoot::DeviceSystem";

		auto p = propertyByCaption(PropertyNames::equipmentIdTemplate);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}
	}

	bool DeviceSystem::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceSystem* systemMessage = message->mutable_deviceobject()->mutable_system();

		Q_UNUSED(systemMessage);
		//systemMessage->set_startxdocpt(m_startXDocPt);
		//systemMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceSystem::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
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
			Q_ASSERT(message.deviceobject().has_system());
			return false;
		}

		const Proto::DeviceSystem& systemMessage = message.deviceobject().system();

		Q_UNUSED(systemMessage);
		//m_startXDocPt = systemMessage.startxdocpt();
		//m_startYDocPt = systemMessage.startydocpt();

		return true;
	}


	//
	//
	// DeviceRack
	//
	//
	DeviceRack::DeviceRack(bool preset /*= false*/, QObject* parent/* = nullptr*/) :
		DeviceObject(DeviceType::Rack, preset, parent)
	{
		auto p = propertyByCaption(PropertyNames::equipmentIdTemplate);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}
	}

	bool DeviceRack::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceRack* rackMessage = message->mutable_deviceobject()->mutable_rack();

		Q_UNUSED(rackMessage);
		//rackMessage->set_startxdocpt(m_startXDocPt);
		//rackMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceRack::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
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
			Q_ASSERT(message.deviceobject().has_rack());
			return false;
		}

		const Proto::DeviceRack& rackMessage = message.deviceobject().rack();

		Q_UNUSED(rackMessage);
		//x = rackMessage.startxdocpt();
		//y = rackMessage.startydocpt();

		return true;
	}

	//
	//
	// DeviceChassis
	//
	//
	DeviceChassis::DeviceChassis(bool preset /*= false*/, QObject* parent/* = nullptr*/) :
		DeviceObject(DeviceType::Chassis, preset, parent)
	{
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, DeviceChassis::type, DeviceChassis::setType);
		typeProp->setUpdateFromPreset(true);
		typeProp->setExpert(true);

		auto p = propertyByCaption(PropertyNames::place);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}

	}

	bool DeviceChassis::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
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
			Q_ASSERT(message.has_deviceobject());
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
			Q_ASSERT(message.deviceobject().has_chassis());
			return false;
		}

		const Proto::DeviceChassis& chassisMessage = message.deviceobject().chassis();

		m_type =  chassisMessage.type();

		return true;
	}

	std::shared_ptr<DeviceModule> DeviceChassis::findLogicModule()
	{
		for(const auto& child : m_children)
		{
			if (child == nullptr)
			{
				Q_ASSERT(child);
				continue;
			}

			if (child->isModule())
			{
				std::shared_ptr<DeviceModule> module = child->toModule();

				if (module == nullptr)
				{
					Q_ASSERT(module);
					continue;
				}

				if (module->isLogicModule())
				{
					return module;
				}
			}
		}

		return {};
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
	DeviceModule::DeviceModule(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Module, preset, parent)
	{
		auto familyTypeProp = ADD_PROPERTY_GETTER_SETTER(DeviceModule::FamilyType, QLatin1String("ModuleFamily"), true, DeviceModule::moduleFamily, DeviceModule::setModuleFamily);
		familyTypeProp->setExpert(true);

		auto moduleVersionProp = ADD_PROPERTY_GETTER_SETTER(int, QLatin1String("ModuleVersion"), true, DeviceModule::moduleVersion, DeviceModule::setModuleVersion);
		moduleVersionProp->setExpert(true);

		auto configScriptProp = ADD_PROPERTY_GETTER_SETTER(QString, QLatin1String("ConfigurationScript"), true, DeviceModule::configurationScript, DeviceModule::setConfigurationScript);
		configScriptProp->setExpert(true);
		configScriptProp->setIsScript(true);

		auto rawDataDescrProp = ADD_PROPERTY_GETTER_SETTER(QString, QLatin1String("RawDataDescription"), true, DeviceModule::rawDataDescription, DeviceModule::setRawDataDescription);
		rawDataDescrProp->setExpert(true);

		auto customFamilyTypeProp = ADD_PROPERTY_GETTER_SETTER(int, QLatin1String("CustomModuleFamily"), true, DeviceModule::customModuleFamily, DeviceModule::setCustomModuleFamily);
		customFamilyTypeProp->setExpert(true);

		familyTypeProp->setUpdateFromPreset(true);
		moduleVersionProp->setUpdateFromPreset(true);
		configScriptProp->setUpdateFromPreset(true);
		rawDataDescrProp->setUpdateFromPreset(true);
		customFamilyTypeProp->setUpdateFromPreset(true);


		auto p = propertyByCaption(PropertyNames::place);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}
	}

	bool DeviceModule::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceModule* moduleMessage = message->mutable_deviceobject()->mutable_module();

		moduleMessage->set_moduletype(static_cast<int>(m_type));
		moduleMessage->set_custommodulefamily(m_customModuleFamily);
		moduleMessage->set_configurationscript(m_configurationScript.toStdString());
		moduleMessage->set_rawdatadescription(m_rawDataDescription.toStdString());

		return true;
	}

	bool DeviceModule::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
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
			Q_ASSERT(message.deviceobject().has_module());
			return false;
		}

		const Proto::DeviceModule& modulemessage = message.deviceobject().module();

		if (modulemessage.has_moduletype())
		{
			m_type =  static_cast<decltype(m_type)>(modulemessage.moduletype());
		}
		else
		{
			m_type =  static_cast<decltype(m_type)>(modulemessage.typeobsolete());

			if ((m_type & 0xff00) == 0x0100
				|| (m_type & 0xff00) == 0x0200
				|| (m_type & 0xff00) == 0x0300
				|| (m_type & 0xff00) == 0x0400
				|| (m_type & 0xff00) == 0x0500
				|| (m_type & 0xff00) == 0x0600
				|| (m_type & 0xff00) == 0x0700
				)
			{
				m_type |= 0x1000;	// Module family 01..07 changed to 11..17, this is for compatibitity
			}
		}

		m_customModuleFamily = static_cast<uint16_t>(modulemessage.custommodulefamily());

		m_configurationScript = QString::fromStdString(modulemessage.configurationscript());
		m_configurationScript = replaceEngeneeringToEngineering(m_configurationScript);		// Shit happens. We had a situaltion when misprinting was detected (EngEneering vs EngIneering).
																							// To avoid manual replacement of this typo for non platform modules, the replace just made.

		m_rawDataDescription = QString::fromStdString(modulemessage.rawdatadescription());

		return true;
	}

	DeviceModule::FamilyType DeviceModule::moduleFamily() const
	{
		return static_cast<DeviceModule::FamilyType>(m_type & 0xFF00);
	}

	void DeviceModule::setModuleFamily(DeviceModule::FamilyType value)
	{
		auto tmp = static_cast<decltype(m_type)>(value);

		Q_ASSERT((tmp & 0x00FF) == 0);

		tmp &= 0xFF00;

		m_type = (m_type & 0x00FF) | tmp;
	}

	int DeviceModule::customModuleFamily() const
	{
		return m_customModuleFamily;
	}

	void DeviceModule::setCustomModuleFamily(int value)
	{
		m_customModuleFamily = static_cast<uint16_t>(value);
	}

	int DeviceModule::moduleVersion() const
	{
		return static_cast<int>(m_type) & 0xFF;
	}

	void DeviceModule::setModuleVersion(int value)
	{
		auto tmp = static_cast<decltype(m_type)>(value);

		Q_ASSERT((tmp & 0xFF00) == 0);

		m_type = (m_type & 0xFF00) | tmp;
	}

	QString DeviceModule::configurationScript() const
	{
		return m_configurationScript;
	}

	void DeviceModule::setConfigurationScript(const QString& value)
	{
		m_configurationScript = value;
	}

	QString DeviceModule::rawDataDescription() const
	{
		return m_rawDataDescription;
	}

	void DeviceModule::setRawDataDescription(const QString& value)
	{
		m_rawDataDescription = value;
	}

	bool DeviceModule::hasRawData() const
	{
		return m_rawDataDescription.isEmpty() != true;
	}

	int DeviceModule::moduleType() const
	{
		return m_type;
	}

	bool DeviceModule::isIOModule() const
	{
		return isInputModule() || isOutputModule();
	}

	bool DeviceModule::isInputModule() const
	{
		FamilyType family = moduleFamily();

		return	family == FamilyType::AIM ||
				family == FamilyType::DIM ||
				family == FamilyType::WAIM ||
				family == FamilyType::TIM ||
				family == FamilyType::RIM ||
				family == FamilyType::AIFM ||
				family == FamilyType::MPS;
	}

	bool DeviceModule::isOutputModule() const
	{
		FamilyType family = moduleFamily();

		return	family == FamilyType::AOM ||
				family == FamilyType::DOM;
	}

	bool DeviceModule::isLogicModule() const
	{
		return moduleFamily() == FamilyType::LM;
	}

	bool DeviceModule::isFSCConfigurationModule() const
	{
		return moduleFamily() == FamilyType::LM || moduleFamily() == FamilyType::BVB;
	}

	bool DeviceModule::isOptoModule() const
	{
		return moduleFamily() == FamilyType::OCM;
	}

	bool DeviceModule::isBvb() const
	{
		return moduleFamily() == FamilyType::BVB;
	}


	//
	//
	// DeviceController
	//
	//
	DeviceController::DeviceController(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Controller, preset, parent)
	{
	}

	bool DeviceController::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceController* controllerMessage = message->mutable_deviceobject()->mutable_controller();

		Q_UNUSED(controllerMessage);
		//controllerMessage->set_startxdocpt(m_startXDocPt);
		//controllerMessage->set_startydocpt(m_startYDocPt);

		return true;
	}

	bool DeviceController::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
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
			Q_ASSERT(message.deviceobject().has_controller());
			return false;
		}

		const Proto::DeviceController& controllerMessage = message.deviceobject().controller();

		Q_UNUSED(controllerMessage);
		//x = controllerMessage.startxdocpt();
		//y = controllerMessage.startydocpt();

		return true;
	}

	//
	//
	// DeviceAppSignal
	//
	//
	DeviceAppSignal::DeviceAppSignal(bool preset /*= false*/, QObject* parent /*= nullptr*/) noexcept :
		DeviceObject(DeviceType::AppSignal, preset, parent)
	{
		// These properties are used in setType()
		// So they don take part in PropertyOnDemand
		//
		addProperty<E::AnalogAppSignalFormat, DeviceAppSignal, &DeviceAppSignal::appSignalDataFormat, &DeviceAppSignal::setAppSignalDataFormat>(PropertyNames::appSignalDataFormat, PropertyNames::categoryAppSignal, true)
				->setUpdateFromPreset(true)
				.setExpert(preset);

		addProperty<QString, DeviceAppSignal, &DeviceAppSignal::appSignalBusTypeId, &DeviceAppSignal::setAppSignalBusTypeId>(PropertyNames::appSignalBusTypeId, PropertyNames::categoryAppSignal, true)
				->setUpdateFromPreset(true)
				.setExpert(preset);

		// Show/Hide analog signal properties
		//
		setSignalType(signalType());

		return;
	}

	void DeviceAppSignal::propertyDemand(const QString& prop)
	{
		DeviceObject::propertyDemand(prop);

		auto typeProp = addProperty<E::SignalType, DeviceAppSignal, &DeviceAppSignal::signalType, &DeviceAppSignal::setSignalType>(PropertyNames::type, QLatin1String(), true);
		auto functionProp = addProperty<E::SignalFunction, DeviceAppSignal, &DeviceAppSignal::function, &DeviceAppSignal::setFunction>(PropertyNames::function, QLatin1String(), true);
		auto byteOrderProp = addProperty<E::ByteOrder, DeviceAppSignal, &DeviceAppSignal::byteOrder, &DeviceAppSignal::setByteOrder>(PropertyNames::byteOrder, QLatin1String(), true);
		auto formatProp = addProperty<E::DataFormat, DeviceAppSignal, &DeviceAppSignal::format, &DeviceAppSignal::setFormat>(PropertyNames::format, QLatin1String(), true);
		auto memoryAreaProp = addProperty<E::MemoryArea, DeviceAppSignal, &DeviceAppSignal::memoryArea, &DeviceAppSignal::setMemoryArea>(PropertyNames::memoryArea, QLatin1String(), true);

		auto sizeProp = addProperty<int, DeviceAppSignal, &DeviceAppSignal::size, &DeviceAppSignal::setSize>(PropertyNames::size, QLatin1String(), true);

		auto valueOffsetProp = addProperty<int, DeviceAppSignal, &DeviceAppSignal::valueOffset, &DeviceAppSignal::setValueOffset>(PropertyNames::valueOffset, QLatin1String(), true);
		auto valueBitProp = addProperty<int, DeviceAppSignal, &DeviceAppSignal::valueBit, &DeviceAppSignal::setValueBit>(PropertyNames::valueBit, QLatin1String(), true);

		auto validitySignalId = addProperty<QString, DeviceAppSignal, &DeviceAppSignal::validitySignalId, &DeviceAppSignal::setValiditySignalId>(PropertyNames::validitySignalId, QLatin1String(), true);

		auto signalSpecPropsStructProp = addProperty<QString, DeviceAppSignal, &DeviceAppSignal::signalSpecPropsStruct, &DeviceAppSignal::setSignalSpecPropsStruct>(PropertyNames::signalSpecificProperties, PropertyNames::categoryAppSignal, true);

		typeProp->setUpdateFromPreset(true);
		typeProp->setExpert(m_preset);

		functionProp->setUpdateFromPreset(true);
		functionProp->setExpert(m_preset);

		byteOrderProp->setUpdateFromPreset(true);
		byteOrderProp->setExpert(m_preset);

		formatProp->setUpdateFromPreset(true);
		formatProp->setExpert(m_preset);

		memoryAreaProp->setUpdateFromPreset(true);
		memoryAreaProp->setExpert(m_preset);

		sizeProp->setUpdateFromPreset(true);
		sizeProp->setExpert(m_preset);

		validitySignalId->setUpdateFromPreset(true);
		validitySignalId->setExpert(m_preset);

		valueOffsetProp->setUpdateFromPreset(true);
		valueOffsetProp->setExpert(m_preset);

		valueBitProp->setUpdateFromPreset(true);
		valueBitProp->setExpert(m_preset);

		signalSpecPropsStructProp->setUpdateFromPreset(true);
		signalSpecPropsStructProp->setExpert(m_preset);
		signalSpecPropsStructProp->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);

		return;
	}

	bool DeviceAppSignal::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::DeviceAppSignal* signalMessage = message->mutable_deviceobject()->mutable_appsignal();

		signalMessage->set_type(static_cast<int>(m_signalType));
		signalMessage->set_function(static_cast<int>(m_function));

		signalMessage->set_byteorder(static_cast<int>(m_byteOrder));
		signalMessage->set_format(static_cast<int>(m_format));
		signalMessage->set_memoryarea(static_cast<int>(m_memoryArea));

		signalMessage->set_size(static_cast<int>(m_size));

		signalMessage->set_valueoffset(static_cast<int>(m_valueOffset));
		signalMessage->set_valuebit(static_cast<int>(m_valueBit));

		signalMessage->set_validitysignalid(m_validitySignalId.toUtf8());

		signalMessage->set_appsignallowadc(m_appSignalLowAdc);
		signalMessage->set_appsignalhighadc(m_appSignalHighAdc);

		signalMessage->set_appsignallowengunits(m_appSignalLowEngUnits);
		signalMessage->set_appsignalhighengunits(m_appSignalHighEngUnits);

		signalMessage->set_appsignaldataformat(static_cast<int>(m_appSignalDataFormat));

		signalMessage->set_appsignalbustypeid(m_appSignalBusTypeId.toStdString());

		signalMessage->set_signalspecpropsstruct(m_signalSpecPropsStruct.toUtf8());
		signalMessage->set_signalspecpropsstructwasfixed(true);		// m_signalSpecPropsStruct was fixed on loading

		return true;
	}

	bool DeviceAppSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_appsignal() == false)
		{
			Q_ASSERT(message.deviceobject().has_appsignal());
			return false;
		}

		const Proto::DeviceAppSignal& signalMessage = message.deviceobject().appsignal();

		if (signalMessage.has_obsoletetype() == true)
		{
			Q_ASSERT(signalMessage.has_type() == false);
			Q_ASSERT(signalMessage.has_function() == false);

			Obsolete::SignalType obsoleteType = static_cast<Obsolete::SignalType>(signalMessage.obsoletetype());

			switch (obsoleteType)
			{
				case Obsolete::SignalType::DiagDiscrete:
					setSignalType(E::SignalType::Discrete);
					m_function = E::SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::DiagAnalog:
					setSignalType(E::SignalType::Analog);
					m_function = E::SignalFunction::Diagnostics;
					break;
				case Obsolete::SignalType::InputDiscrete:
					setSignalType(E::SignalType::Discrete);
					m_function = E::SignalFunction::Input;
					break;
				case Obsolete::SignalType::InputAnalog:
					setSignalType(E::SignalType::Analog);
					m_function = E::SignalFunction::Input;
					break;
				case Obsolete::SignalType::OutputDiscrete:
					setSignalType(E::SignalType::Discrete);
					m_function = E::SignalFunction::Output;
					break;
				case Obsolete::SignalType::OutputAnalog:
					setSignalType(E::SignalType::Analog);
					m_function = E::SignalFunction::Output;
					break;
				default:
					Q_ASSERT(false);
			}
		}
		else
		{
			setSignalType(static_cast<E::SignalType>(signalMessage.type()));				// Show hide some props
			m_function = static_cast<E::SignalFunction>(signalMessage.function());
		}

		m_byteOrder = static_cast<E::ByteOrder>(signalMessage.byteorder());
		m_format = static_cast<E::DataFormat>(signalMessage.format());
		m_memoryArea = static_cast<E::MemoryArea>(signalMessage.memoryarea());

		m_size = signalMessage.size();

		m_valueOffset = signalMessage.valueoffset();
		m_valueBit = signalMessage.valuebit();

		m_validitySignalId = QString::fromUtf8(signalMessage.validitysignalid().data());

		m_appSignalLowAdc = signalMessage.appsignallowadc();
		m_appSignalHighAdc = signalMessage.appsignalhighadc();

		m_appSignalLowEngUnits = signalMessage.appsignallowengunits();
		m_appSignalHighEngUnits =  signalMessage.appsignalhighengunits();

		m_appSignalDataFormat = static_cast<E::AnalogAppSignalFormat>(signalMessage.appsignaldataformat());

		m_appSignalBusTypeId = QString::fromStdString(signalMessage.appsignalbustypeid());

		m_signalSpecPropsStruct = QString::fromStdString(signalMessage.signalspecpropsstruct());

		if (signalMessage.signalspecpropsstructwasfixed() == false)
		{
			// RPCT-2622, RPCT-2621
			// Shit happens. We had a situaltion when misprinting was detected (EngEneering vs EngIneering).
			// To avoid manual replacement of this typo for non platform modules, the replace just made.
			//
			m_signalSpecPropsStruct = replaceEngeneeringToEngineering(m_signalSpecPropsStruct);
		}

		if (m_preset == true)
		{
			setExpertToProperty(PropertyNames::type, true);
			setExpertToProperty(PropertyNames::function, true);
			setExpertToProperty(PropertyNames::byteOrder, true);
			setExpertToProperty(PropertyNames::format, true);
			setExpertToProperty(PropertyNames::memoryArea, true);
			setExpertToProperty(PropertyNames::size, true);
			setExpertToProperty(PropertyNames::valueOffset, true);
			setExpertToProperty(PropertyNames::valueBit, true);
			setExpertToProperty(PropertyNames::validitySignalId, true);
			setExpertToProperty(PropertyNames::appSignalDataFormat, true);
			setExpertToProperty(PropertyNames::signalSpecificProperties, true);
		}

		return true;
	}

	void DeviceAppSignal::expandEquipmentId()
	{
		if (m_validitySignalId.isEmpty() == false)
		{
			if (hasParent() == true)
			{
				m_validitySignalId.replace(QLatin1String("$(PARENT)"), parent()->equipmentIdTemplate(), Qt::CaseInsensitive);
			}

			m_validitySignalId.replace(QLatin1String("$(PLACE)"), QString::number(place()).rightJustified(2, '0'), Qt::CaseInsensitive);
		}

		DeviceObject::expandEquipmentId();

		return;
	}

//	quint32 DeviceAppSignal::valueToMantExp1616(double value)
//	{
//		if (value == 0)
//			return 0;

//		//value = 2;

//		double m = 0;
//		int p = 1;

//		m = frexp (value, &p);

//		p+= 30;

//		if (abs((int)m) < 0x3fffffff)
//		{
//			while (abs((int)m) < 0x3fffffff)
//			{
//				m *= 2;
//				p--;
//			}

//			if ((int)m == -0x40000000)
//			{
//				m *= 2;
//				p--;
//			}
//		}
//		else
//		{
//			while (abs((int)m) > 0x20000000)
//			{
//				m /= 2;
//				p++;
//			}
//		}

//		if (p < -256 || p > 255)
//		{
//			return 0;
//		}

//		quint16 _m16 = (int)m >> 16;
//		quint16 _p16 = static_cast<quint16>(p);

//		quint32 result = (_m16 << 16) | _p16;
//		return result;
//	}

	E::SignalType DeviceAppSignal::signalType() const
	{
		return m_signalType;
	}

	void DeviceAppSignal::setSignalType(E::SignalType value)
	{
		m_signalType = value;

		if (function() == E::SignalFunction::Input ||
			function() == E::SignalFunction::Output)
		{
			bool analogSignalProps = false;
			bool busSignalProps = false;

			switch (m_signalType)
			{
			case E::SignalType::Analog:
				analogSignalProps = true;
				break;
			case E::SignalType::Discrete:
				break;
			case E::SignalType::Bus:
				busSignalProps = true;
				break;
			default:
				Q_ASSERT(false);
			}

			bool propertiesWereChanged = false;

			if (auto p = propertyByCaption(PropertyNames::appSignalDataFormat);
				p != nullptr &&
				p->visible() != analogSignalProps)
			{
				p->setVisible(analogSignalProps);
				propertiesWereChanged = true;
			}
			else
			{
				Q_ASSERT(p);
			}

			if (auto p = propertyByCaption(PropertyNames::appSignalBusTypeId);
				p != nullptr &&
				p->visible() != busSignalProps)
			{
				p->setVisible(busSignalProps);
				propertiesWereChanged = true;
			}
			else
			{
				Q_ASSERT(p);
			}

			if (propertiesWereChanged == true)
			{
				emit propertyListChanged();
			}
		}
	}

	E::SignalFunction DeviceAppSignal::function() const
	{
		return m_function;
	}

	void DeviceAppSignal::setFunction(E::SignalFunction value)
	{
		m_function = value;
	}

	E::ByteOrder DeviceAppSignal::byteOrder() const
	{
		return m_byteOrder;
	}

	void DeviceAppSignal::setByteOrder(E::ByteOrder value)
	{
		m_byteOrder = value;
	}

	E::DataFormat DeviceAppSignal::format() const
	{
		return m_format;
	}

	void DeviceAppSignal::setFormat(E::DataFormat value)
	{
		m_format = value;
	}

	E::MemoryArea DeviceAppSignal::memoryArea() const
	{
		return m_memoryArea;
	}

	void DeviceAppSignal::setMemoryArea(E::MemoryArea value)
	{
		m_memoryArea = value;
	}

	int DeviceAppSignal::size() const
	{
		return m_size;
	}

	void DeviceAppSignal::setSize(int value)
	{
		m_size = value;
	}

	int DeviceAppSignal::valueOffset() const
	{
		return m_valueOffset;
	}

	void DeviceAppSignal::setValueOffset(int value)
	{
		m_valueOffset = value;
	}

	int DeviceAppSignal::valueBit() const
	{
		return m_valueBit;
	}

	void DeviceAppSignal::setValueBit(int value)
	{
		m_valueBit = value;
	}

	QString DeviceAppSignal::validitySignalId() const
	{
		return m_validitySignalId;
	}

	void DeviceAppSignal::setValiditySignalId(QString value)
	{
		m_validitySignalId = value.trimmed();
	}

	bool DeviceAppSignal::isInputSignal() const
	{
		return m_function == E::SignalFunction::Input;
	}

	bool DeviceAppSignal::isOutputSignal() const
	{
		return m_function == E::SignalFunction::Output;
	}

	bool DeviceAppSignal::isDiagSignal() const
	{
		return m_function == E::SignalFunction::Diagnostics;
	}

	bool DeviceAppSignal::isValiditySignal() const
	{
		return m_function == E::SignalFunction::Validity;
	}

	bool DeviceAppSignal::isAnalogSignal() const
	{
		return m_signalType == E::SignalType::Analog;
	}

	bool DeviceAppSignal::isDiscreteSignal() const
	{
		return m_signalType == E::SignalType::Discrete;
	}

	int DeviceAppSignal::appSignalLowAdc() const
	{
		return m_appSignalLowAdc;
	}

	void DeviceAppSignal::setAppSignalLowAdc(int value)
	{
		m_appSignalLowAdc = value;
	}

	int DeviceAppSignal::appSignalHighAdc() const
	{
		return m_appSignalHighAdc;
	}

	void DeviceAppSignal::setAppSignalHighAdc(int value)
	{
		m_appSignalHighAdc = value;
	}

	double DeviceAppSignal::appSignalLowEngUnits() const
	{
		return m_appSignalLowEngUnits;
	}

	void DeviceAppSignal::setAppSignalLowEngUnits(double value)
	{
		m_appSignalLowEngUnits = value;
	}

	double DeviceAppSignal::appSignalHighEngUnits() const
	{
		return m_appSignalHighEngUnits;
	}

	void DeviceAppSignal::setAppSignalHighEngUnits(double value)
	{
		m_appSignalHighEngUnits = value;
	}

	E::AnalogAppSignalFormat DeviceAppSignal::appSignalDataFormat() const
	{
		return m_appSignalDataFormat;
	}

	void DeviceAppSignal::setAppSignalDataFormat(E::AnalogAppSignalFormat value)
	{
		m_appSignalDataFormat = value;
	}

	QString DeviceAppSignal::appSignalBusTypeId() const
	{
		return m_appSignalBusTypeId;
	}

	void DeviceAppSignal::setAppSignalBusTypeId(QString value)
	{
		m_appSignalBusTypeId = value;
	}

	QString DeviceAppSignal::signalSpecPropsStruct() const
	{
		return m_signalSpecPropsStruct;
	}

	void DeviceAppSignal::setSignalSpecPropsStruct(QString value)
	{
		m_signalSpecPropsStruct = value;
	}

	//
	//
	// Workstation
	//
	//
	Workstation::Workstation(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Workstation, preset, parent)
	{
		//auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, Workstation::type, Workstation::setType)
		//typeProp->setUpdateFromPreset(true);

		auto p = propertyByCaption(PropertyNames::equipmentIdTemplate);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}
	}

	bool Workstation::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::Workstation* workstationMessage = message->mutable_deviceobject()->mutable_workstation();

		workstationMessage->set_type(m_type);

		return true;
	}

	bool Workstation::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_workstation() == false)
		{
			Q_ASSERT(message.deviceobject().has_workstation());
			return false;
		}

		const Proto::Workstation& workstationMessage = message.deviceobject().workstation();

		m_type =  workstationMessage.type();

		return true;
	}

	int Workstation::type() const
	{
		return m_type;
	}

	void Workstation::setType(int value)
	{
		m_type = value;
	}


	//
	//
	// Software
	//
	//
	Software::Software(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Software, preset, parent)
	{
		ADD_PROPERTY_GETTER_SETTER(E::SoftwareType, PropertyNames::type, true, Software::softwareType, Software::setSoftwareType)
				->setExpert(true)
				.setUpdateFromPreset(true);
	}

	bool Software::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		Proto::Software* softwareMessage = message->mutable_deviceobject()->mutable_software();

		softwareMessage->set_type(static_cast<int>(m_softwareType));

		return true;
	}

	bool Software::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_software() == false)
		{
			Q_ASSERT(message.deviceobject().has_software());
			return false;
		}

		const Proto::Software& softwareMessage = message.deviceobject().software();

		m_softwareType =  static_cast<E::SoftwareType>(softwareMessage.type());

		return true;
	}

	E::SoftwareType Software::softwareType() const
	{
		return m_softwareType;
	}

	void Software::setSoftwareType(E::SoftwareType value)
	{
		m_softwareType = value;
	}


	//
	//
	// EquipmentSet
	//
	//
	EquipmentSet::EquipmentSet(std::shared_ptr<DeviceRoot> root)
	{
		set(root);
	}

	EquipmentSet::~EquipmentSet()
	{
		// Release m_root in separate thrtead, if it is possible
		//
		m_deviceTable.clear();	// Clear it first because it also holds m_root

		if (m_root.use_count() == 1)
		{
			std::shared_ptr<Hardware::DeviceObject> equipmentSharedPointer = m_root;
			m_root.reset();

			QtConcurrent::run(
				[](std::shared_ptr<Hardware::DeviceObject> deviceObject)
				{
					deviceObject.reset();
				},
				equipmentSharedPointer);
		}

		return;
	}

	void EquipmentSet::set(std::shared_ptr<DeviceRoot> root)
	{
		m_deviceTable.clear();

		if (root == nullptr)
		{
			Q_ASSERT(root);
			return;
		}

		m_root = root;

		// fill map for fast access
		//
		m_deviceTable.insert(m_root->equipmentIdTemplate(), m_root);
		addDeviceChildrenToHashTable(m_root);

		return;
	}

	std::shared_ptr<DeviceObject> EquipmentSet::deviceObject(const QString& equipmentId)
	{
		auto it = m_deviceTable.find(equipmentId);

		if (it != m_deviceTable.end())
		{
			return it.value();
		}
		else
		{
			return nullptr;
		}
	}

	const std::shared_ptr<DeviceObject> EquipmentSet::deviceObject(const QString& equipmentId) const
	{
		auto it = m_deviceTable.find(equipmentId);

		if (it != m_deviceTable.end())
		{
			return it.value();
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<DeviceRoot> EquipmentSet::root()
	{
		return m_root;
	}

	const std::shared_ptr<DeviceRoot> EquipmentSet::root() const
	{
		return m_root;
	}

	[[nodiscard]] std::vector<std::shared_ptr<DeviceObject>> EquipmentSet::devices()
	{
		std::vector<std::shared_ptr<DeviceObject>> result;
		result.reserve(m_deviceTable.size());

		for (auto d : m_deviceTable)
		{
			result.push_back(d);
		}

		return result;
	}

	void EquipmentSet::dump(bool dumpProps, QDebug d) const
	{
		if (m_root)
		{
			m_root->dump(dumpProps, true);
		}
		else
		{
			d << "EquipmentSet::root is empty";
		}

		return;
	}

	void EquipmentSet::addDeviceChildrenToHashTable(std::shared_ptr<DeviceObject> parent)
	{
		for (int i = 0; i < parent->childrenCount(); i++)
		{
			const std::shared_ptr<DeviceObject>& child = parent->child(i);
			m_deviceTable.insert(child->equipmentIdTemplate(), child);

			addDeviceChildrenToHashTable(child);
		}

		return;
	}

	void equipmentWalker(DeviceObject* currentDevice, std::function<void (DeviceObject*)> processBeforeChildren, std::function<void (DeviceObject*)> processAfterChildren)
	{
		if (currentDevice == nullptr)
		{
			Q_ASSERT(currentDevice != nullptr);

			QString msg = QString(QObject::tr("%1: DeviceObject null pointer!")).arg(__FUNCTION__);

			qDebug() << msg;
			return;
		}

		if (processBeforeChildren != nullptr)
		{
			processBeforeChildren(currentDevice);
		}

		int childrenCount = currentDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = currentDevice->child(i).get();

			equipmentWalker(device, processBeforeChildren, processAfterChildren);
		}

		if (processAfterChildren != nullptr)
		{
			processAfterChildren(currentDevice);
		}
	}

	void equipmentWalker(DeviceObject* currentDevice, std::function<void (DeviceObject*)> processBeforeChildren)
	{
		equipmentWalker(currentDevice, processBeforeChildren, nullptr);
	}

	void SerializeEquipmentFromXml(const QString& filePath, std::shared_ptr<Hardware::DeviceRoot>& deviceRoot)
	{
		QXmlStreamReader equipmentReader;
		QFile file(filePath);

		std::shared_ptr<DeviceObject> currentDevice;

		if (file.open(QIODevice::ReadOnly))
		{
			equipmentReader.setDevice(&file);

			while (!equipmentReader.atEnd())
			{
				QXmlStreamReader::TokenType token = equipmentReader.readNext();

				switch (token)
				{
				case QXmlStreamReader::StartElement:
				{
					const QXmlStreamAttributes& attr = equipmentReader.attributes();
					const QString classNameHash = attr.value("classNameHash").toString();
					if (classNameHash.isEmpty())
					{
						qDebug() << "Attribute classNameHash of DeviceObject not found";
						continue;
					}
					bool ok = false;
					quint32 hash = classNameHash.toUInt(&ok, 16);
					if (!ok)
					{
						qDebug() << QString("Could not interpret hash %s").arg(classNameHash);
						continue;
					}
					std::shared_ptr<Hardware::DeviceObject> deviceObject = Hardware::DeviceObjectFactory.Create(hash);
					if (deviceObject == nullptr)
					{
						qDebug() << QString("Unknown element %s found").arg(equipmentReader.name().toString());
						continue;
					}

					if (deviceObject->isRoot() == true)
					{
						currentDevice = deviceObject;
						deviceRoot = deviceObject->toRoot();
						continue;
					}

					if (currentDevice == nullptr)
					{
						qDebug() << "DeviceRoot should be the root xml element";
						return;
					}

					deviceObject->setSpecificProperties(attr.value("SpecificProperties").toString());

					for (auto p : deviceObject->properties())
					{
						Q_ASSERT(p);

						if (p->readOnly() || p->caption() == QLatin1String("SpecificProperties"))
						{
							continue;
						}

						QVariant tmp = QVariant::fromValue(attr.value(p->caption()).toString());
						bool result = tmp.convert(p->value().userType());
						if (result == false)
						{
							Q_ASSERT(tmp.canConvert(p->value().userType()));
						}
						else
						{
							p->setValue(tmp);
						}
					}

					currentDevice->addChild(deviceObject);
					currentDevice = deviceObject;
					break;
				}
				case QXmlStreamReader::EndElement:
					if (currentDevice != nullptr && currentDevice->isRoot() == false)
					{
						if (currentDevice->hasParent() == false)
						{
							Q_ASSERT(currentDevice->hasParent());
							break;
						}

						currentDevice = currentDevice->parent();
					}
					else
					{
						return;	// Closing root element, nothing to read left
					}
					break;
				default:
					continue;
				}
			}
			if (equipmentReader.hasError())
			{
				qDebug() << "Parse equipment.xml error";
			}
		}
	}

	QString expandDeviceSignalTemplate(	const Hardware::DeviceObject& startDeviceObject,
										const QString& templateStr,
										QString* errMsg)
	{
		if (errMsg == nullptr)
		{
			Q_ASSERT(false);
			return QString("Null pointer");
		}

		QString resultStr;

		int searchStartPos = 0;

		do
		{
			int macroStartPos = templateStr.indexOf(TemplateMacro::START_TOKEN, searchStartPos);

			if (macroStartPos == -1)
			{
				// no more macroses
				//
				resultStr += templateStr.mid(searchStartPos);
				break;
			}

			resultStr += templateStr.mid(searchStartPos, macroStartPos - searchStartPos);

			int macroEndPos = templateStr.indexOf(TemplateMacro::END_TOKEN, macroStartPos + 2);

			if (macroEndPos == -1)
			{
				*errMsg = QString("End of macro is not found in template %1 of device object %2. ").
							arg(templateStr).arg(startDeviceObject.equipmentIdTemplate());
				return QString();
			}

			QString macroStr = templateStr.mid(macroStartPos + 2, macroEndPos - (macroStartPos + 2));

			QString expandedMacroStr = expandDeviceObjectMacro(startDeviceObject, macroStr, errMsg);

			if (errMsg->isEmpty() == false)
			{
				return QString();
			}

			resultStr += expandedMacroStr;

			searchStartPos = macroEndPos + 1;
		}
		while(true);

		return resultStr;
	}

	QString expandDeviceObjectMacro(const Hardware::DeviceObject& startDeviceObject,
									const QString& macroStr,
									QString* errMsg)
	{
		if (errMsg == nullptr)
		{
			Q_ASSERT(false);
			return QString("Null pointer");
		}

		QStringList macroFields = macroStr.split(".");

		const Hardware::DeviceObject* deviceObject = nullptr;
		QString propertyCaption;

		switch(macroFields.count())
		{
		case 1:
			{
				// property only
				//
				deviceObject = &startDeviceObject;
				propertyCaption = macroFields.at(0);
			}
			break;

		case 2:
			{
				// parentObject.property
				//
				QString parentObjectType = macroFields.at(0);
				propertyCaption = macroFields.at(1);

				deviceObject = getParentDeviceObjectOfType(startDeviceObject, parentObjectType, errMsg);

				if (errMsg->isEmpty() == false)
				{
					return QString();
				}

				if (deviceObject == nullptr)
				{
					*errMsg = QString("Macro expand error! Parent device object of type '%1' is not found for device object %2").
									arg(parentObjectType).arg(startDeviceObject.equipmentIdTemplate());
					return QString();
				}

			}
			break;

		default:
			*errMsg = QString("Unknown format of macro %1 in template of device signal %2").
					arg(macroStr).arg(startDeviceObject.equipmentIdTemplate());
			return QString();
		}

		if (deviceObject->propertyExists(propertyCaption) == false)
		{
			*errMsg = QString("Device signal %1 macro expand error! Property '%2' is not found in device object %3.").
								arg(startDeviceObject.equipmentIdTemplate()).
								arg(propertyCaption).
								arg(deviceObject->equipmentIdTemplate());
			return QString();
		}

		QString propertyValue = deviceObject->propertyValue(propertyCaption).toString();

		return propertyValue;
	}

	const Hardware::DeviceObject* getParentDeviceObjectOfType(const Hardware::DeviceObject& startObject,
															  const QString& parentObjectType,
															  QString* errMsg)
	{
		if (errMsg == nullptr)
		{
			Q_ASSERT(false);
			return nullptr;
		}

		static const std::map<QString, Hardware::DeviceType> objectTypes {
				std::make_pair(QString("root"), Hardware::DeviceType::Root),
				std::make_pair(QString("system"), Hardware::DeviceType::System),
				std::make_pair(QString("rack"), Hardware::DeviceType::Rack),
				std::make_pair(QString("chassis"), Hardware::DeviceType::Chassis),
				std::make_pair(QString("module"), Hardware::DeviceType::Module),
				std::make_pair(QString("workstation"), Hardware::DeviceType::Workstation),
				std::make_pair(QString("software"), Hardware::DeviceType::Software),
				std::make_pair(QString("controller"), Hardware::DeviceType::Controller),
				std::make_pair(QString("signal"), Hardware::DeviceType::AppSignal),
		};

		std::map<QString, Hardware::DeviceType>::const_iterator it = objectTypes.find(parentObjectType.toLower());

		if (it == objectTypes.end())
		{
			*errMsg = QString("Unknown object type '%1' in call of getParentObjectOfType(...) for device object %2").
							arg(parentObjectType).arg(startObject.equipmentIdTemplate());
			return nullptr;
		}

		Hardware::DeviceType requestedDeviceType = it->second;

		const Hardware::DeviceObject* parent = &startObject;

		do
		{
			if (parent == nullptr)
			{
				break;
			}

			if (parent->deviceType() == requestedDeviceType)
			{
				return parent;
			}

			parent = parent->parent().get();
		}
		while(true);

		return nullptr;
	}
}
