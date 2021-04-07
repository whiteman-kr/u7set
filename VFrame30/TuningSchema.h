#pragma once

#include "Schema.h"

namespace VFrame30
{
	// Schema for TuningClient
	//
	class TuningSchema : public Schema
	{
		Q_OBJECT

	public:
		TuningSchema(void);
		virtual ~TuningSchema(void);

	public:
		virtual QStringList getSignalList() const override;
	};
}


