#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/PropertyNames.h>

namespace Hardware
{
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

	bool DeviceModule::SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const
	{
		bool result = DeviceObject::SaveData(message, saveTree, predicate);
		if (result == false || message->HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->HasExtension(::Proto::deviceobject));
			return false;
		}

		// --
		//
		auto moduleMessage = message->MutableExtension(::Proto::deviceobject)->mutable_module();

		moduleMessage->set_moduletype(static_cast<int>(m_type));
		moduleMessage->set_custommodulefamily(m_customModuleFamily);
		moduleMessage->set_configurationscript(m_configurationScript.toStdString());
		moduleMessage->set_rawdatadescription(m_rawDataDescription.toStdString());

		return true;
	}

	bool DeviceModule::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::deviceobject));
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		const auto& deviceobject = message.GetExtension(::Proto::deviceobject);
		if (deviceobject.has_module() == false)
		{
			Q_ASSERT(deviceobject.has_module());
			return false;
		}

		auto&& modulemessage = deviceobject.module();

		if (modulemessage.has_moduletype())
		{
			m_type = static_cast<decltype(m_type)>(modulemessage.moduletype());
		}
		else
		{
			m_type = static_cast<decltype(m_type)>(modulemessage.typeobsolete());

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

	QString DeviceModule::moduleFamilyStr() const
	{
		return E::valueToString(moduleFamily());
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
		return moduleFamily() == FamilyType::LM ||
			   moduleFamily() == FamilyType::VDU ||
			   moduleFamily() == FamilyType::BVB ||
			   moduleFamily() == FamilyType::MSO;
	}


	bool DeviceModule::isOptoModule() const
	{
		return moduleFamily() == FamilyType::OCM;
	}

	bool DeviceModule::isBvb() const
	{
		return moduleFamily() == FamilyType::BVB;
	}

	bool DeviceModule::isMso() const
	{
		return moduleFamily() == FamilyType::MSO;
	}

	bool DeviceModule::isNonPlatformAppDataSourceModule() const
	{
		return isBvb() || isMso();
	}

	bool DeviceModule::isAppDataSourceModule() const
	{
		return isLogicModule() || isNonPlatformAppDataSourceModule();
	}

	bool DeviceModule::isVdu() const
	{
		return moduleFamily() == FamilyType::VDU;
	}

	bool DeviceModule::hasSubsystem() const
	{
		return moduleFamily() == FamilyType::LM ||
			   moduleFamily() == FamilyType::BVB ||
			   moduleFamily() == FamilyType::MSO;
	}
} // namespace Hardware
