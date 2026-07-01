#include <VFrame30/MonitorSchema.h>
#include <VFrame30/SchemaLayer.h>

#include <VFrame30/SchemaItemImage.h>
#include <VFrame30/SchemaItemLine.h>
#include <VFrame30/SchemaItemPath.h>
#include <VFrame30/SchemaItemRect.h>

#include <VFrame30/SchemaItemImageValue.h>
#include <VFrame30/SchemaItemIndicator.h>
#include <VFrame30/SchemaItemLineEdit.h>
#include <VFrame30/SchemaItemPushButton.h>
#include <VFrame30/SchemaItemSlider.h>
#include <VFrame30/SchemaItemValue.h>


namespace
{
	struct MonitorSchemaTraits : VFrame30::SchemaTraits
	{
		bool isItemSupported(const QString& clearClassName) const override
		{
			static const std::set<QString> supportedItems{
				VFrame30::SchemaItem::type<VFrame30::SchemaItemRect>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemPath>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLine>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemImage>(),

				VFrame30::SchemaItem::type<VFrame30::SchemaItemValue>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemSlider>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemPushButton>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLineEdit>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemIndicator>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemImageValue>(),
			};

			return supportedItems.contains(clearClassName);
		}
	};
} // namespace

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

	const SchemaTraits& MonitorSchema::traits() const
	{
		static const MonitorSchemaTraits st{};
		return st;
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