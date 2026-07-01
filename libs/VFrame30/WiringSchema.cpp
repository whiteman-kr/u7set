#include <VFrame30/WiringSchema.h>

#include <VFrame30/SchemaLayer.h>

namespace
{
	struct WiringSchemaTraits : VFrame30::SchemaTraits
	{
		bool isItemSupported(const QString& clearClassName) const override
		{
			Q_UNUSED(clearClassName);
			return false;
		}
	};
} // namespace

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

	const SchemaTraits& WiringSchema::traits() const
	{
		const static WiringSchemaTraits st{};
		return st;
	}

} // namespace VFrame30
