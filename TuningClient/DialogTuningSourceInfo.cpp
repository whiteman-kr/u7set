#include "DialogTuningSourceInfo.h"
#include "ui_DialogTuningSourceInfo.h"
#include "TcpTuningClient.h"

DialogTuningSourceInfo::DialogTuningSourceInfo(QWidget *parent, quint64 tuningSourceId) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::DialogTuningSourceInfo),
    m_tuningSourceId(tuningSourceId)
{
    ui->setupUi(this);

    TuningSource ts;

    if (theTcpTuningClient->tuningSourceInfo(m_tuningSourceId, ts) == true)
    {
        setWindowTitle(ts.m_info.equipmentid().c_str());
    }
    else
    {
        setWindowTitle("???");
    }

    QStringList headerLabels;
    headerLabels<<"Parameter";
    headerLabels<<"Value";

    ui->treeWidget->setColumnCount(headerLabels.size());
    ui->treeWidget->setHeaderLabels(headerLabels);

    QTreeWidgetItem* infoItem = new QTreeWidgetItem(QStringList()<<"Source info");

    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"id"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"equipmentID"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"caption"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"dataType"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"ip"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"port"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"channel"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"subsystemID"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"subsystem"));

    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"lmNumber"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"lmModuleType"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"lmAdapterID"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"lmDataEnable"));
    infoItem->addChild(new QTreeWidgetItem(QStringList()<<"lmDataID"));

    ui->treeWidget->addTopLevelItem(infoItem);

    infoItem->setExpanded(true);

    QTreeWidgetItem* stateItem = new QTreeWidgetItem(QStringList()<<"Source State");

    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"isReply"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"requestCount"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"replyCount"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"commandQueueSize"));

    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"errUntimelyReplay"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"errSent"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"errPartialSent"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"errReplySize"));
    stateItem->addChild(new QTreeWidgetItem(QStringList()<<"errNoReply"));

    ui->treeWidget->addTopLevelItem(stateItem);

    stateItem->setExpanded(true);

    QTreeWidgetItem* errorsRUPItem = new QTreeWidgetItem(QStringList()<<"errors in reply RupFrameHeader");

    errorsRUPItem->addChild(new QTreeWidgetItem(QStringList()<<"errRupProtocolVersion"));
    errorsRUPItem->addChild(new QTreeWidgetItem(QStringList()<<"errRupFrameSize"));
    errorsRUPItem->addChild(new QTreeWidgetItem(QStringList()<<"errRupNoTuningData"));
    errorsRUPItem->addChild(new QTreeWidgetItem(QStringList()<<"errRupModuleType"));
    errorsRUPItem->addChild(new QTreeWidgetItem(QStringList()<<"errRupFramesQuantity"));
    errorsRUPItem->addChild(new QTreeWidgetItem(QStringList()<<"errRupFrameNumber"));

    ui->treeWidget->addTopLevelItem(errorsRUPItem);

    QTreeWidgetItem* errorsFotipItem = new QTreeWidgetItem(QStringList()<<"errors in reply FotipHeader");

    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipProtocolVersion"));
    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipUniqueID"));
    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipLmNumber"));
    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipSubsystemCode"));

    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipOperationCode"));
    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipFrameSize"));
    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipRomSize"));
    errorsFotipItem->addChild(new QTreeWidgetItem(QStringList()<<"errFotipRomFrameSize"));

    ui->treeWidget->addTopLevelItem(errorsFotipItem);

    QTreeWidgetItem* errorsFotipFlagItem = new QTreeWidgetItem(QStringList()<<"errors reported by LM in reply FotipHeader.flags");

    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagBoundsCheckSuccess"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagWriteSuccess"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagDataTypeErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagOpCodeErr"));

    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagStartAddrErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagRomSizeErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagRomFrameSizeErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagFrameSizeErr"));

    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagProtocolVersionErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagSubsystemKeyErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagUniueIDErr"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagOffsetErr"));

    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagApplySuccess"));
    errorsFotipFlagItem->addChild(new QTreeWidgetItem(QStringList()<<"fotipFlagSetSOR"));

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

    if (theTcpTuningClient->tuningSourceInfo(m_tuningSourceId, ts) == false)
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

    item->child(c++)->setText(1, QString::number(ts.m_info.id()));
    item->child(c++)->setText(1, ts.m_info.equipmentid().c_str());
    item->child(c++)->setText(1, ts.m_info.caption().c_str());
    item->child(c++)->setText(1, QString::number(ts.m_info.datatype()));
    item->child(c++)->setText(1, ts.m_info.ip().c_str());
    item->child(c++)->setText(1, QString::number(ts.m_info.port()));
    item->child(c++)->setText(1, QString::number(ts.m_info.channel()));
    item->child(c++)->setText(1, QString::number(ts.m_info.subsystemid()));
    item->child(c++)->setText(1, ts.m_info.subsystem().c_str());

    item->child(c++)->setText(1, QString::number(ts.m_info.lmnumber()));
    item->child(c++)->setText(1, QString::number(ts.m_info.lmmoduletype()));
    item->child(c++)->setText(1, ts.m_info.lmadapterid().c_str());
    item->child(c++)->setText(1, QString::number(ts.m_info.lmdataenable()));
    item->child(c++)->setText(1, QString::number(ts.m_info.lmdataid()));

    // state

    item = ui->treeWidget->topLevelItem(1);
    if (item == nullptr)
    {
        assert(item);
        return;
    }

    c = 0;

    item->child(c++)->setText(1, ts.m_state.isreply() ? "Yes" : "No");
    item->child(c++)->setText(1, QString::number(ts.m_state.requestcount()));
    item->child(c++)->setText(1, QString::number(ts.m_state.replycount()));
    item->child(c++)->setText(1, QString::number(ts.m_state.commandqueuesize()));

    item->child(c++)->setText(1, QString::number(ts.m_state.erruntimelyreplay()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errsent()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errpartialsent()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errreplysize()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errnoreply()));

    // RupFrameHeader

    item = ui->treeWidget->topLevelItem(2);
    if (item == nullptr)
    {
        assert(item);
        return;
    }

    c = 0;

    item->child(c++)->setText(1, QString::number(ts.m_state.errrupprotocolversion()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errrupframesize()));
	item->child(c++)->setText(1, QString::number(ts.m_state.errrupnontuningdata()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errrupmoduletype()));

    item->child(c++)->setText(1, QString::number(ts.m_state.errrupframesquantity()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errrupframenumber()));

    // FotipHeader

    item = ui->treeWidget->topLevelItem(3);
    if (item == nullptr)
    {
        assert(item);
        return;
    }

    c = 0;

    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipprotocolversion()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipuniqueid()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errfotiplmnumber()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipsubsystemcode()));

    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipoperationcode()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipframesize()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipromsize()));
    item->child(c++)->setText(1, QString::number(ts.m_state.errfotipromframesize()));

    // FotipFlags

    item = ui->treeWidget->topLevelItem(4);
    if (item == nullptr)
    {
        assert(item);
        return;
    }

    c = 0;


    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagboundschecksuccess()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagwritesuccess()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagdatatypeerr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagopcodeerr()));

    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagstartaddrerr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagromsizeerr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagromframesizeerr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagframesizeerr()));

    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagprotocolversionerr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagsubsystemkeyerr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflaguniueiderr()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagoffseterr()));

    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagapplysuccess()));
    item->child(c++)->setText(1, QString::number(ts.m_state.fotipflagsetsor()));
}



