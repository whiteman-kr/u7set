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
		virtual bool SaveData(Proto::Envelope2* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope2& message) override;

		// Expand EquipmentIDTemplate, ValiditySignalId for this and for all children
		//
	public:
		virtual void expandEquipmentId() override;

		// Properties
		//
	public:
		// DiagSignal properties
		//
		[[nodiscard]] bool isReflection() const;
		void setIsReflection(bool value);

		[[nodiscard]] const QString& reflectedSignalId() const;
		void setReflectedSignalId(const QString& value);

		[[nodiscard]] E::DiagLevel level() const;
		void setLevel(E::DiagLevel value);

		[[nodiscard]] const QString& signalTypeId() const;
		void setSignalTypeId(const QString& value);

		[[nodiscard]] const QString& validitySignalId() const;
		void setValiditySignalId(const QString& value);

		// Location properties
		//
		[[nodiscard]] int valueOffset() const;
		void setValueOffset(int value);

		[[nodiscard]] int valueBit() const;
		void setValueBit(int value);

		[[nodiscard]] int valueBitSize() const;
		void setValueBitSize(int value);

		[[nodiscard]] int discreteContainerSize() const;
		void setDiscreteContainerSize(int value);

		// MATS properties
		//
		[[nodiscard]] bool logChanges() const;
		void setLogChanges(bool value);

		[[nodiscard]] bool archive() const;
		void setArchive(bool value);

		[[nodiscard]] bool reserved() const;
		void setReserved(bool value);

		[[nodiscard]] double coarseAperture() const;
		void setCoarseAperture(double value);

		[[nodiscard]] double fineAperture() const;
		void setFineAperture(double value);

		[[nodiscard]] E::ApertureType apertureType() const;
		void setApertureType(E::ApertureType value);

		[[nodiscard]] int decimalPlaces() const;
		void setDecimalPlaces(int value);

		// Online properties
		//
	public:
		const std::shared_ptr<Hardware::DiagSignalTypeObject>& diagSignalType() const;
		void setDiagSignalType(std::shared_ptr<DiagSignalTypeObject> value);

		const std::shared_ptr<Hardware::DiagSignal>& validitySignal() const;
		void setValiditySignal(std::shared_ptr<DiagSignal> value);

		// Data
		//
	private:
		bool m_isReflection = false;	// The signal is a reflected valued of another signal
		QString m_reflectedSignalId;	// The reflected signal id if m_isReflection is true

		E::DiagLevel m_level = E::DiagLevel::Message;
		
		QString m_signalTypeId;
		QString m_validitySignalId;

		// Data properties
		//
		int m_valueOffset = 0;
		int m_valueBit = 0;
		int m_valueBitSize = 1;
		int m_discreteContainerSize = 2;

		// MATS properties
		//
		bool m_logChanges = false;
		bool m_archive = true;
		bool m_reserved = false;

		double m_coarseAperture = 1;
		double m_fineAperture = 0.5;
		E::ApertureType m_apertureType = E::ApertureType::RangePercent;

		int m_decimalPlaces = 1;

		// Data used only in "online" part, like DiagDataService, Diagnostics, etc (but not u7)
		//
	private:
		std::shared_ptr<Hardware::DiagSignalTypeObject> m_diagSignalType; // Bind via m_signalTypeId
		std::shared_ptr<Hardware::DiagSignal> m_validitySignal;           // Bind via m_validitySignalId
	};

} // namespace Hardware