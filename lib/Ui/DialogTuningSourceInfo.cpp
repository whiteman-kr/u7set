#include "DialogTuningSourceInfo.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningTcpClient.h"

DialogTuningSourceInfo::DialogTuningSourceInfo(TuningTcpClient* tcpClient, QWidget* parent, Hash tuningSourceEquipmentId) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningSourceEquipmentId(tuningSourceEquipmentId),
	m_tcpClient(tcpClient)
{
	setAttribute(Qt::WA_DeleteOnClose);

	if (m_tcpClient == nullptr)
	{
		assert(m_tcpClient);
		return;
	}

	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_tuningSourceEquipmentId, &ts) == true)
	{
		setWindowTitle(ts.info.lmequipmentid().c_str());
	}
	else
	{
		setWindowTitle("???");
	}

	//

	QHBoxLayout* l = new QHBoxLayout();

	m_treeWidget = new QTreeWidget();

	l->addWidget(m_treeWidget);

	setLayout(l);

	setFixedSize(640, 600);

	QStringList headerLabels;
	headerLabels << tr("Parameter");
	headerLabels << tr("Value");
	headerLabels << QString();

	m_treeWidget->setColumnCount(headerLabels.size());
	m_treeWidget->setHeaderLabels(headerLabels);

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

	m_treeWidget->addTopLevelItem(infoItem);

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

	m_treeWidget->addTopLevelItem(stateItem);

	stateItem->setExpanded(true);

	QTreeWidgetItem* errorsRUPItem = new QTreeWidgetItem(QStringList() << tr("errors in reply RupFrameHeader"));

	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupProtocolVersion"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupFrameSize"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupNoTuningData"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupModuleType"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupFramesQuantity"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupFrameNumber"));
	errorsRUPItem->addChild(new QTreeWidgetItem(QStringList() << "errRupCRC"));

	m_treeWidget->addTopLevelItem(errorsRUPItem);

	QTreeWidgetItem* errorsFotipItem = new QTreeWidgetItem(QStringList() << tr("errors in reply FotipHeader"));

	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipProtocolVersion"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipUniqueID"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipLmNumber"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipSubsystemCode"));

	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipOperationCode"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipFrameSize"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipRomSize"));
	errorsFotipItem->addChild(new QTreeWidgetItem(QStringList() << "errFotipRomFrameSize"));

	m_treeWidget->addTopLevelItem(errorsFotipItem);

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

	m_treeWidget->addTopLevelItem(errorsFotipFlagItem);

	updateData();

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_updateStateTimerId = startTimer(250);

}

DialogTuningSourceInfo::~DialogTuningSourceInfo()
{

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
	if (m_tcpClient == nullptr)
	{
		assert(m_tcpClient);
		return;
	}

	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_tuningSourceEquipmentId, &ts) == false)
	{
		return;
	}

	// info

	QTreeWidgetItem* item = m_treeWidget->topLevelItem(0);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	int c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildNumber(item, c++, ts.info.id());
	setChildText(item, c++, ts.info.lmequipmentid().c_str());
	setChildText(item, c++, ts.info.lmcaption().c_str());
	setChildNumber(item, c++, ts.info.lmdatatype());
	setChildText(item, c++, ts.info.lmip().c_str());
	setChildNumber(item, c++, ts.info.lmport());
	setChildText(item, c++, ts.info.lmsubsystemchannel().c_str());
	setChildNumber(item, c++, ts.info.lmsubsystemid());
	setChildText(item, c++, ts.info.lmsubsystem().c_str());

	setChildNumber(item, c++, ts.info.lmnumber());
	setChildNumber(item, c++, ts.info.lmmoduletype());
	setChildText(item, c++, ts.info.lmadapterid().c_str());
	setChildNumber(item, c++, ts.info.lmdataenable());
	setChildNumber(item, c++, ts.info.lmdataid());

	// state

	item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildText(item, c++, ts.state.isreply() ? "Yes" : "No");
	setChildNumber(item, c++, ts.state.requestcount());
	setChildNumber(item, c++, ts.state.replycount());
	setChildNumber(item, c++, ts.state.commandqueuesize());
	setChildText(item, c++, ts.state.controlisactive() ? "Yes" : "No");
	setChildText(item, c++, ts.state.setsor() ? "Yes" : "No");

	setChildNumber(item, c++, ts.state.erruntimelyreplay());
	setChildNumber(item, c++, ts.state.errsent());
	setChildNumber(item, c++, ts.state.errpartialsent());
	setChildNumber(item, c++, ts.state.errreplysize());
	setChildNumberDelta(item, c++, ts.state.errnoreply(), ts.previousState().errnoreply());
	setChildNumber(item, c++, ts.state.erranaloglowboundcheck());
	setChildNumber(item, c++, ts.state.erranaloghighboundcheck());

	updateParentItemState(item);

	// RupFrameHeader

	item = m_treeWidget->topLevelItem(2);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildNumberDelta(item, c++, ts.state.errrupprotocolversion(), ts.previousState().errrupprotocolversion());
	setChildNumberDelta(item, c++, ts.state.errrupframesize(), ts.previousState().errrupframesize());
	setChildNumberDelta(item, c++, ts.state.errrupnontuningdata(), ts.previousState().errrupnontuningdata());
	setChildNumberDelta(item, c++, ts.state.errrupmoduletype(), ts.previousState().errrupmoduletype());

	setChildNumberDelta(item, c++, ts.state.errrupframesquantity(), ts.previousState().errrupframesquantity());
	setChildNumberDelta(item, c++, ts.state.errrupframenumber(), ts.previousState().errrupframenumber());

	setChildNumberDelta(item, c++, ts.state.errrupcrc(), ts.previousState().errrupcrc());

	updateParentItemState(item);

	// FotipHeader

	item = m_treeWidget->topLevelItem(3);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildNumberDelta(item, c++, ts.state.errfotipprotocolversion(), ts.previousState().errfotipprotocolversion());
	setChildNumberDelta(item, c++, ts.state.errfotipuniqueid(), ts.previousState().errfotipuniqueid());
	setChildNumberDelta(item, c++, ts.state.errfotiplmnumber(), ts.previousState().errfotiplmnumber());
	setChildNumberDelta(item, c++, ts.state.errfotipsubsystemcode(), ts.previousState().errfotipsubsystemcode());

	setChildNumberDelta(item, c++, ts.state.errfotipoperationcode(), ts.previousState().errfotipoperationcode());
	setChildNumberDelta(item, c++, ts.state.errfotipframesize(), ts.previousState().errfotipframesize());
	setChildNumberDelta(item, c++, ts.state.errfotipromsize(), ts.previousState().errfotipromsize());
	setChildNumberDelta(item, c++, ts.state.errfotipromframesize(), ts.previousState().errfotipromframesize());

	updateParentItemState(item);

	// FotipFlags

	item = m_treeWidget->topLevelItem(4);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setChildNumber(item, c++, ts.state.fotipflagboundschecksuccess());
	setChildNumber(item, c++, ts.state.fotipflagwritesuccess());
	setChildNumberDelta(item, c++, ts.state.fotipflagdatatypeerr(), ts.previousState().fotipflagdatatypeerr());
	setChildNumberDelta(item, c++, ts.state.fotipflagopcodeerr(), ts.previousState().fotipflagopcodeerr());

	setChildNumberDelta(item, c++, ts.state.fotipflagstartaddrerr(), ts.previousState().fotipflagstartaddrerr());
	setChildNumberDelta(item, c++, ts.state.fotipflagromsizeerr(), ts.previousState().fotipflagromsizeerr());
	setChildNumberDelta(item, c++, ts.state.fotipflagromframesizeerr(), ts.previousState().fotipflagromframesizeerr());
	setChildNumberDelta(item, c++, ts.state.fotipflagframesizeerr(), ts.previousState().fotipflagframesizeerr());

	setChildNumberDelta(item, c++, ts.state.fotipflagprotocolversionerr(), ts.previousState().fotipflagprotocolversionerr());
	setChildNumberDelta(item, c++, ts.state.fotipflagsubsystemkeyerr(), ts.previousState().fotipflagsubsystemkeyerr());
	setChildNumberDelta(item, c++, ts.state.fotipflaguniueiderr(), ts.previousState().fotipflaguniueiderr());
	setChildNumberDelta(item, c++, ts.state.fotipflagoffseterr(), ts.previousState().fotipflagoffseterr());

	setChildNumber(item, c++, ts.state.fotipflagapplysuccess());
	setChildNumber(item, c++, ts.state.fotipflagsetsor());

	updateParentItemState(item);
}

void DialogTuningSourceInfo::setChildNumberDelta(QTreeWidgetItem* item, int childIndex, quint64 number, quint64 previousNumber)
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

		//childItem->setBackground(1, QBrush(Qt::red));
		//childItem->setForeground(1, QBrush(Qt::white));
		childItem->setForeground(1, QBrush(Qt::red));
	}
	else
	{
		//childItem->setBackground(1, QBrush(Qt::white));
		//childItem->setForeground(1, QBrush(Qt::black));
		childItem->setForeground(1, QBrush(Qt::black));
	}
}

void DialogTuningSourceInfo::setChildNumber(QTreeWidgetItem* item, int childIndex, quint64 number)
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
		//item->setBackground(1, QBrush(Qt::red));
		//item->setForeground(1, QBrush(Qt::white));
		item->setForeground(1, QBrush(Qt::red));
		item->setText(1, tr("E: %1").arg(errorCount));
	}
	else
	{
		//item->setBackground(1, QBrush(Qt::white));
		item->setForeground(1, QBrush(Qt::black));
		item->setText(1, QString());

	}

}
