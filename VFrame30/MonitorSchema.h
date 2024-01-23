#pragma once

#include "Schema.h"

namespace VFrame30
{
	// Schema for Monitor Application
	//
	class MonitorSchema : public Schema
	{
		Q_OBJECT

	public:
		MonitorSchema(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};
}


