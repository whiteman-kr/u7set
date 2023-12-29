#pragma once

#include "DeviceObject.h"


namespace Hardware
{
	//
	//
	// DiagSignal
	//
	//
	class DiagSignal : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DiagSignal(bool preset = false, QObject* parent = nullptr) noexcept;
		virtual ~DiagSignal() = default;

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Expand EquipmentIDTemplate, ValiditySignalId for this and for all children
		//
	public:
		virtual void expandEquipmentId() override;

		// Properties
		//
	public:
		[[nodiscard]] const QString& signalTypeId() const;
		void setSignalTypeId(const QString& value);

		[[nodiscard]] int valueOffset() const;
		void setValueOffset(int value);

		[[nodiscard]] int valueBit() const;
		void setValueBit(int value);

		[[nodiscard]] const QString& validitySignalId() const;
		void setValiditySignalId(const QString& value);

		// Data
		//
	private:
		QString m_signalTypeId;
		int m_valueOffset = 0;
		int m_valueBit = 0;

		QString m_validitySignalId;

		// Online properties
		//
	public:
		std::shared_ptr<Hardware::DiagSignalType> diagSignalType() const;
		void setDiagSignalType(std::shared_ptr<DiagSignalType> value);


		// Data used only in "online" part, like DiagDataService, Disagnostics, etc (but not u7)
		//
	private:
		std::shared_ptr<Hardware::DiagSignalType> m_diagSignalType; // Bind via m_signalTypeId
		std::shared_ptr<Hardware::DiagSignal> m_validitySinal;      // Bind via m_signalTypeId
	};

} // namespace Hardware