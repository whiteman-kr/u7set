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
		virtual ~MonitorSchema(void);

	public:
		virtual QStringList getSignalList() const override;
	};
}


