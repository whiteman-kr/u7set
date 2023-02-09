#include "MonitorSchema.h"
#include "Settings.h"
#include "SchemaItemValue.h"
#include "SchemaItemImageValue.h"

namespace VFrame30
{

	MonitorSchema::MonitorSchema(void) :
		Schema()
	{
		setUnit(SchemaUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		addLayer(std::make_shared<SchemaLayer>(this, "Drawing", true));
		addLayer(std::make_shared<SchemaLayer>(this, "Notes", false));

		setTagsList(QStringList{"monitor"});

		return;
	}
	
	MonitorSchema::~MonitorSchema(void)
	{
		//qDebug() << "MonitorSchema::~MonitorSchema(void)";
	}

	QStringList MonitorSchema::getSignalList() const
	{
		std::set<QString> signalMap;	// signal ids can be duplicated, std::set removes dupilcates

		for (const auto& layer : layers())
		{
			// Get all signals
			//
			for (const auto& item : layer->items())
			{
				if (item->isType<VFrame30::SchemaItemValue>() == true)
				{
					const VFrame30::SchemaItemValue* itemValue = item->toType<VFrame30::SchemaItemValue>();
					assert(itemValue);

					const QStringList& appSignals = itemValue->signalIds();
					for (const QString& id : appSignals)
					{
						signalMap.insert(id);
					}
				}

				if (const auto itemImageValue = item->toType<VFrame30::SchemaItemImageValue>();
					itemImageValue != nullptr)
				{
					const QStringList& appSignals = itemImageValue->signalIds();
					for (const QString& id : appSignals)
					{
						signalMap.insert(id);
					}
				}
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
}
