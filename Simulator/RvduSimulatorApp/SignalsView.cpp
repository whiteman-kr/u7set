#include "SignalsView.h"

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

	for (unsigned int i = 0; i < rnLibAppSignalsCount(); i++)
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

			m_tableWidget->setItem(i, c++, createTableItem(vduSignalType));
		}

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

			m_tableWidget->setItem(i, c++, createTableItem(vduSignalInOutType));
		}

		{
			const char* const unit = rnLibAppSignalUnit(i);
			m_tableWidget->setItem(i, c++, createTableItem(QString::fromUtf8(unit)));
		}

		{
			QString address;
			if (rnLibIsInternal(i) == true)
			{
				address = "";
			}
			else
			{
				address = tr("%1:%2").arg(rnLibSignalOffset(i)).arg(rnLibSignalBit(i));
			}
			m_tableWidget->setItem(i, c++, createTableItem(address));
		}

		{
			m_tableWidget->setItem(i, c++, createTableItem(rnLibSignalTunable(i) ? tr("Tunable") : ""));
		}

		{
			QString optoPort;
			if (rnLibIsRxSignal(i) == true)
			{
				optoPort = tr("%1").arg(rnLibRxPortIndex(i));
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
	int count = m_tableWidget->rowCount();

	if (count != rnLibAppSignalsCount() || count == 0) 
	{
		return;
	}
		
	for (int i = 0; i < count; i++)
	{
		const struct VduAppSignalParam* const param = rnLibAppSignalParam(i);
		if (param == NULL)
		{
			return;
		}

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
		textCaption = tr("%1 [%2..%3]").arg(caption)
			.arg(rnLibSignalLowLimitFloat(index)).arg(rnLibSignalHighLimitFloat(index));
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