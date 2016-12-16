#include "DialogTuningSources.h"
#include "ui_DialogTuningSources.h"
#include "MainWindow.h"
#include "TuningObjectManager.h"
#include "DialogTuningSourceInfo.h"

DialogTuningSources::DialogTuningSources(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTuningSources)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	QStringList headerLabels;
	headerLabels<<"Id";
	headerLabels<<"EquipmentId";
	headerLabels<<"Caption";
	headerLabels<<"Ip";
	headerLabels<<"Port";
	headerLabels<<"Channel";
	headerLabels<<"SubsystemID";
	headerLabels<<"Subsystem";
	headerLabels<<"LmNumber";

    headerLabels<<"IsReply";
    headerLabels<<"RequestCount";
    headerLabels<<"ReplyCount";
    headerLabels<<"CommandQueueSize";

    ui->treeWidget->setColumnCount(headerLabels.size());
    ui->treeWidget->setHeaderLabels(headerLabels);

    update(false);

    connect(theObjectManager, &TuningObjectManager::tuningSourcesArrived, this, &DialogTuningSources::slot_tuningSourcesArrived);

	m_updateStateTimerId = startTimer(250);
}

DialogTuningSources::~DialogTuningSources()
{
	theDialogTuningSources = nullptr;
	delete ui;
}

void DialogTuningSources::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		update(true);
	}
}

void DialogTuningSources::slot_tuningSourcesArrived()
{
	update(false);
}

void DialogTuningSources::update(bool refreshOnly)
{
    std::vector<TuningSource> tsi = theObjectManager->tuningSourcesInfo();
	int count = static_cast<int>(tsi.size());

    if (ui->treeWidget->topLevelItemCount() != count)
    {
        refreshOnly = false;
    }

    if (refreshOnly == false)
    {
        ui->treeWidget->clear();

        for (int i = 0; i < count; i++)
        {
            QStringList connectionStrings;

            TuningSource& ts = tsi[i];

            connectionStrings << QString::number(ts.m_info.id());
            connectionStrings << ts.m_info.equipmentid().c_str();
            connectionStrings << ts.m_info.caption().c_str();
            connectionStrings << ts.m_info.ip().c_str();
            connectionStrings << QString::number(ts.m_info.port());

            QChar chChannel = 'A' + ts.m_info.channel();
            connectionStrings << chChannel;

            connectionStrings << QString::number(ts.m_info.subsystemid());
            connectionStrings << ts.m_info.subsystem().c_str();
            connectionStrings << QString::number(ts.m_info.lmnumber());

            QTreeWidgetItem* item = new QTreeWidgetItem(connectionStrings);

            item->setData(0, Qt::UserRole, ts.m_info.id());

            ui->treeWidget->addTopLevelItem(item);
        }

        for (int i = 0; i < ui->treeWidget->columnCount(); i++)
        {
            ui->treeWidget->resizeColumnToContents(i);
        }
    }

    const int dynamicColumn = 9;

    for (int i = 0; i < count; i++)
	{
		TuningSource& ts = tsi[i];

        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);

        if (item == nullptr)
        {
            assert(false);
            continue;
        }

        int col = dynamicColumn;

        item->setText(col++, ts.m_state.isreply() ? "Yes" : "No");
        item->setText(col++, QString::number(ts.m_state.requestcount()));
        item->setText(col++, QString::number(ts.m_state.replycount()));
        item->setText(col++, QString::number(ts.m_state.commandqueuesize()));
    }

}

DialogTuningSources* theDialogTuningSources = nullptr;

void DialogTuningSources::on_treeWidget_doubleClicked(const QModelIndex &index)
{
    QTreeWidgetItem* item = ui->treeWidget->topLevelItem(index.row());

    if (item == nullptr)
    {
        assert(false);
        return;
    }

    quint64 id = item->data(0, Qt::UserRole).value<quint64>();

    DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(this, id);
    dlg->exec();
}
