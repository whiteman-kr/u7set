#include "TuningSourceWidget.h"
#include "TcpTuningServiceClient.h"
#include "../UtilsLib/Ui/WidgetUtils.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>

struct staticPropertyFieldDefinition
{
	QString fieldName;
	std::function<QString(const Network::DataSourceInfo& info, int channel)> fieldValueGetter;
};

struct dynamicPropertyFieldDefinition
{
	QString fieldName;
	std::function<QString(const Network::TuningSourceState& state)> fieldValueGetter;
};

static const QList<staticPropertyFieldDefinition> staticPropertiesFieldList {
	{ QStringLiteral("EquipmentID"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::fromStdString(info.moduleequipmentid()); } },
	{ QStringLiteral("Caption"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::fromStdString(info.modulecaption()); } },
	{ QStringLiteral("DataType"), [](const Network::DataSourceInfo& info, int channel) { return E::valueToString(static_cast<E::LanControllerType>(info.lancontrollerinfo(channel).lancontrollertype())); } },
	{ QStringLiteral("IP"), [](const Network::DataSourceInfo& info, int channel) { return QString::fromStdString(info.lancontrollerinfo(channel).tuningip()); } },
	{ QStringLiteral("Port"), [](const Network::DataSourceInfo& info, int channel) { return QString::number(info.lancontrollerinfo(channel).tuningport()); } },
	{ QStringLiteral("Channel"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::fromStdString(info.subsystemchannel()); } },
	{ QStringLiteral("SubsystemKey"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::number(info.subsystemkey()); } },
	{ QStringLiteral("SubsystemID"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::fromStdString(info.subsystemid()); } },

	{ QStringLiteral("LmNumber"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::number(info.lmnumber()); } },
	{ QStringLiteral("ModuleType"), [](const Network::DataSourceInfo& info, int /*channel*/) { return QString::number(info.moduletype()); } },
	{ QStringLiteral("AdapterID"), [](const Network::DataSourceInfo& info, int channel) { return QString::fromStdString(info.lancontrollerinfo(channel).equipmentid()); } },
	{ QStringLiteral("TuningEnable"), [](const Network::DataSourceInfo& info, int channel) { return info.lancontrollerinfo(channel).tuningenable() ? "Yes" : "No"; } },

	{ QStringLiteral("SourceID"), [](const Network::DataSourceInfo& info, int /*channel*/) { return "0x" + QString("%1").arg(info.id(), sizeof(info.id()) * 2, 16, QChar('0')).toUpper(); } },
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

	{ QStringLiteral("ErrTuningFrameUpdate"), [](const Network::TuningSourceState& state) { return QString::number(state.errtuningframeupdate()); } },
};

TuningSourceWidget::TuningSourceWidget(quint64 id, QString equipmentId, QString controllerEquipmentId, int controllerIndex, QWidget *parent) :
	QWidget(parent),
	m_id(id),
	m_equipmentId(equipmentId),
	m_controllerEquipmentId(controllerEquipmentId),
	m_controllerIndex(controllerIndex)
{
	setWindowFlag(Qt::Dialog, true);

	setAttribute(Qt::WA_DeleteOnClose);
	QHBoxLayout* hl = new QHBoxLayout();
	m_splitter = new QSplitter(this);
	hl->addWidget(m_splitter);
	setLayout(hl);

	// Source info
	//
	m_infoTable = new QTableView(this);
	m_infoModel = new QStandardItemModel(static_cast<int>(staticPropertiesFieldList.count()), 2, this);

	for (int i = 0; i < staticPropertiesFieldList.count(); i++)
	{
		auto& field = staticPropertiesFieldList[i];

		m_infoModel->setData(m_infoModel->index(i, 0), field.fieldName);
	}

	initTable(m_infoTable, m_infoModel);

	// Source state
	//
	m_stateTable = new QTableView(this);
	m_stateModel = new QStandardItemModel(static_cast<int>(dynamicPropertiesFieldList.count()), 2, this);

	for (int i = 0; i < dynamicPropertiesFieldList.count(); i++)
	{
		auto& field = dynamicPropertiesFieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 0), field.fieldName);
	}

	initTable(m_stateTable, m_stateModel);

	setWindowTitle(equipmentId);

	setWindowPosition(this, "TuningSourceWidget/" + m_equipmentId);

	QSettings settings;
	m_splitter->restoreState(settings.value("TuningSourceWidget/" + m_equipmentId + "/splitterState", m_splitter->saveState()).toByteArray());

	m_infoTable->setColumnWidth(0, settings.value("TuningSourceWidget/" + m_equipmentId + "/infoColumnWidth", m_infoTable->columnWidth(0)).toInt());
	m_stateTable->setColumnWidth(0, settings.value("TuningSourceWidget/" + m_equipmentId + "/stateColumnWidth", m_stateTable->columnWidth(0)).toInt());
}

TuningSourceWidget::~TuningSourceWidget()
{
	emit forgetMe();
}

void TuningSourceWidget::updateStateFields()
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	const Network::TuningSourceState* pState = nullptr;

	for (const auto& ts : m_tcpClientSocket->tuningSources())
	{
		if (ts.id() == id() && ts.equipmentId() == equipmentId())
		{
			for (int i = 0; i < ts.statesCount(); i++)
			{
				if (QString::fromStdString(ts.state(i).lanequipmentid()) == controllerEquipmentId())
				{
					pState = &ts.state(i);
					break;
				}
			}
		}
		if (pState != nullptr)
		{
			break;
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

	const Network::DataSourceInfo* pInfo = nullptr;

	for (const auto& ts : m_tcpClientSocket->tuningSources())
	{
		if (ts.id() == id() && ts.equipmentId() == equipmentId())
		{
			pInfo = &ts.info();
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

		m_infoModel->setData(m_infoModel->index(i, 1), field.fieldValueGetter(*pInfo, m_controllerIndex));
	}
}

void TuningSourceWidget::unsetClientSocket()
{
	m_tcpClientSocket = nullptr;
}

void TuningSourceWidget::closeEvent(QCloseEvent *event)
{
	saveWindowPosition(this, "TuningSourceWidget/" + m_equipmentId);

	QSettings settings;
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
