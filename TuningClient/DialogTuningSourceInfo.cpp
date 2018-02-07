#include "DialogTuningSourceInfo.h"
#include "ui_DialogTuningSourceInfo.h"
#include "MainWindow.h"
#include "../lib/Tuning/TuningSignalManager.h"

DialogTuningSourceInfo::DialogTuningSourceInfo(TuningClientTcpClient* tcpClient, QWidget* parent, Hash tuningSourceHash) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningSourceHash(tuningSourceHash),
	ui(new Ui::DialogTuningSourceInfo),
	m_tcpClient(tcpClient)
{
	assert(tcpClient);

	ui->setupUi(this);

	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_tuningSourceHash, &ts) == true)
	{
		setWindowTitle(ts.info.equipmentid().c_str());
	}
	else
	{
		setWindowTitle("???");
	}

	QStringList headerLabels;
	headerLabels << tr("Parameter");
	headerLabels << tr("Value");

	ui->treeWidget->setColumnCount(headerLabels.size());
	ui->treeWidget->setHeaderLabels(headerLabels);

	QTreeWidgetItem* infoItem = new QTreeWidgetItem(QStringList() << tr("Source info"));

	infoItem->addChild(new QTreeWidgetItem(QStringList() << "id"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "equipmentID"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "caption"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "dataType"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "ip"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "port"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "channel"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "subsystemID"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "subsystem"));

	infoItem->addChild(new QTreeWidgetItem(QStringList() << "lmNumber"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "lmModuleType"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "lmAdapterID"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "lmDataEnable"));
	infoItem->addChild(new QTreeWidgetItem(QStringList() << "lmDataID"));

	ui->treeWidget->addTopLevelItem(infoItem);

	infoItem->setExpanded(true);

	QTreeWidgetItem* stateItem = new QTreeWidgetItem(QStringList() << tr("Source State"));

	stateItem->addChild(new QTreeWidgetItem(QStringList() << "isReply"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "requestCount"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "replyCount"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "commandQueueSize"));

	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errUntimelyReplay"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errSent"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errPartialSent"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errReplySize"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errNoReply"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errAnalogLowBoundCheck"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "errAnalogHighBoundCheck"));

	ui->treeWidget->addTopLevelItem(stateItem);

	stateItem->setExpanded(true);

	QTreeWidgetItem* errorsRUPItem = new QTreeWidgetItem(QStringList() << tr("errors in reply RupFrameHeader"));

	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupProtocolVersion"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupFrameSize"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupNoTuningData"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupModuleType"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupFramesQuantity"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupFrameNumber"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupCRC"));


	ui->treeWidget->addTopLevelItem(errorsRUPItem);

	QTreeWidgetItem* errorsFotipItem = new QTreeWidgetItem(QStringList() << tr("errors in reply FotipHeader"));

	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipProtocolVersion"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipUniqueID"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipLmNumber"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipSubsystemCode"));

	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipOperationCode"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipFrameSize"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipRomSize"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipRomFrameSize"));

	ui->treeWidget->addTopLevelItem(errorsFotipItem);

	QTreeWidgetItem* errorsFotipFlagItem = new QTreeWidgetItem(QStringList() << tr("errors reported by LM in reply FotipHeader.flags"));

	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagBoundsCheckSuccess"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagWriteSuccess"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagDataTypeErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagOpCodeErr"));

	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagStartAddrErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagRomSizeErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagRomFrameSizeErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagFrameSizeErr"));

	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagProtocolVersionErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagSubsystemKeyErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagUniueIDErr"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagOffsetErr"));

	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagApplySuccess"));
	errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList() << "fotipFlagSetSOR"));

	ui->treeWidget->addTopLevelItem(errorsFotipFlagItem);

	updateData();

	for (int i = 0; i < ui->treeWidget->columnCount(); i++)
	{
		ui->treeWidget->resizeColumnToContents(i);
	}

	m_updateStateTimerId = startTimer(250);

}

DialogTuningSourceInfo::~DialogTuningSourceInfo()
{
	delete ui;
}

void DialogTuningSourceInfo::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateData();
	}
}


void DialogTuningSourceInfo::updateData()
{
	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_tuningSourceHash, &ts) == false)
	{
		return;
	}

	// info

	QTreeWidgetItem* item = ui->treeWidget->topLevelItem(0);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	int c = 0;

	item->child(c++)->setText(1, QString::number(ts.info.id()));
	item->child(c++)->setText(1, ts.info.equipmentid().c_str());
	item->child(c++)->setText(1, ts.info.caption().c_str());
	item->child(c++)->setText(1, QString::number(ts.info.datatype()));
	item->child(c++)->setText(1, ts.info.ip().c_str());
	item->child(c++)->setText(1, QString::number(ts.info.port()));

	QChar chChannel = 'A' + ts.info.channel();

	item->child(c++)->setText(1, chChannel);
	item->child(c++)->setText(1, QString::number(ts.info.subsystemid()));
	item->child(c++)->setText(1, ts.info.subsystem().c_str());

	item->child(c++)->setText(1, QString::number(ts.info.lmnumber()));
	item->child(c++)->setText(1, QString::number(ts.info.lmmoduletype()));
	item->child(c++)->setText(1, ts.info.lmadapterid().c_str());
	item->child(c++)->setText(1, QString::number(ts.info.lmdataenable()));
	item->child(c++)->setText(1, QString::number(ts.info.lmdataid()));

	// state

	item = ui->treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->child(c++)->setText(1, ts.state.isreply() ? "Yes" : "No");
	item->child(c++)->setText(1, QString::number(ts.state.requestcount()));
	item->child(c++)->setText(1, QString::number(ts.state.replycount()));
	item->child(c++)->setText(1, QString::number(ts.state.commandqueuesize()));

	item->child(c++)->setText(1, QString::number(ts.state.erruntimelyreplay()));
	item->child(c++)->setText(1, QString::number(ts.state.errsent()));
	item->child(c++)->setText(1, QString::number(ts.state.errpartialsent()));
	item->child(c++)->setText(1, QString::number(ts.state.errreplysize()));
	item->child(c++)->setText(1, QString::number(ts.state.errnoreply()));
	item->child(c++)->setText(1, QString::number(ts.state.erranaloglowboundcheck()));
	item->child(c++)->setText(1, QString::number(ts.state.erranaloghighboundcheck()));

	// RupFrameHeader

	item = ui->treeWidget->topLevelItem(2);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->child(c++)->setText(1, QString::number(ts.state.errrupprotocolversion()));
	item->child(c++)->setText(1, QString::number(ts.state.errrupframesize()));
	item->child(c++)->setText(1, QString::number(ts.state.errrupnontuningdata()));
	item->child(c++)->setText(1, QString::number(ts.state.errrupmoduletype()));

	item->child(c++)->setText(1, QString::number(ts.state.errrupframesquantity()));
	item->child(c++)->setText(1, QString::number(ts.state.errrupframenumber()));

	item->child(c++)->setText(1, QString::number(ts.state.errrupcrc()));

	// FotipHeader

	item = ui->treeWidget->topLevelItem(3);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->child(c++)->setText(1, QString::number(ts.state.errfotipprotocolversion()));
	item->child(c++)->setText(1, QString::number(ts.state.errfotipuniqueid()));
	item->child(c++)->setText(1, QString::number(ts.state.errfotiplmnumber()));
	item->child(c++)->setText(1, QString::number(ts.state.errfotipsubsystemcode()));

	item->child(c++)->setText(1, QString::number(ts.state.errfotipoperationcode()));
	item->child(c++)->setText(1, QString::number(ts.state.errfotipframesize()));
	item->child(c++)->setText(1, QString::number(ts.state.errfotipromsize()));
	item->child(c++)->setText(1, QString::number(ts.state.errfotipromframesize()));

	// FotipFlags

	item = ui->treeWidget->topLevelItem(4);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;


	item->child(c++)->setText(1, QString::number(ts.state.fotipflagboundschecksuccess()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagwritesuccess()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagdatatypeerr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagopcodeerr()));

	item->child(c++)->setText(1, QString::number(ts.state.fotipflagstartaddrerr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagromsizeerr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagromframesizeerr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagframesizeerr()));

	item->child(c++)->setText(1, QString::number(ts.state.fotipflagprotocolversionerr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagsubsystemkeyerr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflaguniueiderr()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagoffseterr()));

	item->child(c++)->setText(1, QString::number(ts.state.fotipflagapplysuccess()));
	item->child(c++)->setText(1, QString::number(ts.state.fotipflagsetsor()));

}



