#include "DialogTuningSourceInfo.h"
#include "ui_DialogTuningSourceInfo.h"
#include "MainWindow.h"
#include "../lib/Tuning/TuningSignalManager.h"

DialogTuningSourceInfo::DialogTuningSourceInfo(TuningClientTcpClient* tcpClient, QWidget* parent, Hash tuningSourceEquipmentId) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningSourceEquipmentId(tuningSourceEquipmentId),
	ui(new Ui::DialogTuningSourceInfo),
	m_tcpClient(tcpClient)
{
	assert(tcpClient);

	setAttribute(Qt::WA_DeleteOnClose);

	ui->setupUi(this);

	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_tuningSourceEquipmentId, &ts) == true)
	{
		setWindowTitle(ts.info.lmequipmentid().c_str());
	}
	else
	{
		setWindowTitle("???");
	}

	QStringList headerLabels;
	headerLabels << tr("Parameter");
	headerLabels << tr("Value");
	headerLabels << QString();

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
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "controlIsActive"));
	stateItem->addChild(new QTreeWidgetItem(QStringList() << "setSOR"));

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

	if (m_tcpClient->tuningSourceInfo(m_tuningSourceEquipmentId, &ts) == false)
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

	item->setData(0, Qt::UserRole, 0);

	setChildText(item, c++, ts.info.id());
	setChildText(item, c++, ts.info.lmequipmentid().c_str());
	setChildText(item, c++, ts.info.lmcaption().c_str());
	setChildText(item, c++, ts.info.lmdatatype());
	setChildText(item, c++, ts.info.lmip().c_str());
	setChildText(item, c++, ts.info.lmport());
	setChildText(item, c++, ts.info.lmsubsystemchannel().c_str());
	setChildText(item, c++, ts.info.lmsubsystemid());
	setChildText(item, c++, ts.info.lmsubsystem().c_str());

	setChildText(item, c++, ts.info.lmnumber());
	setChildText(item, c++, ts.info.lmmoduletype());
	setChildText(item, c++, ts.info.lmadapterid().c_str());
	setChildText(item, c++, ts.info.lmdataenable());
	setChildText(item, c++, ts.info.lmdataid());

	// state

	item = ui->treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildText(item, c++, ts.state.isreply() ? "Yes" : "No");
	setChildText(item, c++, ts.state.requestcount());
	setChildText(item, c++, ts.state.replycount());
	setChildText(item, c++, ts.state.commandqueuesize());
	setChildText(item, c++, ts.state.controlisactive() ? "Yes" : "No");
	setChildText(item, c++, ts.state.setsor() ? "Yes" : "No");

	setChildText(item, c++, ts.state.erruntimelyreplay());
	setChildText(item, c++, ts.state.errsent());
	setChildText(item, c++, ts.state.errpartialsent());
	setChildText(item, c++, ts.state.errreplysize());
	setChildText(item, c++, ts.state.errnoreply());
	setChildText(item, c++, ts.state.erranaloglowboundcheck());
	setChildText(item, c++, ts.state.erranaloghighboundcheck());

	updateParentItemState(item);

	// RupFrameHeader

	item = ui->treeWidget->topLevelItem(2);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildText(item, c++, ts.state.errrupprotocolversion(), ts.previousState().errrupprotocolversion());
	setChildText(item, c++, ts.state.errrupframesize(), ts.previousState().errrupframesize());
	setChildText(item, c++, ts.state.errrupnontuningdata(), ts.previousState().errrupnontuningdata());
	setChildText(item, c++, ts.state.errrupmoduletype(), ts.previousState().errrupmoduletype());

	setChildText(item, c++, ts.state.errrupframesquantity(), ts.previousState().errrupframesquantity());
	setChildText(item, c++, ts.state.errrupframenumber(), ts.previousState().errrupframenumber());

	setChildText(item, c++, ts.state.errrupcrc(), ts.previousState().errrupcrc());

	updateParentItemState(item);

	// FotipHeader

	item = ui->treeWidget->topLevelItem(3);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildText(item, c++, ts.state.errfotipprotocolversion(), ts.previousState().errfotipprotocolversion());
	setChildText(item, c++, ts.state.errfotipuniqueid(), ts.previousState().errfotipuniqueid());
	setChildText(item, c++, ts.state.errfotiplmnumber(), ts.previousState().errfotiplmnumber());
	setChildText(item, c++, ts.state.errfotipsubsystemcode(), ts.previousState().errfotipsubsystemcode());

	setChildText(item, c++, ts.state.errfotipoperationcode(), ts.previousState().errfotipoperationcode());
	setChildText(item, c++, ts.state.errfotipframesize(), ts.previousState().errfotipframesize());
	setChildText(item, c++, ts.state.errfotipromsize(), ts.previousState().errfotipromsize());
	setChildText(item, c++, ts.state.errfotipromframesize(), ts.previousState().errfotipromframesize());

	updateParentItemState(item);

	// FotipFlags

	item = ui->treeWidget->topLevelItem(4);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildText(item, c++, ts.state.fotipflagboundschecksuccess());
	setChildText(item, c++, ts.state.fotipflagwritesuccess());
	setChildText(item, c++, ts.state.fotipflagdatatypeerr(), ts.previousState().fotipflagdatatypeerr());
	setChildText(item, c++, ts.state.fotipflagopcodeerr(), ts.previousState().fotipflagopcodeerr());

	setChildText(item, c++, ts.state.fotipflagstartaddrerr(), ts.previousState().fotipflagstartaddrerr());
	setChildText(item, c++, ts.state.fotipflagromsizeerr(), ts.previousState().fotipflagromsizeerr());
	setChildText(item, c++, ts.state.fotipflagromframesizeerr(), ts.previousState().fotipflagromframesizeerr());
	setChildText(item, c++, ts.state.fotipflagframesizeerr(), ts.previousState().fotipflagframesizeerr());

	setChildText(item, c++, ts.state.fotipflagprotocolversionerr(), ts.previousState().fotipflagprotocolversionerr());
	setChildText(item, c++, ts.state.fotipflagsubsystemkeyerr(), ts.previousState().fotipflagsubsystemkeyerr());
	setChildText(item, c++, ts.state.fotipflaguniueiderr(), ts.previousState().fotipflaguniueiderr());
	setChildText(item, c++, ts.state.fotipflagoffseterr(), ts.previousState().fotipflagoffseterr());

	setChildText(item, c++, ts.state.fotipflagapplysuccess());
	setChildText(item, c++, ts.state.fotipflagsetsor());

	updateParentItemState(item);
}

void DialogTuningSourceInfo::setChildText(QTreeWidgetItem* item, int childIndex, quint64 number, quint64 previousNumber)
{
	setChildText(item, childIndex, QString::number(number));

	// Highlight increasing number

	if (item == nullptr)
	{
		assert(item);
		return;
	}

	if (childIndex >= item->childCount())
	{
		assert(false);
		return;
	}

	QTreeWidgetItem* childItem = item->child(childIndex);
	if (childItem == nullptr)
	{
		assert(childItem);
		return;
	}

	if (number > previousNumber)
	{
		bool ok = false;
		int errorCount = item->data(0, Qt::UserRole).toInt(&ok);

		if (ok == true)
		{
			errorCount++;
			item->setData(0, Qt::UserRole, errorCount);
		}
		else
		{
			assert(ok);
		}

		childItem->setBackground(1, QBrush(Qt::red));
		childItem->setForeground(1, QBrush(Qt::white));
	}
	else
	{
		childItem->setBackground(1, QBrush(Qt::white));
		childItem->setForeground(1, QBrush(Qt::black));
	}
}

void DialogTuningSourceInfo::setChildText(QTreeWidgetItem* item, int childIndex, quint64 number)
{
	setChildText(item, childIndex, QString::number(number));
}

void DialogTuningSourceInfo::setChildText(QTreeWidgetItem* item, int childIndex, const QString& text)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	if (childIndex >= item->childCount())
	{
		assert(false);
		return;
	}

	QTreeWidgetItem* childItem = item->child(childIndex);
	if (childItem == nullptr)
	{
		assert(childItem);
		return;
	}

	childItem->setText(1, text);
}

void DialogTuningSourceInfo::updateParentItemState(QTreeWidgetItem* item)
{
	bool ok = false;
	int errorCount = item->data(0, Qt::UserRole).toInt(&ok);
	if (ok == false)
	{
		assert(ok);
		return;
	}

	if (errorCount > 0)
	{
		item->setBackground(1, QBrush(Qt::red));
		item->setForeground(1, QBrush(Qt::white));
		item->setText(1, tr("E: %1").arg(errorCount));
	}
	else
	{
		item->setBackground(1, QBrush(Qt::white));
		item->setForeground(1, QBrush(Qt::black));
		item->setText(1, QString());

	}

}
