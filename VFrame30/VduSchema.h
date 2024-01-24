#pragma once

#include "Schema.h"

namespace VFrame30
{
	// Schema for Monitor Application
	//
	class VduSchema : public Schema
	{
		Q_OBJECT

	public:
		VduSchema(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};
}


