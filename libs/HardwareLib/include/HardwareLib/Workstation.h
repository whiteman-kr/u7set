#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// Workstation
	//
	//
	class Workstation : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit Workstation(bool preset = false, QObject* parent = nullptr);
		virtual ~Workstation() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Public Methods
		//
	public:
		// Properties
		//
	public:
		[[nodiscard]] int type() const;
		void setType(int value);

		[[nodiscard]] QString hostname() const;
		void setHostname(QString value);

		// Data
		//
	private:
		int m_type = 0;
		QString m_hostname;
	};
} // namespace Hardware