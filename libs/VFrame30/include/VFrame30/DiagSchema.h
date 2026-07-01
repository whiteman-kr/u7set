#pragma once

#include <VFrame30/Schema.h>

namespace VFrame30
{

	class DiagSchema : public Schema
	{
		Q_OBJECT

	public:
		DiagSchema(void);
		virtual ~DiagSchema(void);

		virtual const SchemaTraits& traits() const override;
	};

} // namespace VFrame30
