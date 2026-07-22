#include <VFrame30/ActuatorSchema.h>

#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemAfb.h>
#include <VFrame30/SchemaItemBus.h>
#include <VFrame30/SchemaItemConst.h>
#include <VFrame30/SchemaItemImage.h>
#include <VFrame30/SchemaItemLine.h>
#include <VFrame30/SchemaItemLink.h>
#include <VFrame30/SchemaItemLoopback.h>
#include <VFrame30/SchemaItemPath.h>
#include <VFrame30/SchemaItemRect.h>
#include <VFrame30/SchemaItemSignal.h>
#include <VFrame30/SchemaItemTerminator.h>
#include <VFrame30/SchemaItemUfb.h>
#include <VFrame30/SchemaLayer.h>


namespace
{
	struct ActuatorSchemaTraits : VFrame30::SchemaTraits
	{
		bool isItemSupported(const QString& clearClassName) const override
		{
			static const std::set<QString> supportedItems{
				VFrame30::SchemaItem::type<VFrame30::SchemaItemImage>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLine>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemPath>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemRect>(),

				VFrame30::SchemaItem::type<VFrame30::SchemaItemAfb>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemBusComposer>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemBusExtractor>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemConst>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemInOut>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemInput>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLink>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLoopbackSource>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLoopbackTarget>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemOutput>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemTerminator>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemUfb>(),
			};

			return supportedItems.contains(clearClassName);
		}
	};
} // namespace

namespace VFrame30
{
	ActuatorSchema::ActuatorSchema(void)
	{
		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::ActuatorTypeId,
								   true,
								   ActuatorSchema::actuatorTypeId,
								   ActuatorSchema::setActuatorTypeId);

		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::lmDescriptionFile,
								   true,
								   ActuatorSchema::lmDescriptionFile,
								   ActuatorSchema::setLmDescriptionFile);

		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerLogicName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"actuatorlogic"});

		return;
	}

	ActuatorSchema::~ActuatorSchema(void) = default;

	const SchemaTraits& ActuatorSchema::traits() const
	{
		static const ActuatorSchemaTraits st;
		return st;
	}

	bool ActuatorSchema::SaveData(Proto::Envelope* message) const
	{
		bool result = Schema::SaveData(message);

		if (result == false || message->HasExtension(Proto::schema) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schema));
			return false;
		}

		// --
		//
		Proto::ActuatorSchema* as = message->MutableExtension(Proto::schema)->mutable_actuator_schema();

		as->set_actuatortypeid(m_actuatorTypeId.toStdString());
		as->set_counter(m_counter);
		as->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());

		return true;
	}

	bool ActuatorSchema::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schema) == false)
		{
			assert(message.HasExtension(Proto::schema));
			return false;
		}

		// --
		//
		bool result = Schema::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		const Proto::Schema& schemaMessage = message.GetExtension(Proto::schema);

		if (schemaMessage.has_actuator_schema() == false)
		{
			assert(schemaMessage.has_actuator_schema());
			return false;
		}

		const Proto::ActuatorSchema& as = schemaMessage.actuator_schema();

		m_actuatorTypeId = QString::fromStdString(as.actuatortypeid());
		m_counter = as.counter();
		m_lmDescriptionFile = QString::fromStdString(as.lmdescriptionfile());

		return true;
	}

	void ActuatorSchema::Draw(CDrawParam* drawParam, const QRectF& clipRect)
	{
		BuildFblConnectionMap();

		Schema::Draw(drawParam, clipRect);
		return;
	}

	std::map<QString, SchemaItemLoopback*> ActuatorSchema::getLoopbacksMap() const
	{
		std::map<QString, VFrame30::SchemaItemLoopback*> result;

		auto it = std::find_if(layers().begin(),
							   layers().end(),
							   [](const std::shared_ptr<SchemaLayer>& layer)
							   {
								   return layer->compile() == true;
							   });

		if (it != layers().end())
		{
			const auto& layer = *it;

			for (const auto& item : layer->items())
			{
				if (VFrame30::SchemaItemLoopback* itemLoopback = item->toType<SchemaItemLoopback>(); //
					itemLoopback != nullptr)
				{
					result[itemLoopback->loopbackId()] = itemLoopback;
				}
			}
		}

		return result;
	}

	QString ActuatorSchema::actuatorTypeId() const
	{
		return m_actuatorTypeId;
	}

	void ActuatorSchema::setActuatorTypeId(const QString& value)
	{
		m_actuatorTypeId = value;
	}

	int ActuatorSchema::nextCounterValue()
	{
		return ++m_counter;
	}

	QString ActuatorSchema::lmDescriptionFile() const
	{
		return m_lmDescriptionFile;
	}

	void ActuatorSchema::setLmDescriptionFile(QString value)
	{
		m_lmDescriptionFile = value;
	}
} // namespace VFrame30
