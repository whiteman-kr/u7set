#pragma once

#include <VFrame30/Schema.h>

namespace VFrame30
{
	// Schema for TuningClient
	//
	class TuningSchema : public Schema
	{
		Q_OBJECT

	public:
		TuningSchema(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};
}


