#include "RWMultyToolBox.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QSettings>
#include <QFile>
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

	loadSignalFromCSV("signals.csv");
	if (signalDefs.empty())
	{
		setDefaultSignalDefs();
		saveSignalToCSV("signals.csv");
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
				static std::uniform_real_distribution<float> distFloat(0.0, 110.0);
				static std::uniform_int_distribution<int> distInt(0, 110);

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

					if (!idItem || !valueItem)
						continue;

					int channel = (row) % 32 + 1; // 01-32
					QChar ab = (gen() % 2 == 0) ? 'A' : 'B';

					
					auto it = std::find_if(signalDefs.begin(),
										   signalDefs.end(),
										   [this](const SignalDef& def)
										   {
											   return def.type == m_valueType;
										   });

					if (it == signalDefs.end())
						continue;

					const SignalDef& def = *it;

					if (m_valueType == SignalType::Discrete)
					{
						QString signalId = def.pattern;
						if (signalId.contains("%1"))
							signalId = signalId.arg(channel, 2, 10, QChar('0'));
						idItem->setText(signalId);
						valueItem->setText((gen() % 2 == 0) ? "true" : "false");
					}
					else if (m_valueType == SignalType::AnalogInt32)
					{
						QStringList ids = def.pattern.split('|', Qt::SkipEmptyParts);
						for (int i = 0; i < ids.size() && row < tableWidget->rowCount(); ++i, ++row)
						{
							auto* idItem2 = tableWidget->item(row, 0);
							auto* valueItem2 = tableWidget->item(row, 1);
							if (idItem2 && valueItem2)
							{
								idItem2->setText(ids[i]);
								valueItem2->setText(QString::number(distInt(gen)));
							}
						}
						return;
					}
					else if (m_valueType == SignalType::AnalogFloat)
					{
						QString signalId = def.pattern;
						if (signalId.contains("%1"))
							signalId = signalId.arg(channel, 2, 10, QChar('0'));
						if (signalId.contains("%2"))
							signalId = signalId.arg(ab);
						idItem->setText(signalId);
						valueItem->setText(QString::number(distFloat(gen), 'f', 2));
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

void RWMultyToolBox::loadSignalFromCSV(const QString& filename)
{
	signalDefs.clear();

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine().trimmed();
		if (line.isEmpty() || line.startsWith('#'))
			continue;

		auto parts = line.split(';');
		if (parts.size() < 2)
			continue;

		SignalType type;
		if (parts[0].compare("I", Qt::CaseInsensitive) == 0)
			type = SignalType::AnalogInt32;
		else if (parts[0].compare("F", Qt::CaseInsensitive) == 0)
			type = SignalType::AnalogFloat;
		else if (parts[0].compare("D", Qt::CaseInsensitive) == 0)
			type = SignalType::Discrete;
		else
			continue;

		QString pattern = parts[1];
		int start = (parts.size() > 2) ? parts[2].toInt() : 1;
		int end = (parts.size() > 3) ? parts[3].toInt() : start;

		signalDefs.push_back({type, pattern, start, end});
	}
}

void RWMultyToolBox::setDefaultSignalDefs()
{
	signalDefs.clear();

	signalDefs.push_back({SignalType::Discrete, "#SYSTEMID_RACKID_CH01_MD04_CTRLIN_IN%1VALID", 1, 32});
	signalDefs.push_back({SignalType::AnalogInt32, "#SYSTEMID_RACKID_CH01_MD02_PI_SERIALNO|#SYSTEMID_RACKID_CH01_MD04_PI_SERIALNO", 0, 0});
	signalDefs.push_back({SignalType::AnalogFloat, "#SYSTEMID_RACKID_CH01_MD02_CTRLIN_IN%1%2", 1, 32});
}

void RWMultyToolBox::saveSignalToCSV(const QString& filename) const
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << "TYPE;ID;[Start];[END]\n";
	for (const auto& def : signalDefs)
	{
		QString typeChar;
		switch (def.type)
		{
		case SignalType::Discrete:
			typeChar = "D";
			break;
		case SignalType::AnalogInt32:
			typeChar = "I";
			break;
		case SignalType::AnalogFloat:
			typeChar = "F";
			break;
		default:
			typeChar = "ERROR";
			break;
		}
		out << typeChar << ";" << def.pattern << ";" << def.start << ";" << def.end << "\n";
	}
	file.close();
}