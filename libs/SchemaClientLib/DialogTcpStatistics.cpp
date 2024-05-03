#include <SchemaClientLib/DialogTcpStatistics.h>

#include "TcpStatisticsWidget.h"

namespace SchemaClientLib
{
	//
	// DialogStatistics
	//
	DialogTcpStatistics::DialogTcpStatistics(QWidget* parent) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
	{
		setWindowTitle(tr("Connections Statistics"));

		setAttribute(Qt::WA_DeleteOnClose);

		QVBoxLayout* mainLayout = new QVBoxLayout();

		auto tcpStatisticsWidget = new TcpStatisticsWidget{true, this};
		mainLayout->addWidget(tcpStatisticsWidget);

		connect(tcpStatisticsWidget, &TcpStatisticsWidget::closeClicked, this, &DialogTcpStatistics::reject);

		setLayout(mainLayout);

		return;
	}

	void DialogTcpStatistics::reject()
	{
		emit dialogClosed();
		QDialog::reject();
	}
} // namespace SchemaClientLib