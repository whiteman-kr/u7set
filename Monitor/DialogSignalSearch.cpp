#include "DialogSignalSearch.h"
#include "ui_DialogSignalSearch.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"
#include "Settings.h"


QString DialogSignalSearch::m_signalId = "";

DialogSignalSearch::DialogSignalSearch(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalSearch)
{
	ui->setupUi(this);

	// Restore window pos
	//
	if (theSettings.m_signalSearchPos.x() != -1 && theSettings.m_signalSearchPos.y() != -1)
	{
		move(theSettings.m_signalSearchPos);
		restoreGeometry(theSettings.m_signalSearchGeometry);
	}

	// Restore columns width
	//
	QDataStream stream(&theSettings.m_signalSearchColumnWidth, QIODevice::ReadOnly);

	for (int i = 0; i < theSettings.m_signalSearchColumnCount; i++)
	{
		int width;
		stream >> width;
		ui->signalsTree->setColumnWidth(i, width);
	}

	//

	QStringList columns;
	columns << "SignalID";
	columns << "Caption";
	ui->signalsTree->setHeaderLabels(columns);

	ui->signalsTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->signalsTree, &QTreeWidget::customContextMenuRequested,this, &DialogSignalSearch::prepareContextMenu);

	ui->editSignalID->setText(m_signalId);
	ui->editSignalID->setPlaceholderText(tr("Enter SignalID here"));

	m_signals = theSignals.signalList();

	search();
}

DialogSignalSearch::~DialogSignalSearch()
{
	delete ui;
}

void DialogSignalSearch::on_editSignalID_textEdited(const QString &arg1)
{
	m_signalId = arg1;
	search();

}
void DialogSignalSearch::search()
{
	ui->signalsTree->clear();

	for (const AppSignalParam& s : m_signals)
	{
		if (s.customSignalId().startsWith(m_signalId, Qt::CaseInsensitive) == false)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << s.customSignalId() << s.caption());
		//item->setData(0, Qt::UserRole, qVariantFromValue((AppSignalParam*)&s));

		item->setData(0, Qt::UserRole, QVariant::fromValue<AppSignalParam>(s));

		ui->signalsTree->addTopLevelItem(item);
	}

	ui->labelFound->setText(QString("Signals found: %1").arg(ui->signalsTree->topLevelItemCount()));

}

void DialogSignalSearch::on_DialogSignalSearch_finished(int result)
{
	Q_UNUSED(result);

	// Save columns width
	//
	theSettings.m_signalSearchColumnWidth.clear();

	QDataStream stream(&theSettings.m_signalSearchColumnWidth, QIODevice::WriteOnly);

	for (int i = 0; i < ui->signalsTree->columnCount(); i++)
	{
		stream << (int)ui->signalsTree->columnWidth(i);
	}
	theSettings.m_signalSearchColumnCount = ui->signalsTree->columnCount();

	// Save window position
	//
	theSettings.m_signalSearchPos = pos();
	theSettings.m_signalSearchGeometry = saveGeometry();
}

void DialogSignalSearch::prepareContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		assert(cw);
		return;
	}

	QTreeWidgetItem* item = ui->signalsTree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	AppSignalParam signal = item->data(0, Qt::UserRole).value<AppSignalParam>();
	cw->currentTab()->signalContextMenu(QStringList() << signal.appSignalId());

	return;
}



void DialogSignalSearch::on_signalsTree_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		assert(cw);
		return;
	}

	QTreeWidgetItem* item = ui->signalsTree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	AppSignalParam signal = item->data(0, Qt::UserRole).value<AppSignalParam>();
	cw->currentTab()->signalInfo(signal.appSignalId());

	return;
}
