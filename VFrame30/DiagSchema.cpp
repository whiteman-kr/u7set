#include "DiagSchema.h"
#include "SchemaLayer.h"

namespace VFrame30
{
	DiagSchema::DiagSchema(void)
	{
		setUnit(SchemaUnit::Display);

		setDocWidth(1000);
		setDocHeight(750);

		addLayer(std::make_shared<SchemaLayer>(this, "Drawing", true));
		addLayer(std::make_shared<SchemaLayer>(this, "Notes", false));

		setTagsList(QStringList{"diagnostics"});

		return;
	}

	DiagSchema::~DiagSchema(void)
	{
	}
}
