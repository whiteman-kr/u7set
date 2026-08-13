#include "SignalsView.h"

#include <Rvdu/rn_sifp.h>
#include <RvduSimulatorLib.h>

#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QTimer>
#include <QSettings>

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


SignalsView::SignalsView(QWidget* parent)
	: QWidget(parent)
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

	if (QSettings().value("SignalsView/tableColumnsCount", 0) != m_tableHeaders.size()) 
	{
		m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	}
	else	
	{
		ba = QSettings().value("SignalsView/tableState").toByteArray();
		if (ba.isEmpty() == false)
		{
			m_tableWidget->horizontalHeader()->restoreState(ba);
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


	QSettings().setValue("SignalsView/tableColumnsCount", m_tableHeaders.size());
	QSettings().setValue("SignalsView/tableState", m_tableWidget->horizontalHeader()->saveState());

	m_instance = nullptr;
}

void SignalsView::setupUi()
{
	m_tableWidget = new QTableWidget(this);

	m_tableHeaders << tr("Index") << tr("AppSignalID") << tr("Caption") << tr("Type") << tr("I/O Type") << tr("Units") << tr("Address")
				   << tr("Tunable") << tr("OptoPort") << tr("Value");

	m_tableWidget->setColumnCount(m_tableHeaders.size());
	m_tableWidget->setHorizontalHeaderLabels(m_tableHeaders);
	m_tableWidget->verticalHeader()->setVisible(false);

	m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_tableWidget);

	connect(m_tableWidget,
			&QTableWidget::itemDoubleClicked,
			this,
			[this](QTableWidgetItem* /*item*/)
			{
				auto rows = m_tableWidget->selectionModel()->selectedRows();
				if (rows.isEmpty() == true)
				{
					return;
				}
				int row = rows.first().row();

				onSignalClicked(static_cast<unsigned int>(row));
			});
}

static QTableWidgetItem* createTableItem(const QString& text)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	return item;
}

void SignalsView::fillSignalsList()
{
	m_tableWidget->clearContents();

	m_tableWidget->setRowCount(static_cast<int>(rnLibAppSignalsCount()));

	for (alt_u16 i = 0; i < rnLibAppSignalsCount(); i++)
	{
		const struct VduAppSignalParam* const param = rnLibAppSignalParam(i);
		if (param == NULL)
		{
			return;
		}
		int c = 0;
		
		{
			m_tableWidget->setItem(i, c++, createTableItem(QString("%1").arg(i, 4, 10, QChar('0'))));
		}

		{
			const char* const appSignalId = rnLibAppSignalCustomID(i);
			m_tableWidget->setItem(i, c++, createTableItem(QString::fromUtf8(appSignalId)));
		}

		{
			const char* const caption = rnLibAppSignalCaption(i);
			m_tableWidget->setItem(i, c++, createTableItem(QString::fromUtf8(caption)));
		}

		{
			QString vduSignalType;
			switch (param->vduSignalType)
			{
			case Discrete:
				{
					vduSignalType = "D";
				}
				break;
			case AnalogSignedInt32:
				{
					vduSignalType = "SI32";
				}
				break;
			case AnalogFloat32:
				{
					vduSignalType = "FP32";
				}
				break;
			default:
				{
					;
				}
			}
			m_tableWidget->setItem(i, c++, createTableItem(vduSignalType));
		}

		{
			QString vduSignalInOutType;
			switch (param->vduSignalInOutType)
			{
			case Internal:
				{
					vduSignalInOutType = "Internal";
				}
				break;
			case Input:
				{
					vduSignalInOutType = "Input";
				}
				break;
			case Output:
				{
					vduSignalInOutType = "Output";
				}
				break;
			case RxSignal:
				{
					vduSignalInOutType = "RX";
				}
				break;
			default:
				{
					;
				}
			}

			for (alt_u16 j = 0; j < rnLibTxAppSignalsCount(); j++)
			{
				const struct VduTxRxAppSignal* const txRxSignal = rnLibTxAppSignal(j);
				if (txRxSignal == NULL)
				{
					assert(txRxSignal);
					return;
				}

				if (txRxSignal->signalIndex == i)
				{
					vduSignalInOutType += ", TX";
					break;
				}
			}
			m_tableWidget->setItem(i, c++, createTableItem(vduSignalInOutType));
		}

		{
			const char* const unit = rnLibAppSignalUnit(i);
			m_tableWidget->setItem(i, c++, createTableItem(QString::fromUtf8(unit)));
		}

		{
			QString address;
			if (param->vduSignalInOutType == Internal)
			{
				address = "";
			}
			else
			{
				address = tr("%1:%2").arg(param->optoAddr.offset).arg(param->optoAddr.bitNo);
			}
			m_tableWidget->setItem(i, c++, createTableItem(address));
		}

		{
			m_tableWidget->setItem(i, c++, createTableItem(param->props.bits.tunable ? tr("Tunable") : ""));
		}

		{
			QString optoPort;
			if (param->vduSignalInOutType == RxSignal)
			{
				optoPort = tr("%1").arg(param->rxPortIndex);
			}
			else
			{
				optoPort = "";
			}
			m_tableWidget->setItem(i, c++, createTableItem(optoPort));
		}

		// Value
		m_tableWidget->setItem(i, c++, createTableItem(""));
	}

	updateSignalValues();
}

void SignalsView::updateSignalValues()
{
	alt_u16 count = m_tableWidget->rowCount();

	if (count != rnLibAppSignalsCount() || count == 0) 
	{
		return;
	}
		
	for (alt_u16 i = 0; i < count; i++)
	{
		const struct VduAppSignalParam* const param = rnLibAppSignalParam(i);
		if (param == NULL)
		{
			return;
		}

		int iRes = 0;
		float fRes = 0;
		QString value;
		switch (param->vduSignalType)
		{
		case Discrete:
			{
				rnLibSignalValueInt(i, &iRes);
				value = tr("%1").arg(iRes);
			}
			break;
		case AnalogSignedInt32:
			{
				rnLibSignalValueInt(i, &iRes);
				value = tr("%1").arg(iRes);
			}
			break;
		case AnalogFloat32:
			{
				rnLibSignalValueFloat(i, &fRes);
				value = tr("%1").arg(fRes, 0, 'f', 3);
			}
			break;
		default:
			{
				;
			}
		}

		QTableWidgetItem* item = m_tableWidget->item(i, m_tableHeaders.size() - 1);
		if (item == nullptr) 
		{
			Q_ASSERT(item);
			continue;
		}
		item->setText(value);

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

	alt_32 iRes = 0;
	float fRes = 0;

	QString label;
	QString textCaption;
	QString value;
	switch (param->vduSignalType)
	{
	case Discrete:
		{
			rnLibSignalValueInt(index, &iRes);
			value = tr("%1").arg(iRes);
			label = tr("%1").arg(appSignalId);
			textCaption = tr("%1 [0..1]").arg(caption);
		}
		break;
	case AnalogSignedInt32:
		{
			rnLibSignalValueInt(index, &iRes);
			value = tr("%1").arg(iRes);
			label = tr("%1").arg(appSignalId);
			textCaption = tr("%1 [%2..%3]").arg(caption).arg(param->lowLimit.intValue).arg(param->highLimit.intValue);
		}
		break;
	case AnalogFloat32:
		{
			rnLibSignalValueFloat(index, &fRes);
			value = tr("%1").arg(fRes, 0, 'f', 3);
			label = tr("%1").arg(appSignalId);
			textCaption = tr("%1 [%2..%3]").arg(caption).arg(param->lowLimit.floatValue).arg(param->highLimit.floatValue);
		}
		break;
	default:
		{
			;
		}
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
	if (param->vduSignalType == Discrete || param->vduSignalType == AnalogSignedInt32)
	{
		int newValue = text.toInt(&correct);
		if (correct == true)
		{
			(void)rnLibWriteSignalValueInt(index, newValue);
		}
	}
	else if (param->vduSignalType == AnalogFloat32)
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