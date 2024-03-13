#include "VduSchema.h"
#include "SchemaLayer.h"

namespace VFrame30
{

	VduSchema::VduSchema(void) :
		Schema()
	{
		setUnit(SchemaUnit::Display);

		setDocWidth(mm2in(1280));
		setDocHeight(mm2in(1024));

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		addLayer(std::make_shared<SchemaLayer>(this, "Frame", false));
		addLayer(std::make_shared<SchemaLayer>(this, "Drawing", true));
		addLayer(std::make_shared<SchemaLayer>(this, "Notes", false));

		setTagsList(QStringList{"vdu"});

		return;
	}
	
	bool VduSchema::SaveData(Proto::Envelope* message) const
	{
		return Schema::SaveData(message);
	}

	bool VduSchema::LoadData(const Proto::Envelope& message)
	{
		return Schema::LoadData(message);
	}
}
