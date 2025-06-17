#include "RWMultyToolBox.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <random>


RWMultyToolBox::RWMultyToolBox(QWidget* parent) :
	QWidget(parent)
{
	tableWidget = new QTableWidget(32, 2, this); // 2 columns: Signal ID, Value
	tableWidget->setHorizontalHeaderLabels({"Signal ID", "Value"});
	tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	for (int row = 0; row < tableWidget->rowCount(); ++row)
	{
		tableWidget->setItem(row, 0, new QTableWidgetItem(""));
		tableWidget->setItem(row, 1, new QTableWidgetItem(""));
	}

	// Create Read and Write buttons
	QPushButton* readBtn = new QPushButton("Read All", this);
	QPushButton* writeBtn = new QPushButton("Write All", this);
	QPushButton* autoFillBtn = new QPushButton("Auto Fill", this);

	connect(readBtn,
			&QPushButton::clicked,
			this,
			[this]()
			{
				QString* idList = new QString;
				for (int row = 0; row < filledRowCount(); ++row)
				{
					auto* idItem = tableWidget->item(row, 0);
					if (idItem && !idItem->text().isEmpty())
					{
						idList->append(idItem->text() + "|");
					}
				}
				if (!idList->isEmpty() && idList->endsWith('|'))
					idList->chop(1);
				emit requestRead(*idList);
				return;
			});

	connect(writeBtn,
			&QPushButton::clicked,
			this,
			[this]()
			{
				QString* idList = new QString;
				QString* valueList = new QString;
				for (int row = 0; row < filledRowCount(); ++row)
				{
					auto* idItem = tableWidget->item(row, 0);
					auto* valueItem = tableWidget->item(row, 1);
					if (idItem && valueItem && !idItem->text().isEmpty())
					{
						idList->append(idItem->text() + "|");
						valueList->append(valueItem->text() + "|");
					}
				}
				if (!idList->isEmpty() && idList->endsWith('|'))
					idList->chop(1);
				if (!valueList->isEmpty() && valueList->endsWith('|'))
					valueList->chop(1);
				emit requestWrite(*idList, *valueList);
				return;
			});

	connect(autoFillBtn,
			&QPushButton::clicked,
			this,
			[this]()
			{
				static std::random_device rd;
				static std::mt19937 gen(rd());
				static std::uniform_real_distribution<float> dist(0.0, 110.0);

				// Clear existing items
				for (int row = 0; row < tableWidget->rowCount(); ++row)
				{
					auto* idItem = tableWidget->item(row, 0);
					auto* valueItem = tableWidget->item(row, 1);
					if (idItem)
						idItem->setText("");
					if (valueItem)
						valueItem->setText("");
				}
				// Auto-fill the table with random values
				for (int row = 0; row < tableWidget->rowCount(); ++row)
				{
					auto* idItem = tableWidget->item(row, 0);
					auto* valueItem = tableWidget->item(row, 1);

					
					if (idItem && valueItem)
					{
						int channel = (row) % 32 + 1; // 01-32
						QChar ab = (gen() % 2 == 0) ? 'A' : 'B';

						if (m_valueType == SignalType::Discrete)
						{
							//#SYSTEMID_RACKID_CH01_MD04_CTRLIN_IN01VALID
							QString signalId = QString("#SYSTEMID_RACKID_CH01_MD04_CTRLIN_IN%1VALID").arg(channel, 2, 10, QChar('0'));

							bool randomValue = (gen() % 2 == 0);
							idItem->setText(signalId);
							valueItem->setText(randomValue ? "true" : "false");
							
						}
						else if (m_valueType == SignalType::AnalogInt32)
						{
							QString signalId = QString("#SYSTEMID_RACKID_CH01_MD02_PI_SERIALNO");
							int randomValue = static_cast<int>(dist(gen));
							idItem->setText(signalId);
							valueItem->setText(QString::number(randomValue));

							idItem = tableWidget->item(row+1, 0);
							valueItem = tableWidget->item(row+1, 1);

							signalId = QString("#SYSTEMID_RACKID_CH01_MD04_PI_SERIALNO");
							idItem->setText(signalId);
							randomValue = static_cast<int>(dist(gen));
							valueItem->setText(QString::number(randomValue));
							break;
						}
						else if (m_valueType == SignalType::AnalogFloat)
						{
							QString signalId = QString("#SYSTEMID_RACKID_CH01_MD02_CTRLIN_IN%1%2").arg(channel, 2, 10, QChar('0')).arg(ab);
							float randomValue = dist(gen);
							idItem->setText(signalId);
							valueItem->setText(QString::number(randomValue, 'f', 2));
							
						}
					}
				}
				return;
			});

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(readBtn);
	buttonLayout->addWidget(writeBtn);
	buttonLayout->addWidget(autoFillBtn);


	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tableWidget);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);
}

void RWMultyToolBox::setValueType(SignalType type)
{
	if (m_valueType != type)
	{
		m_valueType = type;
	}
}

int RWMultyToolBox::filledRowCount() const
{
	int count = 0;
	for (int row = 0; row < tableWidget->rowCount(); ++row)
	{
		auto* idItem = tableWidget->item(row, 0);
		auto* valueItem = tableWidget->item(row, 1);
		if (idItem && valueItem && !idItem->text().isEmpty() && !valueItem->text().isEmpty())
		{
			++count;
		}
	}
	return count;
}