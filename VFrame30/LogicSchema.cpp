#include "LogicSchema.h"
#include "SchemaItemAfb.h"
#include "SchemaItemSignal.h"
#include "SchemaItemConnection.h"
#include "PropertyNames.h"

namespace VFrame30
{
	LogicSchema::LogicSchema(void)
	{
		//qDebug() << "LogicSchema::LogicSchema(void)";

		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::equipmentIds, true, LogicSchema::equipmentIds, LogicSchema::setEquipmentIds);
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::lmDescriptionFile, true, LogicSchema::lmDescriptionFile, LogicSchema::setLmDescriptionFile);

		setUnit(SchemaUnit::Inch);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(4);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemaLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemaLayer>("Logic", true));
		Layers.push_back(std::make_shared<SchemaLayer>("Notes", false));

		setTagsList(QStringList{"ApplicationLogic"});

		return;
	}

	LogicSchema ::~LogicSchema (void)
	{
		//qDebug() << "LogicSchema::~LogicSchema(void)  SchemaID = " << schemaId();
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
		ls->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());

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

		// Set the right order for layers, a lot of layers were saved in wrong order (logic, frame, notes)
		// We need order Frame, Logic, Notes
		//
		int frameLayerIndex = -1;
		int logicLayerIndex = -1;

		for (size_t layerIndex = 0; layerIndex < Layers.size(); layerIndex++)
		{
			if (Layers[layerIndex]->name() == QLatin1String("Frame"))
			{
				frameLayerIndex = static_cast<int>(layerIndex);
			}

			if (Layers[layerIndex]->name() == QLatin1String("Logic"))
			{
				logicLayerIndex = static_cast<int>(layerIndex);
			}
		}

		if (frameLayerIndex != -1 &&
			logicLayerIndex != -1 &&
			logicLayerIndex < frameLayerIndex)
		{
			std::swap(Layers[frameLayerIndex], Layers[logicLayerIndex]);
			setActiveLayer(Layers[frameLayerIndex]);	// frameLayerIndex after swap points to LogicLayer
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
			Q_ASSERT(layer);

			for (std::shared_ptr<SchemaItem> item : layer->Items)
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

	void LogicSchema::Draw(CDrawParam* drawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Schema::Draw(drawParam, clipRect);
		return;
    }

	std::set<QString> LogicSchema::getSignalMap() const
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

						appSignals = itemSignal->impactAppSignalIdList();
						for (const QString& id : appSignals)
						{
							signalMap.insert(id);
						}
					}

					if (const VFrame30::SchemaItemReceiver* itemReceiver = item->toType<VFrame30::SchemaItemReceiver>();
						itemReceiver != nullptr)
					{
						for (const QString& appSignalId : itemReceiver->appSignalIdsAsList())
						{
							signalMap.insert(appSignalId);
						}
					}
				}

				break;
			}
		}

		return signalMap;
	}


	QStringList LogicSchema::getSignalList() const
	{
		std::set<QString> signalMap = getSignalMap();	// signal ids can be duplicated, std::set removes dupilcates

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
		return m_equipmentIds.size();
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

}
