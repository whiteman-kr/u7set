#pragma once

#include "Schema.h"

namespace VFrame30
{
	// Schema for TuningClient
	//
	class VFRAME30LIBSHARED_EXPORT TuningSchema : public Schema
	{
		Q_OBJECT

	public:
		TuningSchema(void);
		virtual ~TuningSchema(void);

	public:
		virtual QStringList getSignalList() const override;
	};
}


