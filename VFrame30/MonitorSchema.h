#pragma once

#include "Schema.h"

namespace VFrame30
{
	// Технологический видеокадр
	//
	class VFRAME30LIBSHARED_EXPORT MonitorSchema : public Schema
	{
		Q_OBJECT

	public:
		MonitorSchema(void);
		virtual ~MonitorSchema(void);
	};
}


