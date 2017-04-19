#include "DiagSchema.h"
#include "Settings.h"

namespace VFrame30
{

	DiagSchema::DiagSchema(void)
	{
		setUnit(SchemaUnit::Display);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(20);

		setDocWidth(1000);
		setDocHeight(750);

		Layers.push_back(std::make_shared<SchemaLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemaLayer>("Notes", false));

		return;
	}

	DiagSchema::~DiagSchema(void)
	{
	}

}
