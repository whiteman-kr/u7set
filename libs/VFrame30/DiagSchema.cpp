#include <VFrame30/DiagSchema.h>
#include <VFrame30/SchemaLayer.h>

namespace VFrame30
{
	DiagSchema::DiagSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(420);
		setDocHeight(297);

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerDrawingName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"diagnostics"});

		return;
	}

	DiagSchema::~DiagSchema(void) {}
} // namespace VFrame30
