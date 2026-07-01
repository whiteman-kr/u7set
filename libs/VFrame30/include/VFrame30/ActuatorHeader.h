#pragma once

#include <CommonLib/PropertyObject.h>


namespace Proto
{
	class Envelope;
	class ActuatorSignal;
} // namespace Proto


namespace VFrame30
{
	class ActuatorSignal : public PropertyObject
	{
		Q_OBJECT

	public:
		ActuatorSignal();
		ActuatorSignal(const ActuatorSignal& other);

	private:
		void init();

	public:
		void save(Proto::ActuatorSignal& message) const;
		void load(const Proto::ActuatorSignal& message);

	public:
		[[nodiscard]] QString signalId() const;
		void setSignalId(const QString& signalId);

		[[nodiscard]] E::SignalType signalType() const;
		void setSignalType(E::SignalType signalType);

		[[nodiscard]] E::AnalogAppSignalFormat analogFormat() const;
		void setAnalogFormat(E::AnalogAppSignalFormat analogFormat);

		[[nodiscard]] QString busTypeId() const;
		void setBusTypeId(const QString& busTypeId);

	private:
		QString m_signalId = "";
		E::SignalType m_signalType = E::SignalType::Discrete;
		E::AnalogAppSignalFormat m_analogFormat = E::AnalogAppSignalFormat::Float32;
		QString m_busTypeId;
	};


	class ActuatorHeader : public PropertyObject,
						   public Proto::ObjectSerialization<ActuatorHeader>
	{
	public:
		ActuatorHeader(QObject* parent = nullptr);

	protected:
	private:
		friend class Proto::ObjectSerialization<ActuatorHeader>;
		[[nodiscard]] static std::shared_ptr<ActuatorHeader> CreateObject(const Proto::Envelope& message);

		virtual bool SaveData(::Proto::Envelope* message) const override;
		virtual bool LoadData(const ::Proto::Envelope& message) override;

	private:
		virtual void propertyDemand(const QString& prop) override;

	public:
		QString actuatorTypeId() const;
		void setActuatorTypeId(const QString& actuatorTypeId);

		QString caption() const;
		void setCaption(const QString& caption);

		QString acmPresetName() const;
		void setAcmPresetName(const QString& acmPresetName);

		QString descriptionFile() const;
		void setDescriptionFile(const QString& descriptionFile);

		int lmNumber() const;
		void setLmNumber(int lmNumber);

		static int maxLmNumber();

		QString subsystemId() const;
		void setSubsystemId(const QString& subsystemId);

		bool excludeFromBuild() const;
		void setExcludeFromBuild(bool excludeFromBuild);

	private:
		QString m_actuatorTypeId;
		QString m_caption;

		QString m_acmPresetName;   // HardwareLib::DeviceObject::presetName()
		QString m_descriptionFile; // HardwareLib::PropertyNames::lmDescriptionFile

		int m_lmNumber = 0;
		QString m_subsystemId;

		bool m_excludeFromBuild = false;

		PropertyVector<ActuatorSignal> m_inputs;
		PropertyVector<ActuatorSignal> m_outputs;
	};
} // namespace VFrame30