#include "SimCodePage.h"
#include <QGridLayout>
#include <QMenu>
#include <QFontDatabase>

//
// SimCodeModel
//
SimCodeModel::SimCodeModel(SimIdeSimulator* simulator,
						   QString lmEquipmentId,
						   QObject* parent) :
	QAbstractTableModel(parent),
	m_simulator(simulator),
	m_lmEquipmentId(lmEquipmentId)
{
	assert(simulator);
	assert(lmEquipmentId.isEmpty() == false);

	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimCodeModel::dataChanged);

	dataChanged();	// First init

	return;
}

int SimCodeModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(m_commands.size());
}

int SimCodeModel::columnCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(CodePageColumns::ColumnCount);
}

QVariant SimCodeModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
	if (role == Qt::DisplayRole)
	{
		switch (section)
		{
		case CodePageColumns::Row:		return tr("Row");
		case CodePageColumns::Address:	return tr("Address");
		case CodePageColumns::Code:		return tr("Code");
		default:
			assert(false);
			return QVariant();
		}
	}

	return QVariant();
}

QVariant SimCodeModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	int row = index.row();
	int column = index.column();

	if (row > m_commands.size())
	{
		assert(row < m_commands.size());
		return QVariant();
	}

	const Sim::DeviceCommand& command = m_commands[row];

	// Color
	//
	if (role == Qt::BackgroundRole)
	{
		switch (column)
		{
		case CodePageColumns::Row:
				if (row == 20)
					return QBrush(Qt::darkRed);
				else
					return QBrush(qRgb(0xF0, 0xF0, 0xF0));
		case CodePageColumns::Address:		return QBrush(qRgb(0xF0, 0xF0, 0xF0));
		case CodePageColumns::Code:			return QBrush();
		default:
			assert(false);
			return QVariant();
		}
	}

	if (role == Qt::ForegroundRole)
	{
		switch (column)
		{
		//case CodePageColumns::Row:			return QBrush(Qt::darkGray);
		case CodePageColumns::Row:
			if (row == 20)
				return QBrush(Qt::white);
			else
				return QBrush(Qt::darkGray);

		case CodePageColumns::Address:		return QBrush(Qt::darkGray);
		case CodePageColumns::Code:			return QVariant();
		default:
			assert(false);
			return QVariant();
		}
	}

	// Text
	//
	if (role == Qt::DisplayRole)
	{
		switch (column)
		{
		case CodePageColumns::Row:		return QString(" %1 ").arg(row);
		case CodePageColumns::Address:	return QString(" 0x%1 ").arg(command.m_offset, 4, 16, QChar('0'));
		case CodePageColumns::Code:		return QString(" %1").arg(command.m_string);
		default:
			assert(false);
			return QVariant();
		}
	}

	// Aligment
	//
	if (role == Qt::TextAlignmentRole)
	{
		switch (column)
		{
		case CodePageColumns::Row:		return  QVariant(Qt::AlignRight);
		case CodePageColumns::Address:	return  QVariant(Qt::AlignRight);
		case CodePageColumns::Code:		return  QVariant(Qt::AlignLeft);
		default:
			assert(false);
			return QVariant();
		}
	}

	return QVariant();
}

void SimCodeModel::dataChanged()
{
	auto lm = logicModule();
	if (lm == nullptr)
	{
		beginResetModel();

		// It can be reloading rigt now
		//
		m_commands.clear();
		m_offsetToCommand.clear();

		endResetModel();
		return;
	}

	{
		beginResetModel();

		m_commands = lm->appCommands();
		m_offsetToCommand = lm->offsetToCommand();

		endResetModel();
	}
	return;
}

std::shared_ptr<Sim::LogicModule> SimCodeModel::logicModule()
{
	return m_simulator->logicModule(m_lmEquipmentId);
}

std::shared_ptr<Sim::LogicModule> SimCodeModel::logicModule() const
{
	return m_simulator->logicModule(m_lmEquipmentId);
}

//
// SimCodeView
//
SimCodeView::SimCodeView(QWidget* parent) :
	QTreeView(parent)
{
	setHeaderHidden(true);
	setItemsExpandable(false);	// It's a list
	setRootIsDecorated(false);
	setSortingEnabled(false);
	setUniformRowHeights(true);
	setWordWrap(false);

	QFont fixedFont("Consolas");
	fixedFont.setStyleHint(QFont::TypeWriter);
	fixedFont.setPointSizeF(fixedFont.pointSizeF());	// Makes font bigger
	setFont(fixedFont);

	header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	return;
}

SimCodeView::~SimCodeView()
{
}

//
// SimCodePage
//
SimCodePage::SimCodePage(SimIdeSimulator* simulator,
						 QString lmEquipmentId,
						 QWidget* parent)
	: SimBasePage(simulator, parent),
	m_lmEquipmentId(lmEquipmentId)
{
	assert(m_simulator);
	assert(lmEquipmentId.isEmpty() == false);

	m_model = new SimCodeModel(m_simulator, m_lmEquipmentId, this);

	m_view = new SimCodeView(this);
	m_view->setModel(m_model);

	setLayout(new QBoxLayout(QBoxLayout::TopToBottom));

	layout()->addWidget(m_view);

	return;
}

QString SimCodePage::equipmnetId() const
{
	return m_lmEquipmentId;
}

std::shared_ptr<Sim::LogicModule> SimCodePage::logicModule()
{
	return m_simulator->logicModule(m_lmEquipmentId);
}

std::shared_ptr<Sim::LogicModule> SimCodePage::logicModule() const
{
	return m_simulator->logicModule(m_lmEquipmentId);
}
