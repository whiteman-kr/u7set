#include "TuningSourceWidget.h"
#include "../Proto/network.pb.h"
#include <functional>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include "TcpTuningServiceClient.h"
#include <QHBoxLayout>
#include "../lib/WidgetUtils.h"

struct fieldDefinition
{
	QString fieldName;
	std::function<QString(const Network::TuningSourceState& state)> fieldValueGetter;
};

static const QList<fieldDefinition> fieldList {
	{ QStringLiteral("SourceID"), [](const Network::TuningSourceState& state) { return QString::number(state.sourceid()); } },

	{ QStringLiteral("IsReply"), [](const Network::TuningSourceState& state) { return state.isreply() ? "Yes" : "No"; } },

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

	{ QStringLiteral("ControlIsActive"), [](const Network::TuningSourceState& state) { return state.controlisactive() ? "Yes" : "No"; } },
	{ QStringLiteral("SetSOR"), [](const Network::TuningSourceState& state) { return state.setsor() ? "Yes" : "No"; } },

	{ QStringLiteral("HasUnappliedParams"), [](const Network::TuningSourceState& state) { return state.hasunappliedparams() ? "Yes" : "No"; } },
};

TuningSourceWidget::TuningSourceWidget(quint64 id, QString equipmentId, QWidget *parent) :
	QWidget(parent),
	m_id(id),
	m_equipmentId(equipmentId)
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_stateTable = new QTableView(this);

	m_stateTable->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_stateTable->fontMetrics().height() * 1.4));
	m_stateTable->verticalHeader()->hide();

	m_stateTable->horizontalHeader()->setDefaultSectionSize(200);
	m_stateTable->horizontalHeader()->setStretchLastSection(true);
	m_stateTable->horizontalHeader()->setHighlightSections(false);

	m_stateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_stateTable->setSelectionMode(QAbstractItemView::SingleSelection);
	m_stateTable->setAlternatingRowColors(true);
	m_stateTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_stateModel = new QStandardItemModel(fieldList.count(), 2, this);

	m_stateModel->setHeaderData(0, Qt::Horizontal, "Property");
	m_stateModel->setHeaderData(1, Qt::Horizontal, "Value");

	m_stateTable->setModel(m_stateModel);

	for (int i = 0; i < fieldList.count(); i++)
	{
		auto& field = fieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 0), field.fieldName);
	}

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(m_stateTable);
	setLayout(hl);

	setWindowTitle(equipmentId);

	setWindowPosition(this, "TuningSourceWidget/" + equipmentId + "/geometry");
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
	}

	for (int i = 0; i < fieldList.count(); i++)
	{
		auto& field = fieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 1), field.fieldValueGetter(*pState));
	}
}

void TuningSourceWidget::setClientSocket(TcpTuningServiceClient *tcpClientSocket)
{
	m_tcpClientSocket = tcpClientSocket;

	connect(tcpClientSocket, &TcpTuningServiceClient::tuningSoursesStateUpdated, this, &TuningSourceWidget::updateStateFields);
	connect(tcpClientSocket, &TcpTuningServiceClient::disconnected, this, &TuningSourceWidget::unsetClientSocket);
}

void TuningSourceWidget::unsetClientSocket()
{
	m_tcpClientSocket = nullptr;
}

void TuningSourceWidget::closeEvent(QCloseEvent *event)
{
	QSettings settings;
	settings.setValue("TuningSourceWidget/" + m_equipmentId + "/geometry", geometry());

	QWidget::closeEvent(event);
}
