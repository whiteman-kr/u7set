#include <VFrame30/LogicSchema.h>
#include <VFrame30/PropertyNames.h>


#include <VFrame30/SchemaItemImage.h>
#include <VFrame30/SchemaItemLine.h>
#include <VFrame30/SchemaItemPath.h>
#include <VFrame30/SchemaItemRect.h>

#include <VFrame30/SchemaItemConst.h>
#include <VFrame30/SchemaItemLink.h>
#include <VFrame30/SchemaItemSignal.h>

#include <VFrame30/SchemaItemAfb.h>
#include <VFrame30/SchemaItemBus.h>
#include <VFrame30/SchemaItemConnection.h>
#include <VFrame30/SchemaItemLoopback.h>
#include <VFrame30/SchemaItemTerminator.h>
#include <VFrame30/SchemaItemUfb.h>

#include <VFrame30/SchemaItemImageValue.h>
#include <VFrame30/SchemaItemIndicator.h>
#include <VFrame30/SchemaItemLineEdit.h>
#include <VFrame30/SchemaItemPushButton.h>
#include <VFrame30/SchemaItemSlider.h>
#include <VFrame30/SchemaItemValue.h>

#include <VFrame30/SchemaLayer.h>


namespace
{
	struct LogicSchemaTraits : VFrame30::SchemaTraits
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
				VFrame30::SchemaItem::type<VFrame30::SchemaItemReceiver>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemTerminator>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemTransmitter>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemUfb>(),

				VFrame30::SchemaItem::type<VFrame30::SchemaItemImageValue>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemIndicator>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemLineEdit>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemPushButton>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemSlider>(),
				VFrame30::SchemaItem::type<VFrame30::SchemaItemValue>(),
			};

			return supportedItems.contains(clearClassName);
		}
	};
} // namespace

namespace VFrame30
{
	LogicSchema::LogicSchema(void)
	{
		// qDebug() << "LogicSchema::LogicSchema(void)";

		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::equipmentIds, true, LogicSchema::equipmentIds, LogicSchema::setEquipmentIds);
		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::lmDescriptionFile,
								   true,
								   LogicSchema::lmDescriptionFile,
								   LogicSchema::setLmDescriptionFile);

		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		addLayer(std::make_shared<SchemaLayer>(this, LayerFrameName, false));
		addLayer(std::make_shared<SchemaLayer>(this, LayerLogicName, true));
		addLayer(std::make_shared<SchemaLayer>(this, LayerNotesName, false));

		setTagsList(QStringList{"applogic"});

		return;
	}

	LogicSchema ::~LogicSchema(void)
	{
		// qDebug() << "LogicSchema::~LogicSchema(void)  SchemaID = " << schemaId();
	}

	const SchemaTraits& LogicSchema::traits() const
	{
		static const LogicSchemaTraits st;
		return st;
	}

	bool LogicSchema::SaveData(Proto::Envelope* message) const
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
		Proto::LogicSchema* ls = message->MutableExtension(Proto::schema)->mutable_logic_schema();

		for (const QString& strId : m_equipmentIds)
		{
			::Proto::wstring* hs = ls->add_equipmentids();
			Proto::Write(hs, strId);
		}

		ls->set_counter(m_counter);
		ls->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());

		return true;
	}

	bool LogicSchema::LoadData(const Proto::Envelope& message)
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

		if (schemaMessage.has_logic_schema() == false)
		{
			assert(schemaMessage.has_logic_schema());
			return false;
		}

		const Proto::LogicSchema& ls = schemaMessage.logic_schema();

		m_equipmentIds.clear();
		m_equipmentIds.reserve(ls.equipmentids_size());

		for (int i = 0; i < ls.equipmentids_size(); i++)
		{
			QString s;
			Proto::Read(ls.equipmentids(i), &s);
			m_equipmentIds.push_back(s);
		}

		m_counter = ls.counter();

		// Initialize Labels for SchemaItemAfbs (if they were not created with label)
		//
		for (const auto& layer : layers())
		{
			Q_ASSERT(layer);

			for (const auto& item : layer->items())
			{
				if (item->label().isEmpty() == true)
				{
					int labelCounter = this->nextCounterValue();
					item->setLabel(schemaId() + "_" + QString::number(labelCounter));
				}
			}
		}

		m_lmDescriptionFile = QString::fromStdString(ls.lmdescriptionfile());

		return true;
	}

	void LogicSchema::Draw(CDrawParam* drawParam, const QRectF& clipRect)
	{
		BuildFblConnectionMap();

		Schema::Draw(drawParam, clipRect);
		return;
	}

	std::map<QString, SchemaItemSignal*> LogicSchema::getSignalItemsMap() const
	{
		std::map<QString, SchemaItemSignal*> result;

		for (const auto& layer : layers())
		{
			if (layer->compile() == true)
			{
				// Get all signals
				//
				for (const auto& item : layer->items())
				{
					if (VFrame30::SchemaItemSignal* itemSignal = item->toType<SchemaItemSignal>(); itemSignal != nullptr)
					{
						QStringList appSignals = itemSignal->appSignalIdList();
						for (const QString& id : appSignals)
						{
							result[id] = itemSignal;
						}

						appSignals = itemSignal->impactAppSignalIdList();
						for (const QString& id : appSignals)
						{
							result[id] = itemSignal;
						}
					}
				}
			}
		}

		return result;
	}

	std::map<QString, SchemaItemReceiver*> LogicSchema::getSignalReceiversMap() const
	{
		std::map<QString, SchemaItemReceiver*> result;

		for (const auto& layer : layers())
		{
			if (layer->compile() == true)
			{
				// Get all signals
				//
				for (const auto& item : layer->items())
				{
					if (VFrame30::SchemaItemReceiver* itemReceiver = item->toType<SchemaItemReceiver>(); itemReceiver != nullptr)
					{
						QStringList appSignals = itemReceiver->appSignalIdsAsList();
						for (const QString& id : appSignals)
						{
							result[id] = itemReceiver;
						}
					}
				}
			}
		}

		return result;
	}

	std::map<QString, SchemaItemLoopback*> LogicSchema::getLoopbacksMap() const
	{
		std::map<QString, VFrame30::SchemaItemLoopback*> result;

		for (const auto& layer : layers())
		{
			if (layer->compile() == true)
			{
				// Get all signals
				//
				for (const auto& item : layer->items())
				{
					if (VFrame30::SchemaItemLoopback* itemLoopback = item->toType<SchemaItemLoopback>(); itemLoopback != nullptr)
					{
						result[itemLoopback->loopbackId()] = itemLoopback;
					}
				}
			}
		}
		return result;
	}

	std::map<QString, SchemaItemTransmitter*> LogicSchema::getTransmittersMap() const
	{
		std::map<QString, VFrame30::SchemaItemTransmitter*> result;

		for (const auto& layer : layers())
		{
			if (layer->compile() == true)
			{
				// Get all signals
				//
				for (const auto& item : layer->items())
				{
					if (VFrame30::SchemaItemTransmitter* itemTransmitter = item->toType<SchemaItemTransmitter>();
						itemTransmitter != nullptr)
					{
						for (const QString& c : itemTransmitter->connectionIdsAsList())
						{
							result[c] = itemTransmitter;
						}
					}
				}
			}
		}
		return result;
	}

	std::map<QString, SchemaItemReceiver*> LogicSchema::getReceiversMap() const
	{
		std::map<QString, VFrame30::SchemaItemReceiver*> result;

		for (const auto& layer : layers())
		{
			if (layer->compile() == true)
			{
				// Get all signals
				//
				for (const auto& item : layer->items())
				{
					if (VFrame30::SchemaItemReceiver* itemReceiver = item->toType<SchemaItemReceiver>(); itemReceiver != nullptr)
					{
						for (const QString& c : itemReceiver->connectionIdsAsList())
						{
							result[c] = itemReceiver;
						}
					}
				}
			}
		}
		return result;
	}

	QString LogicSchema::equipmentIds() const
	{
		QString result;

		for (QString s : m_equipmentIds)
		{
			s = s.trimmed();

			if (result.isEmpty() == false)
			{
				result.append(QChar::LineFeed);
			}

			result.append(s);
		}

		return result;
	}

	const QStringList& LogicSchema::equipmentIdList() const
	{
		return m_equipmentIds;
	}

	void LogicSchema::setEquipmentIds(const QString& s)
	{
		m_equipmentIds = s.split(QChar::LineFeed, Qt::SkipEmptyParts);

		for (QString& id : m_equipmentIds)
		{
			id = id.trimmed();
		}
	}

	void LogicSchema::setEquipmentIdList(const QStringList& s)
	{
		m_equipmentIds = s;
	}

	QStringList* LogicSchema::mutable_equipmentIds()
	{
		return &m_equipmentIds;
	}

	bool LogicSchema::isMultichannelSchema() const
	{
		return m_equipmentIds.size() > 1;
	}

	int LogicSchema::channelCount() const
	{
		return static_cast<int>(m_equipmentIds.size());
	}

	int LogicSchema::nextCounterValue()
	{
		return ++m_counter;
	}

	QString LogicSchema::lmDescriptionFile() const
	{
		return m_lmDescriptionFile;
	}

	void LogicSchema::setLmDescriptionFile(QString value)
	{
		m_lmDescriptionFile = value;
	}
} // namespace VFrame30
