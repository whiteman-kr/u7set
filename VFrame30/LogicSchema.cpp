#include "Stable.h"
#include "LogicSchema.h"
#include "SchemaItemAfb.h"
#include "SchemaItemSignal.h"

namespace VFrame30
{
	LogicSchema::LogicSchema(void)
	{
		qDebug() << "LogicSchema::LogicSchema(void)";

		ADD_PROPERTY_GETTER_SETTER(QString, "EquipmentIDs", true, LogicSchema::equipmentIds, LogicSchema::setEquipmentIds)

		setUnit(SchemaUnit::Inch);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(4);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemaLayer>("Logic", true));
		Layers.push_back(std::make_shared<SchemaLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemaLayer>("Notes", false));

		return;
	}

	LogicSchema ::~LogicSchema (void)
	{
		qDebug() << "LogicSchema::~LogicSchema(void)";
	}

	bool LogicSchema::SaveData(Proto::Envelope* message) const
	{
		bool result = Schema::SaveData(message);

		if (result == false || message->has_schema() == false)
		{
			assert(result);
			assert(message->has_schema());
			return false;
		}

		// --
		//
		Proto::LogicSchema* ls = message->mutable_schema()->mutable_logic_schema();

		for (const QString& strId : m_equipmentIds)
		{
			::Proto::wstring* hs = ls->add_equipmentids();
			Proto::Write(hs, strId);
		}

		ls->set_counter(m_counter);

		return true;
	}

	bool LogicSchema::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schema() == false)
		{
			assert(message.has_schema());
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
		if (message.schema().has_logic_schema() == false)
		{
			assert(message.schema().has_logic_schema());
			return false;
		}

		const Proto::LogicSchema& ls = message.schema().logic_schema();

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
		for (std::shared_ptr<SchemaLayer> layer : Layers)
		{
			assert(layer);

			for (std::shared_ptr<SchemaItem> item : layer->Items)
			{
				if (item->isSchemaItemAfb() == true)
				{
					SchemaItemAfb* schemaItemAfb = item->toSchemaItemAfb();
					assert(schemaItemAfb);

					if (schemaItemAfb->label().isEmpty() == true)
					{
						int labelCounter = this->nextCounterValue();
						schemaItemAfb->setLabel(schemaID() + "_" + QString::number(labelCounter));
					}
				}
			}
		}

		return true;
	}

	void LogicSchema::Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Schema::Draw(pDrawParam, clipRect);
		return;
    }

	QStringList LogicSchema::getSignalList() const
	{
		std::set<QString> signalMap;	// signal ids can be duplicated, std::set removes dupilcates

		for (std::shared_ptr<SchemaLayer> layer : Layers)
		{
			if (layer->compile() == true)
			{
				// Get all signals
				//
				for (std::shared_ptr<SchemaItem> item : layer->Items)
				{
					if (item->isType<VFrame30::SchemaItemSignal>() == true)
					{
						const VFrame30::SchemaItemSignal* itemSignal = item->toType<VFrame30::SchemaItemSignal>();
						assert(itemSignal);

						QStringList appSignals = itemSignal->appSignalIdList();

						for (const QString& id : appSignals)
						{
							signalMap.insert(id);
						}
					}
				}

				break;
			}
		}

		// Move set to list
		//
		QStringList result;
		result.reserve(static_cast<int>(signalMap.size()));

		for (const QString& id : signalMap)
		{
			result.append(id);
		}

		return result;
	}

	QStringList LogicSchema::getLabels() const
	{
		QStringList labels;	// signal ids can be duplicated, std::set removes dupilcates
		labels.reserve(256);

		for (std::shared_ptr<SchemaLayer> layer : Layers)
		{
			for (std::shared_ptr<SchemaItem> item : layer->Items)
			{
				if (item->isSchemaItemAfb() == true)
				{
					const VFrame30::SchemaItemAfb* afb = item->toType<VFrame30::SchemaItemAfb>();
					assert(afb);

					labels.append(afb->label());
				}
			}
		}

		return labels;
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

	QStringList LogicSchema::equipmentIdList() const
	{
		return m_equipmentIds;
	}

	void LogicSchema::setEquipmentIds(const QString& s)
	{
		m_equipmentIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
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
		return m_equipmentIds.size();
	}

	int LogicSchema::nextCounterValue()
	{
		return ++m_counter;
	}

}
