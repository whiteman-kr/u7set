#include "DevToolsConnections.h"
#include "TcpStatisticsWidget.h"

namespace SchemaClientLib
{
	DevToolsConnections::DevToolsConnections(QWidget* parent) :
		QWidget{parent}
	{
		auto tcpStatisticsWidget = new TcpStatisticsWidget{false, this};

		auto layout = new QVBoxLayout{this};
		layout->addWidget(tcpStatisticsWidget);
		setLayout(layout);

		return;
	}
} // namespace SchemaClientLib