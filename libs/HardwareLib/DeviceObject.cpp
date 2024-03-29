#ifndef HARDWARE_LIB_DOMAIN
#error Do not include this file in the project! Link HardwareLib instead.
#endif

#include "DeviceObjectFactory.h"
#include "../Proto/ProtoCommonHelper.h"
#include "../lib/ConstStrings.h"

#include "./include/HardwareLib/DeviceObject.h"
#include "./include/HardwareLib/PropertyNames.h"
#include "./include/HardwareLib/ScriptDeviceObject.h"

#include "./include/HardwareLib/DeviceRoot.h"
#include "./include/HardwareLib/DeviceSystem.h"
#include "./include/HardwareLib/DeviceRack.h"
#include "./include/HardwareLib/DeviceChassis.h"
#include "./include/HardwareLib/DeviceModule.h"
#include "./include/HardwareLib/DeviceController.h"
#include "./include/HardwareLib/DeviceAppSignal.h"
#include "./include/HardwareLib/DiagSignal.h"
#include "./include/HardwareLib/Workstation.h"
#include "./include/HardwareLib/Software.h"

#include <QtConcurrent>

namespace Hardware
{
	const std::array<QString, 10> DeviceObjectExtensions =
		{
			".hrt",			// DeviceRoot
			".hsm",			// DeviceSystem
			".hrk",			// DeviceRack
			".hcs",			// DeviceChassis
			".hmd",			// DeviceModule
			".hws",			// Workstation
			".hsw",			// Software
			".hcr",			// DeviceController
			".hds",			// DeviceAppSignal
			".hsd",			// DiagSignal
		};

	extern const std::array<QString, 10> DeviceTypeNames =
		{
			"Root",			// DeviceRoot
			"System",		// DeviceSystem
			"Rack",			// DeviceRack
			"Chassis",		// DeviceChassis
			"Module",		// DeviceModule
			"Workstation",	// Workstation
			"Software",		// Software
			"Controller",	// DeviceController
			"AppSignal",	// DeviceAppSignal
			"DiagSignal",	// DiagSignal
		};

	
	//
	//
	// DeviceObject
	//
	//
	DeviceObject::DeviceObject(DeviceType deviceType, bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		PropertyObject(parent),
		m_deviceType(deviceType)
	{
		ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::excludeFromBuild, true, DeviceObject::excludeFromBuild, DeviceObject::setExcludeFromBuild)
			->setUpdateFromPreset(false);

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

		auto specificProp = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::specificProperties, true, DeviceObject::specificPropertiesStruct, DeviceObject::setSpecificPropertiesStruct);
		specificProp->setExpert(true);
		specificProp->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);

		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::tags, true, DeviceObject::tagsAsString, DeviceObject::setTags)
			->setDescription(PropertyNames::tagsDescription)
			.setSpecificEditor(E::PropertySpecificEditor::Tags);

		auto presetProp = ADD_PROPERTY_GETTER(bool, PropertyNames::preset, true, DeviceObject::isPreset);
		presetProp->setExpert(true);

		setPreset(preset);

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
		bool ok = SaveData(message, false, {});
		return ok;
	}

	bool DeviceObject::SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = ::ClassNameHashCode(className);

		message->set_classnamehash(classnamehash);

		auto mutableDeviceObject = message->MutableExtension(::Proto::deviceobject);

		Proto::Write(mutableDeviceObject->mutable_uuid(), m_uuid);
		Proto::Write(mutableDeviceObject->mutable_equipmentid(), m_equipmentId);
		Proto::Write(mutableDeviceObject->mutable_caption(), m_caption);

		mutableDeviceObject->set_excludefrombuild(m_excludeFromBuild);
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

		// Save tags.
		//
		for (const auto& tag : m_tags)
		{
			mutableDeviceObject->add_tags(tag.second.toStdString());
		}

		// --
		//
		if (m_preset == true)
		{
			mutableDeviceObject->set_preset(m_preset);

			mutableDeviceObject->set_presetroot(presetRoot());
			mutableDeviceObject->set_presetversion(presetVersion());
			mutableDeviceObject->set_presetprotectedproperties(presetProtectedPropertiesStr().toStdString());

			Proto::Write(mutableDeviceObject->mutable_presetname(), m_presetName);
			Proto::Write(mutableDeviceObject->mutable_presetobjectuuid(), m_presetObjectUuid);
		}

		// Save children if it is necessary (can be in serialization for the clipboard)
		//
		if (saveTree == true)
		{
			for (const std::shared_ptr<DeviceObject>& child : m_children)
			{
				if (predicate && predicate(*child) == false)
				{
					// If predicate is not present or it returns false, do not save this child.
					//
					continue;
				}

				::Proto::Envelope* childMessage = mutableDeviceObject->add_children();
				Q_ASSERT(childMessage);

				child->SaveData(childMessage, saveTree, predicate);
			}
		}

		return true;
	}

	bool DeviceObject::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::deviceobject));
			return false;
		}

		const auto& deviceobject = message.GetExtension(::Proto::deviceobject);

		m_uuid = Proto::Read(deviceobject.uuid());
		Q_ASSERT(m_uuid.isNull() == false || deviceType() == DeviceType::Root);

		Proto::Read(deviceobject.equipmentid(), &m_equipmentId);
		Proto::Read(deviceobject.caption(), &m_caption);

		m_excludeFromBuild = deviceobject.excludefrombuild();
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

		m_specificPropertiesStruct = QString::fromStdString(deviceobject.specific_properties_struct());
		parseSpecificPropertiesStruct(m_specificPropertiesStruct);

		// Load tags.
		//
		{
			QStringList tags;
			tags.reserve(deviceobject.tags_size());

			for (const std::string& tag : deviceobject.tags())
			{
				tags.push_back(QString::fromStdString(tag));
			}

			setTags(tags);
		}

		// Load specific properties' values. They are already exists after calling parseSpecificPropertiesStruct()
		//
		std::vector<std::shared_ptr<Property>> specificProps = PropertyObject::specificProperties();

		for (const ::Proto::Property& p :  deviceobject.properties())
		{
			auto it = std::find_if(specificProps.begin(), specificProps.end(),
				[p](const std::shared_ptr<Property>& dp)
				{
					return dp->caption().toStdString() == p.name();
				});

			if (it == specificProps.end())
			{
				qDebug() << "ERROR: Can't find property " << p.name().c_str() << " in" << m_equipmentId;
			}
			else
			{
				Property* property = it->get();

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
			setPreset(deviceobject.preset());

			setPresetRoot(deviceobject.presetroot());
			setPresetVersion(deviceobject.presetversion());
			setPresetProtectedPropertiesStr(QString::fromStdString(deviceobject.presetprotectedproperties()));

			Proto::Read(deviceobject.presetname(), &m_presetName);

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
		if (message.HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::deviceobject));
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<DeviceObject> deviceObject = s_deviceObjectFactory.Create(classNameHash);

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
		// Empty predicate means that all objects should be saved
		//
		return SaveObjectTreeIf(message, {});
	}

	bool DeviceObject::SaveObjectTreeIf(Proto::Envelope* message, std::function<bool(const DeviceObject&)> predicate) const
	{
		// Empty predicate means that all objects should be saved
		//
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		try
		{
			if (predicate && predicate(*this) == false)
			{
				// Still return true as it is not an error.
				//
				return true;
			}

			bool ok = this->SaveData(message, true, predicate);
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

	bool DeviceObject::hasParent() const
	{
		return m_parent.expired() == false;
	}

	std::shared_ptr<DeviceObject> DeviceObject::parent()
	{
		return m_parent.lock();
	}

	const std::shared_ptr<DeviceObject> DeviceObject::parent() const
	{
		return m_parent.lock();
	}

	DeviceType DeviceObject::deviceType() const
	{
		return m_deviceType;
	}

	QString DeviceObject::deviceTypeName() const
	{
		Q_ASSERT(static_cast<size_t>(m_deviceType) < std::size(Hardware::DeviceTypeNames));
		return DeviceTypeNames[static_cast<size_t>(m_deviceType)];
	}

	QString DeviceObject::deviceTypeName(DeviceType type)
	{
		Q_ASSERT(static_cast<size_t>(type) < std::size(Hardware::DeviceTypeNames));
		return DeviceTypeNames[static_cast<size_t>(type)];
	}

	bool DeviceObject::isRoot() const
	{
		return deviceType() == DeviceType::Root;
	}

	bool DeviceObject::isSystem() const
	{
		return deviceType() == DeviceType::System;
	}

	bool DeviceObject::isRack() const
	{
		return deviceType() == DeviceType::Rack;
	}

	bool DeviceObject::isChassis() const
	{
		return deviceType() == DeviceType::Chassis;
	}

	bool DeviceObject::isModule() const
	{
		return deviceType() == DeviceType::Module;
	}

	bool DeviceObject::isController() const
	{
		return deviceType() == DeviceType::Controller;
	}

	bool DeviceObject::isWorkstation() const
	{
		return deviceType() == DeviceType::Workstation;
	}

	bool DeviceObject::isSoftware() const
	{
		return deviceType() == DeviceType::Software;
	}

	bool DeviceObject::isAppSignal() const
	{
		return deviceType() == DeviceType::AppSignal;
	}

	bool DeviceObject::isDiagSignal() const 
	{
		return deviceType() == DeviceType::DiagSignal;
	}

	std::shared_ptr<const DeviceRoot> DeviceObject::toRoot() const
	{
		Q_ASSERT(isRoot());
		return toType<DeviceRoot>();
	}

	std::shared_ptr<Hardware::DeviceRoot> DeviceObject::toRoot()
	{
		return toType<DeviceRoot>();
	}

	std::shared_ptr<const DeviceSystem> DeviceObject::toSystem() const
	{
		return toType<DeviceSystem>();
	}

	std::shared_ptr<DeviceSystem> DeviceObject::toSystem()
	{
		return toType<DeviceSystem>();
	}

	std::shared_ptr<const DeviceRack> DeviceObject::toRack() const
	{
		return toType<DeviceRack>();
	}

	std::shared_ptr<DeviceRack> DeviceObject::toRack()
	{
		return toType<DeviceRack>();
	}

	std::shared_ptr<const DeviceChassis> DeviceObject::toChassis() const
	{
		return toType<DeviceChassis>();
	}

	std::shared_ptr<DeviceChassis> DeviceObject::toChassis()
	{
		return toType<DeviceChassis>();
	}

	std::shared_ptr<const DeviceModule> DeviceObject::toModule() const
	{
		return toType<const DeviceModule>();
	}

	std::shared_ptr<DeviceModule> DeviceObject::toModule()
	{
		return toType<DeviceModule>();
	}

	std::shared_ptr<const DeviceController> DeviceObject::toController() const
	{
		return toType<const DeviceController>();
	}

	std::shared_ptr<DeviceController> DeviceObject::toController()
	{
		return toType<DeviceController>();
	}

	std::shared_ptr<const DeviceAppSignal> DeviceObject::toAppSignal() const
	{
		return toType<const DeviceAppSignal>();
	}

	std::shared_ptr<DeviceAppSignal> DeviceObject::toAppSignal()
	{
		return toType<DeviceAppSignal>();
	}

	std::shared_ptr<const Hardware::DiagSignal> DeviceObject::toDiagSignal() const
	{
		return toType<const Hardware::DiagSignal>();
	}

	std::shared_ptr<Hardware::DiagSignal> DeviceObject::toDiagSignal()
	{
		return toType<Hardware::DiagSignal>();
	}

	std::shared_ptr<const Workstation> DeviceObject::toWorkstation() const
	{
		return toType<const Workstation>();
	}

	std::shared_ptr<Workstation> DeviceObject::toWorkstation()
	{
		return toType<Workstation>();
	}

	std::shared_ptr<const Software> DeviceObject::toSoftware() const
	{
		return toType<const Software>();
	}

	std::shared_ptr<Software> DeviceObject::toSoftware()
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

	const Hardware::Software* DeviceObject::getParentSoftware() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isSoftware())
			{
				return deviceObject->toSoftware().get();
			}
		}
		while(deviceObject != nullptr);

		return nullptr;
	}

	const Hardware::Workstation* DeviceObject::getParentWorkstation() const
	{
		const Hardware::DeviceObject* deviceObject = this;

		do
		{
			deviceObject = deviceObject->parent().get();

			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isWorkstation())
			{
				return deviceObject->toWorkstation().get();
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

	std::shared_ptr<const Hardware::DeviceChassis> DeviceObject::getParentChassisShared() const
	{
		std::shared_ptr<const Hardware::DeviceObject> deviceObject = parent();

		do
		{
			if (deviceObject == nullptr)
			{
				break;
			}

			if (deviceObject->isChassis())
			{
				return deviceObject->toChassis();
			}

			deviceObject = deviceObject->parent();
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

	std::shared_ptr<const DeviceObject> DeviceObject::childByEquipmentId(const QString& id) const
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
			return childType == DeviceType::Controller ||
				   childType == DeviceType::DiagSignal;
		}

		if (deviceType() == DeviceType::Workstation)
		{
			return childType == DeviceType::Software ||
				   childType == DeviceType::DiagSignal;
		}

		if (deviceType() == DeviceType::AppSignal &&
			childType == DeviceType::DiagSignal)
		{
			return false;
		}

		if (deviceType() >= childType)
		{
			return false;
		}

		if (childType == DeviceType::Workstation &&
			deviceType() > DeviceType::Module)
		{
			return false;
		}

		if (childType == DeviceType::AppSignal && parent() != nullptr && parent()->isSoftware() == true)
		{
			// Cannot add app signal to software or software\controller
			//
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
		return boolResult;
	}

	QString DeviceObject::replaceEngeneeringToEngineering(const QString& data)
	{
		QString result = data;

		result.replace(QLatin1String("engeneering"), QLatin1String("engineering"), Qt::CaseSensitive);
		result.replace(QLatin1String("Engeneering"), QLatin1String("Engineering"), Qt::CaseSensitive);

		return result;
	}

	bool DeviceObject::isExcludedFromBuild() const
	{
		return m_excludeFromBuild || (parent() ? parent()->isExcludedFromBuild() : false);
	}

	bool DeviceObject::excludeFromBuild() const
	{
		return m_excludeFromBuild;
	}

	void DeviceObject::setExcludeFromBuild(bool exclude)
	{
		m_excludeFromBuild = exclude;
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
			m_equipmentId.replace(QRegularExpression(QStringLiteral("[^a-zA-Z0-9#$_()]")), QStringLiteral("#"));
		}
	}

	QString DeviceObject::equipmentId() const
	{
		if (equipmentIdTemplate().contains(QChar('$')) == false)
		{
			// m_equipmentId does not have any macro variables, just return it
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

	QString DeviceObject::specificPropertiesStruct() const
	{
		return m_specificPropertiesStruct;
	}

	void DeviceObject::setSpecificPropertiesStruct(QString value)
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

	bool DeviceObject::hasTag(const QString& tag) const
	{
		return hasTag(::calcHash(tag));
	}

	bool DeviceObject::hasTag(Hash tagHash) const
	{
		return m_tags.contains(tagHash);
	}

	QStringList DeviceObject::tags() const
	{
		QStringList result;
		result.reserve(m_tags.size());

		for (const auto& pair : m_tags)
		{
			result.push_back(pair.second);
		}

		result.sort();

		return result;
	}

	QString DeviceObject::tagsAsString() const
	{
		return tags().join(" ");
	}
	
	void DeviceObject::setTags(const QStringList& tags)
	{
		m_tags.clear();
		m_tags.reserve(tags.size());

		for (const QString& tag : tags)
		{
			m_tags[::calcHash(tag)] = tag;
		};

		return;
	}

	void DeviceObject::setTags(const QString& tags)
	{
		auto list = tags.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
		return setTags(list);
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

	bool DeviceObject::isPreset() const
	{
		return m_preset;
	}

	void DeviceObject::setPreset(bool isPreset)
	{
		m_preset = isPreset;

		if (m_preset == true)
		{
			Property* p = ADD_PROPERTY_GETTER(bool, PropertyNames::presetRoot, true, DeviceObject::presetRoot);
			p->setExpert(true);

			p = ADD_PROPERTY_GETTER(QUuid, PropertyNames::presetObjectUuid, true, DeviceObject::presetObjectUuid);
			p->setExpert(true);

			p = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::presetName, true, DeviceObject::presetName, DeviceObject::setPresetName);
			p->setExpert(true);

			p = ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::presetProtectedProperties, true, DeviceObject::presetProtectedPropertiesStr, DeviceObject::setPresetProtectedPropertiesStr);
			p->setExpert(true);
			p->setUpdateFromPreset(false);
			p->setDescription(PropertyNames::presetProtectedPropertiesDescription);
		}
		else
		{
			removeProperty(PropertyNames::presetRoot);
			removeProperty(PropertyNames::presetObjectUuid);
			removeProperty(PropertyNames::presetName);
			removeProperty(PropertyNames::presetProtectedProperties);
		}
	}

	bool DeviceObject::presetRoot() const
	{
		return m_presetRoot;
	}

	void DeviceObject::setPresetRoot(bool value)
	{
		m_presetRoot = value;

		if (m_presetRoot == true)
		{
			Property* p = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::presetVersion, true, DeviceObject::presetVersion, DeviceObject::setPresetVersion);
			p->setUpdateFromPreset(true);
			p->setExpert(true);
		}
		else
		{
			removeProperty(PropertyNames::presetVersion);
		}

		return;
	}

	int DeviceObject::presetVersion() const
	{
		return m_presetVersion;
	}

	void DeviceObject::setPresetVersion(int value)
	{
		m_presetVersion = value;
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

	QString DeviceObject::presetProtectedPropertiesStr() const
	{
		return m_presetProtectedProperties.join(", ");
	}

	void DeviceObject::setPresetProtectedPropertiesStr(const QString& value)
	{
		// Split by comma, semicolon, return or space, remove empty parts.
		//
		m_presetProtectedProperties	= value.split(QRegularExpression(QStringLiteral("[,;\\n\\r\\s]")), Qt::SkipEmptyParts);
	}

	const QStringList& DeviceObject::presetProtectedProperties() const
	{
		return m_presetProtectedProperties;
	}

	void DeviceObject::setPresetProtectedProperties(const QStringList& value)
	{
		m_presetProtectedProperties = value;
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
					std::shared_ptr<Hardware::DeviceObject> deviceObject = s_deviceObjectFactory.Create(hash);
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

					deviceObject->setSpecificPropertiesStruct(attr.value("SpecificProperties").toString());

					for (auto p : deviceObject->properties())
					{
						Q_ASSERT(p);

						if (p->readOnly() || p->caption() == QLatin1String("SpecificProperties"))
						{
							continue;
						}

						QVariant tmp = QVariant::fromValue(attr.value(p->caption()).toString());
						bool result = tmp.convert(p->value().metaType());
						if (result == false)
						{
							Q_ASSERT(tmp.canConvert(p->value().metaType()));
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

		qsizetype searchStartPos = 0;

		do
		{
			qsizetype macroStartPos = templateStr.indexOf(TemplateMacro::START_TOKEN, searchStartPos);

			if (macroStartPos == -1)
			{
				// no more macroses
				//
				resultStr += templateStr.mid(searchStartPos);
				break;
			}

			resultStr += templateStr.mid(searchStartPos, macroStartPos - searchStartPos);

			qsizetype macroEndPos = templateStr.indexOf(TemplateMacro::END_TOKEN, macroStartPos + 2);

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
