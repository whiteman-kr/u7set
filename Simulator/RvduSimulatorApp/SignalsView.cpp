#include "SignalsView.h"

#include <RvduSimulatorLib.h>

#include <QTableView>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QClipboard>

SignalsModel::SignalsModel()
{
	m_tableHeaders << tr("Index") << tr("AppSignalID") << tr("Caption") << tr("Type") << tr("I/O Type") << tr("Units") << tr("Address")
				   << tr("Tunable") << tr("OptoPort") << tr("Value");
}

void SignalsModel::setSignalsCount(int count)
{
	m_signalsCount = count;

	beginInsertRows(QModelIndex(), 0, static_cast<int>(m_signalsCount) - 1);

	insertRows(0, static_cast<int>(m_signalsCount));

	endInsertRows();
}

int SignalsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m_signalsCount;
}

int SignalsModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m_tableHeaders.size();
}

// Returns cell content based on the view request role
QVariant SignalsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	int i = index.row();

	const struct VduAppSignalParam* const param = rnLibAppSignalParam(i);
	if (param == NULL)
	{
		Q_ASSERT(param);
		return QVariant();
	}

	switch (index.column())
	{
	case 0:
		{
			return QString("%1").arg(i, 4, 10, QChar('0'));
		}
	case 1:
		{
			const char* const appSignalId = rnLibAppSignalCustomID(i);
			return QString::fromUtf8(appSignalId);
		}
	case 2:
		{
			const char* const caption = rnLibAppSignalCaption(i);
			return QString::fromUtf8(caption);
		}
	case 3:
		{
			QString vduSignalType;
			if (rnLibIsDiscrete(i) == true)
			{
				vduSignalType = "D";
			}
			else if (rnLibIsFloat(i) == true)
			{
				vduSignalType = "FP32";
			}
			else if (rnLibIsInt32(i) == true)
			{
				vduSignalType = "SI32";
			}

			return vduSignalType;
		}

	case 4:
		{
			QString vduSignalInOutType;

			if (rnLibIsInternal(i) == true)
			{
				vduSignalInOutType = "Internal";
			}
			else if (rnLibIsInput(i) == true)
			{
				vduSignalInOutType = "Input";
			}
			else if (rnLibIsOutput(i) == true)
			{
				vduSignalInOutType = "Output";
			}
			else if (rnLibIsRxSignal(i) == true)
			{
				vduSignalInOutType = "RX";
			}

			if (rnLibIsTxSignal(i) == true)
			{
				vduSignalInOutType += ", TX";
			}

			return vduSignalInOutType;
		}

	case 5:
		{
			const char* const unit = rnLibAppSignalUnit(i);
			return QString::fromUtf8(unit);
		}

	case 6:
		{
			QString address;
			if (rnLibIsInternal(i) == false)
			{
				address = tr("RX %1:%2").arg(rnLibSignalOffset(i)).arg(rnLibSignalBit(i));

			}

			if (rnLibIsTxSignal(i) == true)
			{
				address += tr(" TX %1:%2").arg(rnLibTxSignalOffset(i)).arg(rnLibTxSignalBit(i));
			}

			return address.trimmed();
		}

	case 7:
		{
			return (rnLibSignalTunable(i) ? tr("Tunable") : "");
		}

	case 8:
		{
			QString optoPort;
			if (rnLibIsRxSignal(i) == true)
			{
				optoPort = tr("RX %1").arg(rnLibRxPortIndex(i));
			}

			if (rnLibIsTxSignal(i) == true)
			{
				optoPort = tr(" TX %1").arg(rnLibTxPortIndex(i));
			}

			return optoPort.trimmed();
		}

	// Value
	case 9:
		{
			int iRes = 0;
			float fRes = 0;
			QString value;

			if (rnLibIsDiscrete(i) == true)
			{
				rnLibSignalValueInt(i, &iRes);
				value = tr("%1").arg(iRes);
			}
			else if (rnLibIsFloat(i) == true)
			{
				rnLibSignalValueFloat(i, &fRes);
				value = tr("%1").arg(fRes, 0, 'f', 3);
			}
			else if (rnLibIsInt32(i) == true)
			{
				rnLibSignalValueInt(i, &iRes);
				value = tr("%1").arg(iRes);
			}

			return value;
		}
	}

	return QVariant();
}

// Returns column header names
QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section < m_tableHeaders.size())
			return m_tableHeaders.at(section);
	}
	return QVariant();
}


SignalsView* SignalsView::create(QWidget* parent)
{
	if (m_instance == nullptr)
	{
		m_instance = new SignalsView(parent);
		m_instance->setAttribute(Qt::WA_DeleteOnClose);
		m_instance->setWindowFlag(Qt::Window);
		m_instance->show();
	}
	return m_instance;
}


SignalsView::SignalsView(QWidget* parent) :
	QWidget(parent)
{
	setWindowTitle(tr("Signals View"));
	setMinimumSize(1280, 720);

	setupUi();

	fillSignalsList();

	QPoint p = QSettings().value("SignalsView/pos", QPoint(-1, -1)).toPoint();
	if (p.x() != -1 && p.y() != -1)
	{
		move(p);
	}
	QByteArray ba = QSettings().value("SignalsView/geometry").toByteArray();
	if (ba.isEmpty() == false)
	{
		restoreGeometry(ba);
	}

	if (QSettings().value("SignalsView/tableColumnsCount", 0) != m_model.columnCount())
	{
		m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	}
	else
	{
		ba = QSettings().value("SignalsView/tableState").toByteArray();
		if (ba.isEmpty() == false)
		{
			m_tableView->horizontalHeader()->restoreState(ba);
		}
	}

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &SignalsView::onTimer);
	timer->start(250);
}

SignalsView::~SignalsView()
{
	QSettings().setValue("SignalsView/pos", pos());
	QSettings().setValue("SignalsView/geometry", saveGeometry());


	QSettings().setValue("SignalsView/tableColumnsCount", m_model.columnCount());
	QSettings().setValue("SignalsView/tableState", m_tableView->horizontalHeader()->saveState());

	m_instance = nullptr;
}

void SignalsView::setupUi()
{
	m_tableView = new QTableView(this);
	m_tableView->setModel(&m_model);

	m_tableView->verticalHeader()->setVisible(false);

	m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_tableView);

	
	connect(m_tableView,
			&QTableView::doubleClicked,
			this,
			[this](QModelIndex index)
			{
				if (!index.isValid())
				{
					return;
				}

				int row = index.row();
				if (row >= static_cast<int>(rnLibAppSignalsCount()))
				{
					Q_ASSERT(false);
					return;
				}

				onSignalClicked(static_cast<unsigned int>(row));
			});

	connect(m_tableView,
			&QTableView::customContextMenuRequested,
			this,
			[this](const QPoint& pos)
			{
				QModelIndex index = m_tableView->indexAt(pos);
				if (index.isValid() == false)
				{
					return;
				}

				const int row = index.row();

				QMenu menu(m_tableView);
				QModelIndex modelIndex = m_tableView->model()->index(row, 1 /*AppSignalCustomID*/);
				QString value = modelIndex.data(Qt::DisplayRole).toString();

				QAction* action = menu.addAction(QString("Copy %1").arg(value));

				connect(action,
						&QAction::triggered,
						this,
						[value]()
						{
							QClipboard* clipboard = QApplication::clipboard();
							clipboard->setText(value);
						});
				menu.exec(m_tableView->viewport()->mapToGlobal(pos));
			});
}

void SignalsView::fillSignalsList()
{ m_model.setSignalsCount(rnLibAppSignalsCount()); }

void SignalsView::updateSignalValues()
{
	int from = m_tableView->rowAt(0);

	int to = m_tableView->rowAt(m_tableView->height() - m_tableView->horizontalHeader()->height());

	if (from == -1)
	{
		from = 0;
	}

	if (to == -1)
	{
		to = m_model.rowCount() - 1;
	}

	// Redraw visible table items
	//
	int col = m_model.columnCount() - 1;
	for (int row = from; row <= to; row++)
	{
		m_tableView->update(m_model.index(row, col));
	}
}

void SignalsView::onSignalClicked(unsigned int index)
{
	const struct VduAppSignalParam* const param = rnLibAppSignalParam(index);
	if (param == NULL)
	{
		return;
	}

	QString appSignalId = QString::fromUtf8(rnLibAppSignalCustomID(index));
	QString caption = QString::fromUtf8(rnLibAppSignalCaption(index));

	int iRes = 0;
	float fRes = 0;

	QString label;
	QString textCaption;
	QString value;

	if (rnLibIsDiscrete(index) == true)
	{
		rnLibSignalValueInt(index, &iRes);
		value = tr("%1").arg(iRes);
		label = tr("%1").arg(appSignalId);
		textCaption = tr("%1 [0..1]").arg(caption);
	}
	else if (rnLibIsFloat(index) == true)
	{
		rnLibSignalValueFloat(index, &fRes);
		value = tr("%1").arg(fRes, 0, 'f', 3);
		label = tr("%1").arg(appSignalId);
		textCaption = tr("%1 [%2..%3]").arg(caption).arg(rnLibSignalLowLimitFloat(index)).arg(rnLibSignalHighLimitFloat(index));
	}
	else if (rnLibIsInt32(index) == true)
	{
		rnLibSignalValueInt(index, &iRes);
		value = tr("%1").arg(iRes);
		label = tr("%1").arg(appSignalId);
		textCaption = tr("%1 [%2..%3]").arg(caption).arg(rnLibSignalLowLimitInt(index)).arg(rnLibSignalHighLimitInt(index));
	}

	QInputDialog dialog(this);

	dialog.setWindowTitle(label);
	dialog.setLabelText(textCaption + "\nEnter value:");
	dialog.setTextValue(value);

	dialog.resize(500, 200);
	int result = dialog.exec();

	if (result != QDialog::Accepted)
	{
		return;
	}

	QString text = dialog.textValue();

	bool correct = false;
	if (rnLibIsDiscrete(index) == true || rnLibIsInt32(index) == true)
	{
		int newValue = text.toInt(&correct);
		if (correct == true)
		{
			(void)rnLibWriteSignalValueInt(index, newValue);
		}
	}
	else if (rnLibIsFloat(index) == true)
	{
		float newValue = text.toFloat(&correct);
		if (correct == true)
		{
			(void)rnLibWriteSignalValueFloat(index, newValue);
		}
	}

	if (correct == false)
	{
		QMessageBox::critical(this, tr("RVDU"), tr("Invalid number format!"));
	}
}

void SignalsView::onTimer()
{ 
	updateSignalValues(); 
}