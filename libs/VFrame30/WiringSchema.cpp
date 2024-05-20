#include <VFrame30/WiringSchema.h>
#include <VFrame30/SchemaLayer.h>

namespace VFrame30
{

	WiringSchema::WiringSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		addLayer(std::make_shared<SchemaLayer>(this, "Frame", false));
		addLayer(std::make_shared<SchemaLayer>(this, "Drawing", true));
		addLayer(std::make_shared<SchemaLayer>(this, "Notes", false));

		setTagsList(QStringList{"wiring"});

		return;
	}
	
	WiringSchema::~WiringSchema(void)
	{
	}

}
