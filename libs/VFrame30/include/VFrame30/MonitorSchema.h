#pragma once

#include <VFrame30/Schema.h>

namespace VFrame30
{
	// Schema for Monitor Application
	//
	class MonitorSchema : public Schema
	{
		Q_OBJECT

	public:
		MonitorSchema(void);

		virtual const SchemaTraits& traits() const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};
} // namespace VFrame30
