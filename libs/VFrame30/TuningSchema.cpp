#include <VFrame30/TuningSchema.h>
#include <VFrame30/SchemaItemValue.h>
#include <VFrame30/SchemaLayer.h>

namespace VFrame30
{
	TuningSchema::TuningSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(297));
		setDocHeight(mm2in(210));

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		addLayer(std::make_shared<SchemaLayer>(this, "Frame", false));
		addLayer(std::make_shared<SchemaLayer>(this, "Drawing", true));
		addLayer(std::make_shared<SchemaLayer>(this, "Notes", false));

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
		if (const auto& ls = layers();
			std::find_if(ls.begin(), ls.end(), [](const auto& l) { return l->name() == "Frame";}) == ls.end())
		{
			// Add layer.
			//
			addLayer(std::make_shared<SchemaLayer>(this, "Frame", false));

			// Make copy of all layers, then sort it and assign back to schema.
			//
			std::vector<std::shared_ptr<SchemaLayer>> layersCopy = layers();

			std::stable_sort(layersCopy.begin(), layersCopy.end(), [](const SchemaLayerPtr& left, const SchemaLayerPtr& right)
			{
				int l = 99;
				do
				{
					if (left->name() == QLatin1String("Frame"))
					{
						l = 0;
						break;
					}

					if (left->name() == QLatin1String("Drawing"))
					{
						l = 1;
						break;
					}

					if (left->name() == QLatin1String("Notes"))
					{
						l = 2;
						break;
					}
				} while (false);

				int r = 99;
				do
				{
					if (right->name() == QLatin1String("Frame"))
					{
						r = 0;
						break;
					}

					if (right->name() == QLatin1String("Drawing"))
					{
						r = 1;
						break;
					}

					if (right->name() == QLatin1String("Notes"))
					{
						r = 2;
						break;
					}
				} while (false);

				return l < r;
			});

			// Assign sorted layers to the schema.
			//
			clearLayers();
			for (const auto &l : layersCopy)
			{
				addLayer(l);
			}

			// Layers were reordered need to set active layer again.
			//
			if (auto compileLayerIt = std::find_if(layersCopy.begin(), layersCopy.end(), [](const auto& l) { return l->compile(); });
				compileLayerIt != layersCopy.end())
			{
				Q_ASSERT(*compileLayerIt);
				setActiveLayer(*compileLayerIt);
			}
		}

		return true;
	}
}
