#include "TuningSourceWidget.h"
#include "../Proto/network.pb.h"
#include <functional>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include "TcpTuningServiceClient.h"
#include <QHBoxLayout>
#include <QSplitter>
#include "../lib/WidgetUtils.h"

struct staticPropertyFieldDefinition
{
	QString fieldName;
	std::function<QString(const Network::DataSourceInfo& info)> fieldValueGetter;
};

struct dynamicPropertyFieldDefinition
{
	QString fieldName;
	std::function<QString(const Network::TuningSourceState& state)> fieldValueGetter;
};

static const QList<staticPropertyFieldDefinition> staticPropertiesFieldList {
	{ QStringLiteral("EquipmentID"), [](const Network::DataSourceInfo& info) { return QString::fromStdString(info.equipmentid()); } },
	{ QStringLiteral("Caption"), [](const Network::DataSourceInfo& info) { return QString::fromStdString(info.caption()); } },
	{ QStringLiteral("DataType"), [](const Network::DataSourceInfo& info) { return QString::number(info.datatype()); } },
	{ QStringLiteral("IP"), [](const Network::DataSourceInfo& info) { return QString::fromStdString(info.ip()); } },
	{ QStringLiteral("Port"), [](const Network::DataSourceInfo& info) { return QString::number(info.port()); } },
	{ QStringLiteral("Channel"), [](const Network::DataSourceInfo& info) { return QString::number(info.channel()); } },
	{ QStringLiteral("SubsystemID"), [](const Network::DataSourceInfo& info) { return QString::number(info.subsystemid()); } },
	{ QStringLiteral("Subsystem"), [](const Network::DataSourceInfo& info) { return QString::fromStdString(info.subsystem()); } },

	{ QStringLiteral("LmNumber"), [](const Network::DataSourceInfo& info) { return QString::number(info.lmnumber()); } },
	{ QStringLiteral("LmModuleType"), [](const Network::DataSourceInfo& info) { return QString::number(info.lmmoduletype()); } },
	{ QStringLiteral("LmAdapterID"), [](const Network::DataSourceInfo& info) { return QString::fromStdString(info.lmadapterid()); } },
	{ QStringLiteral("LmDataEnable"), [](const Network::DataSourceInfo& info) { return info.lmdataenable() ? "Yes" : "No"; } },
	{ QStringLiteral("LmDataID"), [](const Network::DataSourceInfo& info) { return QString("0x%1").arg(info.lmdataid(), sizeof(info.lmdataid()) * 2, 16, QChar('0')); } },

	{ QStringLiteral("SourceID"), [](const Network::DataSourceInfo& info) { return "0x" + QString("%1").arg(info.id(), sizeof(info.id()) * 2, 16, QChar('0')).toUpper(); } },
};

static const QList<dynamicPropertyFieldDefinition> dynamicPropertiesFieldList {
	{ QStringLiteral("IsReply"), [](const Network::TuningSourceState& state) { return state.isreply() ? "Yes" : "No"; } },

	{ QStringLiteral("ControlIsActive"), [](const Network::TuningSourceState& state) { return state.controlisactive() ? "Yes" : "No"; } },
	{ QStringLiteral("SetSOR"), [](const Network::TuningSourceState& state) { return state.setsor() ? "Yes" : "No"; } },

	{ QStringLiteral("HasUnappliedParams"), [](const Network::TuningSourceState& state) { return state.hasunappliedparams() ? "Yes" : "No"; } },

	{ QStringLiteral("RequestCount"), [](const Network::TuningSourceState& state) { return QString::number(state.requestcount()); } },
	{ QStringLiteral("ReplyCount"), [](const Network::TuningSourceState& state) { return QString::number(state.replycount()); } },

	{ QStringLiteral("CommandQueueSize"), [](const Network::TuningSourceState& state) { return QString::number(state.commandqueuesize()); } },

	{ QStringLiteral("ErrUntimelyReplay"), [](const Network::TuningSourceState& state) { return QString::number(state.erruntimelyreplay()); } },
	{ QStringLiteral("ErrSent"), [](const Network::TuningSourceState& state) { return QString::number(state.errsent()); } },
	{ QStringLiteral("ErrPartialSent"), [](const Network::TuningSourceState& state) { return QString::number(state.errpartialsent()); } },
	{ QStringLiteral("ErrReplySize"), [](const Network::TuningSourceState& state) { return QString::number(state.errreplysize()); } },
	{ QStringLiteral("ErrNoReply"), [](const Network::TuningSourceState& state) { return QString::number(state.errnoreply()); } },

	// errors in reply RupFrameHeader
	//
	{ QStringLiteral("ErrRupProtocolVersion"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupprotocolversion()); } },
	{ QStringLiteral("ErrRupFrameSize"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupframesize()); } },
	{ QStringLiteral("ErrRupNonTuningData"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupnontuningdata()); } },
	{ QStringLiteral("ErrRupModuleType"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupmoduletype()); } },
	{ QStringLiteral("ErrRupFramesQuantity"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupframesquantity()); } },
	{ QStringLiteral("ErrRupFrameNumber"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupframenumber()); } },

	// errors in reply FotipHeader
	//
	{ QStringLiteral("ErrFotipProtocolVersion"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipprotocolversion()); } },
	{ QStringLiteral("ErrFotipUniqueID"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipuniqueid()); } },
	{ QStringLiteral("ErrFotipLmNumber"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotiplmnumber()); } },
	{ QStringLiteral("ErrFotipSubsystemCode"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipsubsystemcode()); } },
	{ QStringLiteral("ErrFotipOperationCode"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipoperationcode()); } },
	{ QStringLiteral("ErrFotipFrameSize"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipframesize()); } },
	{ QStringLiteral("ErrFotipRomSize"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipromsize()); } },
	{ QStringLiteral("ErrFotipRomFrameSize"), [](const Network::TuningSourceState& state) { return QString::number(state.errfotipromframesize()); } },

	// errors reported by LM in reply FotipHeader.flags
	//
	{ QStringLiteral("FotipFlagBoundsCheckSuccess"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagboundschecksuccess()); } },
	{ QStringLiteral("FotipFlagWriteSuccess"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagwritesuccess()); } },
	{ QStringLiteral("FotipFlagDataTypeErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagdatatypeerr()); } },
	{ QStringLiteral("FotipFlagOpCodeErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagopcodeerr()); } },
	{ QStringLiteral("FotipFlagStartAddrErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagstartaddrerr()); } },
	{ QStringLiteral("FotipFlagRomSizeErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagromsizeerr()); } },
	{ QStringLiteral("FotipFlagRomFrameSizeErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagromframesizeerr()); } },
	{ QStringLiteral("FotipFlagFrameSizeErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagframesizeerr()); } },
	{ QStringLiteral("FotipFlagProtocolVersionErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagprotocolversionerr()); } },
	{ QStringLiteral("FotipFlagSubsystemKeyErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagsubsystemkeyerr()); } },
	{ QStringLiteral("FotipFlagUniueIDErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflaguniueiderr()); } },
	{ QStringLiteral("FotipFlagOffsetErr"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagoffseterr()); } },
	{ QStringLiteral("FotipFlagApplySuccess"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagapplysuccess()); } },
	{ QStringLiteral("FotipFlagSetSOR"), [](const Network::TuningSourceState& state) { return QString::number(state.fotipflagsetsor()); } },

	{ QStringLiteral("ErrAnalogLowBoundCheck"), [](const Network::TuningSourceState& state) { return QString::number(state.erranaloglowboundcheck()); } },
	{ QStringLiteral("ErrAnalogHighBoundCheck"), [](const Network::TuningSourceState& state) { return QString::number(state.erranaloghighboundcheck()); } },

	{ QStringLiteral("ErrRupCRC"), [](const Network::TuningSourceState& state) { return QString::number(state.errrupcrc()); } },
};

TuningSourceWidget::TuningSourceWidget(quint64 id, QString equipmentId, QWidget *parent) :
	QWidget(parent),
	m_id(id),
	m_equipmentId(equipmentId)
{
	setAttribute(Qt::WA_DeleteOnClose);
	QHBoxLayout* hl = new QHBoxLayout();
	m_splitter = new QSplitter(this);
	hl->addWidget(m_splitter);
	setLayout(hl);

	// Source info
	//
	m_infoTable = new QTableView(this);
	m_infoModel = new QStandardItemModel(staticPropertiesFieldList.count(), 2, this);

	for (int i = 0; i < staticPropertiesFieldList.count(); i++)
	{
		auto& field = staticPropertiesFieldList[i];

		m_infoModel->setData(m_infoModel->index(i, 0), field.fieldName);
	}

	initTable(m_infoTable, m_infoModel);

	// Source state
	//
	m_stateTable = new QTableView(this);
	m_stateModel = new QStandardItemModel(dynamicPropertiesFieldList.count(), 2, this);

	for (int i = 0; i < dynamicPropertiesFieldList.count(); i++)
	{
		auto& field = dynamicPropertiesFieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 0), field.fieldName);
	}

	initTable(m_stateTable, m_stateModel);

	setWindowTitle(equipmentId);

	setWindowPosition(this, "TuningSourceWidget/" + equipmentId + "/geometry");

	QSettings settings;
	m_splitter->restoreState(settings.value("TuningSourceWidget/" + equipmentId + "/splitterState", m_splitter->saveState()).toByteArray());

	m_infoTable->setColumnWidth(0, settings.value("TuningSourceWidget/" + equipmentId + "/infoColumnWidth", m_infoTable->columnWidth(0)).toInt());
	m_stateTable->setColumnWidth(0, settings.value("TuningSourceWidget/" + equipmentId + "/stateColumnWidth", m_stateTable->columnWidth(0)).toInt());
}

TuningSourceWidget::~TuningSourceWidget()
{
	emit forgetMe();
}

void TuningSourceWidget::updateStateFields()
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	const Network::TuningSourceState* pState = nullptr;

	for (auto& source : m_tcpClientSocket->tuningSources())
	{
		if (source.id() == m_id && source.equipmentId() == m_equipmentId)
		{
			pState = &source.state;
		}
	}

	if (pState == nullptr)
	{
		// Lost widgets tuning source ?
		close();
		deleteLater();
		return;
	}

	for (int i = 0; i < dynamicPropertiesFieldList.count(); i++)
	{
		auto& field = dynamicPropertiesFieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 1), field.fieldValueGetter(*pState));
	}
}

void TuningSourceWidget::setClientSocket(TcpTuningServiceClient *tcpClientSocket)
{
	TEST_PTR_RETURN(tcpClientSocket);

	m_tcpClientSocket = tcpClientSocket;

	connect(tcpClientSocket, &TcpTuningServiceClient::tuningSoursesStateUpdated, this, &TuningSourceWidget::updateStateFields);
	connect(tcpClientSocket, &TcpTuningServiceClient::disconnected, this, &TuningSourceWidget::unsetClientSocket);

	const Network::DataSourceInfo* pInfo = nullptr;

	for (auto& source : m_tcpClientSocket->tuningSources())
	{
		if (source.id() == m_id && source.equipmentId() == m_equipmentId)
		{
			pInfo = &source.info;
		}
	}

	if (pInfo == nullptr)
	{
		// Lost widgets tuning source ?
		close();
		deleteLater();
		return;
	}

	for (int i = 0; i < staticPropertiesFieldList.count(); i++)
	{
		auto& field = staticPropertiesFieldList[i];

		m_infoModel->setData(m_infoModel->index(i, 1), field.fieldValueGetter(*pInfo));
	}
}

void TuningSourceWidget::unsetClientSocket()
{
	m_tcpClientSocket = nullptr;
}

void TuningSourceWidget::closeEvent(QCloseEvent *event)
{
	QSettings settings;
	settings.setValue("TuningSourceWidget/" + m_equipmentId + "/geometry", geometry());
	settings.setValue("TuningSourceWidget/" + m_equipmentId + "/splitterState", m_splitter->saveState());

	settings.setValue("TuningSourceWidget/" + m_equipmentId + "/infoColumnWidth", m_infoTable->columnWidth(0));
	settings.setValue("TuningSourceWidget/" + m_equipmentId + "/stateColumnWidth", m_stateTable->columnWidth(0));

	QWidget::closeEvent(event);
}

void TuningSourceWidget::initTable(QTableView *table, QStandardItemModel *model)
{
	table->verticalHeader()->setDefaultSectionSize(static_cast<int>(table->fontMetrics().height() * 1.4));
	table->verticalHeader()->hide();

	table->horizontalHeader()->setStretchLastSection(true);
	table->horizontalHeader()->setHighlightSections(false);

	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setAlternatingRowColors(true);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);

	model->setHeaderData(0, Qt::Horizontal, "Property");
	model->setHeaderData(1, Qt::Horizontal, "Value");

	table->setColumnWidth(0, 200);

	table->setModel(model);

	m_splitter->addWidget(table);
}
