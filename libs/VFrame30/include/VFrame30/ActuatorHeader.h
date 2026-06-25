#pragma once

#include <CommonLib/PropertyObject.h>


namespace Proto
{
	class Envelope;
} // namespace Proto


namespace VFrame30
{
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
	};
} // namespace VFrame30