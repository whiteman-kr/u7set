#include <VFrame30/DiagSchema.h>
#include <VFrame30/SchemaLayer.h>

#include <VFrame30/SchemaItemImage.h>
#include <VFrame30/SchemaItemLine.h>
#include <VFrame30/SchemaItemPath.h>
#include <VFrame30/SchemaItemRect.h>

#include <VFrame30/SchemaItemDiagValue.h>

namespace
{
	struct DiagSchemaTraits : VFrame30::SchemaTraits
	{
		bool isItemSupported(const QString& clearClassName) const override
		{
			static const std::set<QString> supportedItems{
				VFrame30::SchemaItem::type<VFrame30::SchemaItemImage>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLine>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemPath>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemRect>(),

				VFrame30::SchemaItem::type<VFrame30::SchemaItemDiagValue>(),
			};

			return supportedItems.contains(clearClassName);
		}
	};
} // namespace


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

	const SchemaTraits& DiagSchema::traits() const
	{
		static const DiagSchemaTraits st{};
		return st;
	}
} // namespace VFrame30
