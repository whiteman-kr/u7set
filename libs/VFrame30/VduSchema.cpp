#include <VFrame30/VduSchema.h>

#include <VFrame30/SchemaItemVduImage.h>
#include <VFrame30/SchemaItemVduImageValue.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduTrend.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaLayer.h>


namespace
{
	struct VduSchemaTraits : VFrame30::SchemaTraits
	{
		bool isItemSupported(const QString& clearClassName) const override
		{
			static const std::set<QString> supportedItems{
				VFrame30::SchemaItem::type<VFrame30::SchemaItemVduImage>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemVduLine>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemVduRect>(),

				VFrame30::SchemaItem::type<VFrame30::SchemaItemVduImageValue>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemVduTrend>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemVduValue>(),
			};

			return supportedItems.contains(clearClassName);
		}
	};
} // namespace


namespace VFrame30
{
	VduSchema::VduSchema(void) :
		Schema()
	{
		setUnit(SchemaUnit::Display);

		setDocWidth(mm2in(1280));
		setDocHeight(mm2in(1024));

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerDrawingName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"vdu"});

		return;
	}

	const SchemaTraits& VduSchema::traits() const
	{
		static const VduSchemaTraits st{};
		return st;
	}

	bool VduSchema::SaveData(Proto::Envelope* message) const
	{
		return Schema::SaveData(message);
	}

	bool VduSchema::LoadData(const Proto::Envelope& message)
	{
		return Schema::LoadData(message);
	}
} // namespace VFrame30
