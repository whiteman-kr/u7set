#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DeviceController
	//
	//
	class DeviceController : public DeviceObject
	{
		Q_OBJECT
	public:
		explicit DeviceController(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceController() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Properties
		//
	public:
		[[nodiscard]] int diagDataOffset() const;
		void setDiagDataOffset(int value);

		// Data
		//
	private:
		int m_diagDataOffset = 0;
	};
} // namespace Hardware