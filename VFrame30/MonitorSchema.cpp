#include "MonitorSchema.h"
#include "Settings.h"
#include "SchemaItemValue.h"

namespace VFrame30
{

	MonitorSchema::MonitorSchema(void)
	{
		qDebug() << "MonitorSchema::MonitorSchema(void)";

		setUnit(SchemaUnit::Display);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(20);

		setDocWidth(1000);
		setDocHeight(750);

		setBackgroundColor(qRgb(0xF8, 0xF8, 0xF8));

		Layers.push_back(std::make_shared<SchemaLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemaLayer>("Notes", false));

		return;
	}
	
	MonitorSchema::~MonitorSchema(void)
	{
		qDebug() << "MonitorSchema::~MonitorSchema(void)";
	}

	QStringList MonitorSchema::getSignalList() const
	{
		std::set<QString> signalMap;	// signal ids can be duplicated, std::set removes dupilcates

		for (std::shared_ptr<SchemaLayer> layer : Layers)
		{
			// Get all signals
			//
			for (std::shared_ptr<SchemaItem> item : layer->Items)
			{
				if (item->isType<VFrame30::SchemaItemValue>() == true)
				{
					const VFrame30::SchemaItemValue* itemValue = item->toType<VFrame30::SchemaItemValue>();
					assert(itemValue);

					QStringList appSignals;
					appSignals << itemValue->signalId();

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
