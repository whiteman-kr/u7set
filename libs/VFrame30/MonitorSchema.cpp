#include <VFrame30/MonitorSchema.h>
#include <VFrame30/SchemaLayer.h>


namespace VFrame30
{
	MonitorSchema::MonitorSchema(void) :
		Schema()
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerDrawingName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"monitor"});

		return;
	}

	bool MonitorSchema::SaveData(Proto::Envelope* message) const
	{
		return Schema::SaveData(message);
	}

	bool MonitorSchema::LoadData(const Proto::Envelope& message)
	{
		bool ok = Schema::LoadData(message);
		if (ok == false)
		{
			return false;
		}

		// RPCT-3512, layer Frame was added before Drawing, Notes.
		// If the loaded schema does not have layer Frame, then add it manually, in the order: Frame, Drawing, Notes.
		//
		if (const auto& ls = layers(); std::find_if(ls.begin(),
													ls.end(),
													[](const auto& l)
													{
														return l->name() == LayerFrameName;
													}) == ls.end())
		{
			// Add layer.
			//
			addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));

			fixLayerOrder();
		}

		return true;
	}
} // namespace VFrame30