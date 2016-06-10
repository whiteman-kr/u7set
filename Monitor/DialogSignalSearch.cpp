#include "DialogSignalSearch.h"
#include "ui_DialogSignalSearch.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"
#include "Settings.h"


QString DialogSignalSearch::m_signalID = "";
int DialogSignalSearch::m_signalTypeIndex = 0;

DialogSignalSearch::DialogSignalSearch(QWidget *parent) :
	QDialog(parent),
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

	ui->comboSignalType->blockSignals(true);
	ui->comboSignalType->addItem(tr("Analog"));
	ui->comboSignalType->addItem(tr("Discrete"));
	ui->comboSignalType->setCurrentIndex(m_signalTypeIndex);
	ui->comboSignalType->blockSignals(false);

	ui->editSignalID->setText(m_signalID);

	m_signals = theSignals.signalList();

	search();
}

DialogSignalSearch::~DialogSignalSearch()
{
	delete ui;
}

void DialogSignalSearch::on_editSignalID_textEdited(const QString &arg1)
{
	m_signalID = arg1;
	search();

}

void DialogSignalSearch::on_comboSignalType_currentIndexChanged(int index)
{
	m_signalTypeIndex = index;
	search();
}

void DialogSignalSearch::search()
{
	ui->signalsTree->clear();

	if (m_signalID.isEmpty() == false)
	{
		for (const Signal& s : m_signals)
		{
			if (m_signalTypeIndex == 0 && s.isDiscrete())
			{
				continue;
			}
			if (m_signalTypeIndex == 1 && s.isAnalog())
			{
				continue;
			}
			if (s.customAppSignalID().startsWith(m_signalID, Qt::CaseInsensitive) == false)
			{
				continue;
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<s.customAppSignalID()<<s.caption());
			item->setData(0, Qt::UserRole, qVariantFromValue((Signal*)&s));

			ui->signalsTree->addTopLevelItem(item);
		}
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

	MonitorMainWindow* mainWindow = dynamic_cast<MonitorMainWindow*>(parent());
	if (mainWindow == nullptr)
	{
		assert(mainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(mainWindow->centralWidget());
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

	Signal* signal = (Signal*)(item->data(0, Qt::UserRole).value<Signal*>());
	if (signal == nullptr)
	{
		assert(signal);
		return;
	}

	cw->currentTab()->signalContextMenu(QStringList()<<signal->appSignalID());
}


