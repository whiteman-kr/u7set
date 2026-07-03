#include <VFrame30/ActuatorHeader.h>

#include <HardwareLib/Subsystem.h>
#include <VFrame30/PropertyNames.h>


namespace VFrame30
{
	//
	// ActuatorSignal
	//
	ActuatorSignal::ActuatorSignal()
	{
		init();
		return;
	}

	ActuatorSignal::ActuatorSignal(const ActuatorSignal& other)
	{
		Proto::ActuatorSignal message;

		other.save(message);
		load(message);

		init();
		return;
	}

	void ActuatorSignal::init()
	{
		removeAllProperties();

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::signalId,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::signalId,
								 ActuatorSignal::setSignalId)
			->setDescription(tr("SignalID for the actuator type. Only alphanumeric characters and underscores are allowed."))
			.setValidator("[A-Za-z0-9_]+")
			.setViewOrder(1);

		ADD_PROPERTY_GET_SET_CAT(E::SignalType,
								 PropertyNames::signalType,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::signalType,
								 ActuatorSignal::setSignalType)
			->setViewOrder(2);

		ADD_PROPERTY_GET_SET_CAT(E::AnalogAppSignalFormat,
								 PropertyNames::analogFormat,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::analogFormat,
								 ActuatorSignal::setAnalogFormat)
			->setDescription(tr("Analog signal format for the actuator type if applicable."))
			.setViewOrder(3);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::busTypeId,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::busTypeId,
								 ActuatorSignal::setBusTypeId)
			->setDescription(tr("BusTypeID for the actuator type if applicable."))
			.setViewOrder(4);

		return;
	}

	void ActuatorSignal::save(Proto::ActuatorSignal& message) const
	{
		message.set_signalid(m_data.signalId.toStdString());
		message.set_signaltype(static_cast<int>(m_data.signalType));
		message.set_analogformat(static_cast<int>(m_data.analogFormat));
		message.set_bustypeid(m_data.busTypeId.toStdString());
	}

	void ActuatorSignal::load(const Proto::ActuatorSignal& message)
	{
		m_data.signalId = QString::fromStdString(message.signalid());
		m_data.signalType = static_cast<E::SignalType>(message.signaltype());
		m_data.analogFormat = static_cast<E::AnalogAppSignalFormat>(message.analogformat());
		m_data.busTypeId = QString::fromStdString(message.bustypeid());
	}

	QString ActuatorSignal::signalId() const
	{
		return m_data.signalId;
	}

	void ActuatorSignal::setSignalId(const QString& signalId)
	{
		m_data.signalId = signalId;
	}

	E::SignalType ActuatorSignal::signalType() const
	{
		return m_data.signalType;
	}

	void ActuatorSignal::setSignalType(E::SignalType signalType)
	{
		m_data.signalType = signalType;
	}

	E::AnalogAppSignalFormat ActuatorSignal::analogFormat() const
	{
		return m_data.analogFormat;
	}

	void ActuatorSignal::setAnalogFormat(E::AnalogAppSignalFormat analogFormat)
	{
		m_data.analogFormat = analogFormat;
	}

	QString ActuatorSignal::busTypeId() const
	{
		return m_data.busTypeId;
	}

	void ActuatorSignal::setBusTypeId(const QString& busTypeId)
	{
		m_data.busTypeId = busTypeId;
	}

	//
	// ActuatorHeader
	//
	ActuatorHeader::ActuatorHeader(QObject* parent) :
		PropertyObject(parent)
	{
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::ActuatorTypeId,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::actuatorTypeId,
								 ActuatorHeader::setActuatorTypeId)
			->setDescription(tr("Unique identifier for the actuator type. Only alphanumeric characters and underscores are allowed."))
			.setValidator("[A-Za-z0-9_]+")
			.setViewOrder(1);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::caption,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::caption,
								 ActuatorHeader::setCaption)
			->setDescription(tr("Caption for the actuator type."))
			.setViewOrder(2);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::description,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::description,
								 ActuatorHeader::setDescription)
			->setDescription(tr("Description for the actuator type."))
			.setViewOrder(3);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::acmPreset,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::acmPresetName,
								 ActuatorHeader::setAcmPresetName)
			->setDescription(tr("ACM preset name for the actuator type."))
			.setViewOrder(4);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::descriptionFile,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::descriptionFile,
								 ActuatorHeader::setDescriptionFile)
			->setDescription(tr("Description file for the actuator type."))
			.setViewOrder(5);

		// Subsystem
		//
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::subsystemId,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::subsystemId,
								 ActuatorHeader::setSubsystemId)
			->setDescription(tr("SubsystemID for the actuator type."))
			.setViewOrder(6);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::lmNumber,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::lmNumber,
								 ActuatorHeader::setLmNumber)
			->setDescription(tr("LM number in subsystem for the actuator type."))
			.setViewOrder(7);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::excludeFromBuild,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorHeader::excludeFromBuild,
								 ActuatorHeader::setExcludeFromBuild)
			->setDescription(tr("Exclude the actuator type from the build process."))
			.setViewOrder(8);


		addProperty<PropertyVector<ActuatorSignal>>(
			PropertyNames::inputs,
			PropertyNames::commonCategory,
			true,
			[this]()
			{
				return m_inputs;
			},
			[this](const auto& v)
			{
				if (m_inputs != v)
				{
					m_modified = true;
					m_inputs = v;
				}
			})
			->setDescription(tr("Input signals for the actuator type."))
			.setViewOrder(9);

		addProperty<PropertyVector<ActuatorSignal>>(
			PropertyNames::outputs,
			PropertyNames::commonCategory,
			true,
			[this]()
			{
				return m_outputs;
			},
			[this](const auto& v)
			{
				if (m_outputs != v)
				{
					m_modified = true;
					m_outputs = v;
				}
			})
			->setDescription(tr("Output signals for the actuator type."))
			.setViewOrder(10);

		ADD_PROPERTY_GETTER(int, QStringLiteral("Version"), true, ActuatorHeader::version)->setViewOrder(100);

		return;
	}

	std::shared_ptr<ActuatorHeader> ActuatorHeader::CreateObject(const Proto::Envelope& message)
	{
		auto result = std::make_shared<ActuatorHeader>();
		if (result->LoadData(message) == false)
		{
			result.reset();
			return result;
		}

		return result;
	}

	bool ActuatorHeader::SaveData(::Proto::Envelope* message) const
	{
		message->set_classnamehash(::ClassNameHashCode("ActuatorHeader"));

		if (m_modified == true)
		{
			m_version++;
			m_modified = false;
		}

		auto* m = message->MutableExtension(::Proto::actuatorHeader);
		return SaveData(m);
	}

	bool ActuatorHeader::LoadData(const ::Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::actuatorHeader) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::actuatorHeader));
			return false;
		}

		m_modified = false;

		const auto& m = message.GetExtension(::Proto::actuatorHeader);
		return LoadData(m);
	}

	bool ActuatorHeader::SaveData(Proto::ActuatorHeader* message) const
	{
		message->set_actuatortypeid(m_actuatorTypeId.toStdString());
		message->set_caption(m_caption.toStdString());
		message->set_description(m_description.toStdString());

		message->set_acmpresetname(m_acmPresetName.toStdString());
		message->set_descriptionfile(m_descriptionFile.toStdString());

		message->set_lmnumber(m_lmNumber);
		message->set_subsystemid(m_subsystemId.toStdString());

		message->set_excludefrombuild(m_excludeFromBuild);

		for (const auto& input : m_inputs)
		{
			auto* inputMessage = message->add_inputs();
			input->save(*inputMessage);
		}

		for (const auto& output : m_outputs)
		{
			auto* outputMessage = message->add_outputs();
			output->save(*outputMessage);
		}

		message->set_version(m_version);
		return true;
	}

	bool ActuatorHeader::LoadData(const Proto::ActuatorHeader& message)
	{
		m_actuatorTypeId = QString::fromStdString(message.actuatortypeid());
		m_caption = QString::fromStdString(message.caption());
		m_description = QString::fromStdString(message.description());

		m_acmPresetName = QString::fromStdString(message.acmpresetname());
		m_descriptionFile = QString::fromStdString(message.descriptionfile());

		m_lmNumber = message.lmnumber();
		m_subsystemId = QString::fromStdString(message.subsystemid());

		m_excludeFromBuild = message.excludefrombuild();

		m_inputs.clear();
		m_inputs.reserve(message.inputs_size());
		for (const auto& inputMessage : message.inputs())
		{
			auto s = m_inputs.createItem();
			s->load(inputMessage);
			m_inputs.push_back(std::move(s));
		}

		m_outputs.clear();
		m_outputs.reserve(message.outputs_size());
		for (const auto& outputMessage : message.outputs())
		{
			auto s = m_outputs.createItem();
			s->load(outputMessage);
			m_outputs.push_back(std::move(s));
		}

		m_version = message.version();
		return true;
	}

	QString ActuatorHeader::actuatorTypeId() const
	{
		return m_actuatorTypeId;
	}

	void ActuatorHeader::setActuatorTypeId(const QString& actuatorTypeId)
	{
		if (m_actuatorTypeId != actuatorTypeId)
		{
			m_actuatorTypeId = actuatorTypeId;
			m_modified = true;
		}
	}

	QString ActuatorHeader::caption() const
	{
		return m_caption;
	}

	void ActuatorHeader::setCaption(const QString& caption)
	{
		if (m_caption != caption)
		{
			m_caption = caption.trimmed();
			m_modified = true;
		}
	}

	QString ActuatorHeader::description() const
	{
		return m_description;
	}

	void ActuatorHeader::setDescription(const QString& description)
	{
		if (m_description != description)
		{
			m_description = description.trimmed();
			m_modified = true;
		}
	}

	QString ActuatorHeader::acmPresetName() const
	{
		return m_acmPresetName;
	}

	void ActuatorHeader::setAcmPresetName(const QString& acmPresetName)
	{
		if (m_acmPresetName != acmPresetName)
		{
			m_acmPresetName = acmPresetName.trimmed();
			m_modified = true;
		}
	}

	QString ActuatorHeader::descriptionFile() const
	{
		return m_descriptionFile;
	}

	void ActuatorHeader::setDescriptionFile(const QString& descriptionFile)
	{
		if (m_descriptionFile != descriptionFile)
		{
			m_descriptionFile = descriptionFile.trimmed();
			m_modified = true;
		}
	}

	int ActuatorHeader::lmNumber() const
	{
		return m_lmNumber;
	}

	void ActuatorHeader::setLmNumber(int lmNumber)
	{
		int clampedLmNumber = std::clamp(lmNumber, 0, maxLmNumber());
		if (m_lmNumber != clampedLmNumber)
		{
			m_lmNumber = clampedLmNumber;
			m_modified = true;
		}
	}

	int ActuatorHeader::maxLmNumber()
	{
		return Hardware::Subsystem::MaxChannelValue;
	}

	QString ActuatorHeader::subsystemId() const
	{
		return m_subsystemId;
	}

	void ActuatorHeader::setSubsystemId(const QString& subsystemId)
	{
		if (m_subsystemId != subsystemId)
		{
			m_subsystemId = subsystemId.trimmed();
			m_modified = true;
		}
	}

	bool ActuatorHeader::excludeFromBuild() const
	{
		return m_excludeFromBuild;
	}

	void ActuatorHeader::setExcludeFromBuild(bool excludeFromBuild)
	{
		m_excludeFromBuild = excludeFromBuild;
	}

	PropertyVector<ActuatorSignal> ActuatorHeader::inputs() const
	{
		return m_inputs;
	}

	PropertyVector<ActuatorSignal> ActuatorHeader::outputs() const
	{
		return m_outputs;
	}

	int ActuatorHeader::version() const
	{
		return m_version;
	}
} // namespace VFrame30