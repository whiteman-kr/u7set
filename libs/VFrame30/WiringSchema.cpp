#include <VFrame30/SchemaLayer.h>
#include <VFrame30/WiringSchema.h>

namespace VFrame30
{

	WiringSchema::WiringSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerDrawingName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"wiring"});

		return;
	}

	WiringSchema::~WiringSchema(void) {}

} // namespace VFrame30
