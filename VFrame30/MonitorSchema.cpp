#include "MonitorSchema.h"
#include "Settings.h"

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
		QStringList result;
		return result;
	}
}
