#include <VFrame30/SchemaItemValue.h>
#include <VFrame30/SchemaLayer.h>
#include <VFrame30/TuningSchema.h>

namespace VFrame30
{
	TuningSchema::TuningSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(297));
		setDocHeight(mm2in(210));

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerDrawingName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"tuning"});

		return;
	}

	bool TuningSchema::SaveData(Proto::Envelope* message) const
	{
		return Schema::SaveData(message);
	}

	bool TuningSchema::LoadData(const Proto::Envelope& message)
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
