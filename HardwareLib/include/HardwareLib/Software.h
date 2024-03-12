#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// Software
	//
	//
	class Software : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit Software(bool preset = false, QObject* parent = nullptr);
		virtual ~Software() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Public Methods
		//
	public:
		[[nodiscard]] QString hostname() const;

		// Properties
		//
	public:
		[[nodiscard]] E::SoftwareType softwareType() const;
		void setSoftwareType(E::SoftwareType value);

		// Data
		//
	private:
		E::SoftwareType m_softwareType = E::SoftwareType::Monitor;
	};

} // namespace Hardware