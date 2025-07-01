#include "RWMultiToolBox.h"

#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QVBoxLayout>
#include <random>

#define MAX_ROW_COUNT 32


RWMultiToolBox::RWMultiToolBox(QWidget* parent) :
	QWidget(parent)
{
	m_tableWidget = new QTableWidget(MAX_ROW_COUNT, 2, this); // 2 columns: Signal ID, Value
	m_tableWidget->setHorizontalHeaderLabels({"Signal ID", "Value"});
	m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	for (int row = 0; row < m_tableWidget->rowCount(); ++row)
	{
		m_tableWidget->setItem(row, 0, new QTableWidgetItem(""));
		m_tableWidget->setItem(row, 1, new QTableWidgetItem(""));
	}

	loadSignalFromCSV(m_signalsFileName);
	if (m_signalDefs.empty())
	{
		setDefaultSignalDefinition();
		saveSignalToCSV(m_signalsFileName);
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
				onReadButtonClicked();
			});

	connect(writeBtn,
			&QPushButton::clicked,
			this,
			[this]()
			{
				onWriteButtonClicked();
			});

	connect(autoFillBtn,
			&QPushButton::clicked,
			this,
			[this]()
			{
				onAutoFillButtonClicked();
			});

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(readBtn);
	buttonLayout->addWidget(writeBtn);
	buttonLayout->addWidget(autoFillBtn);


	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_tableWidget);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);
}

void RWMultiToolBox::setValueType(SignalType type)
{
	if (m_valueType != type)
	{
		m_valueType = type;
	}
}

SignalType RWMultiToolBox::valueType() const
{
	return m_valueType;
}

int RWMultiToolBox::filledRowCount() const
{
	int count = 0;
	for (int row = 0; row < m_tableWidget->rowCount(); ++row)
	{
		auto* idItem = m_tableWidget->item(row, 0);
		auto* valueItem = m_tableWidget->item(row, 1);
		if (idItem && valueItem && !idItem->text().isEmpty() && !valueItem->text().isEmpty())
		{
			++count;
		}
	}
	return count;
}

void RWMultiToolBox::loadSignalFromCSV(const QString& filename)
{
	m_signalDefs.clear();

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

		m_signalDefs.emplace_back(type, pattern, start, end);
	}
}

void RWMultiToolBox::setDefaultSignalDefinition()
{
	m_signalDefs.clear();

	m_signalDefs.emplace_back(SignalType::Discrete, "#SYSTEMID_RACKID_CH01_MD04_CTRLIN_IN%1VALID", 1, MAX_ROW_COUNT);
	m_signalDefs.emplace_back(SignalType::AnalogInt32,
							  "#SYSTEMID_RACKID_CH01_MD02_PI_SERIALNO|#SYSTEMID_RACKID_CH01_MD04_PI_SERIALNO",
							  0,
							  0);
	m_signalDefs.emplace_back(SignalType::AnalogFloat, "#SYSTEMID_RACKID_CH01_MD02_CTRLIN_IN%1%2", 1, MAX_ROW_COUNT);
}

void RWMultiToolBox::saveSignalToCSV(const QString& filename) const
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << "TYPE;ID;[Start];[END]\n";
	for (const auto& def : m_signalDefs)
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

void RWMultiToolBox::updateSignalFromCSV(const QString& filename)
{
	loadSignalFromCSV(filename);
}

void RWMultiToolBox::onReadButtonClicked()
{
	QString* idList = new QString;
	for (int row = 0; row < filledRowCount(); ++row)
	{
		auto* idItem = m_tableWidget->item(row, 0);
		if (idItem && !idItem->text().isEmpty())
		{
			idList->append(idItem->text() + "|");
		}
	}
	if (!idList->isEmpty() && idList->endsWith('|'))
		idList->chop(1);
	emit requestRead(*idList);
	return;
}

void RWMultiToolBox::onWriteButtonClicked()
{
	QString* idList = new QString;
	QString* valueList = new QString;
	for (int row = 0; row < filledRowCount(); ++row)
	{
		auto* idItem = m_tableWidget->item(row, 0);
		auto* valueItem = m_tableWidget->item(row, 1);
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
}

void RWMultiToolBox::onAutoFillButtonClicked()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> distFloat(0.0, 110.0);
	static std::uniform_int_distribution<int> distInt(0, 110);

	// Clear existing items
	for (int row = 0; row < m_tableWidget->rowCount(); ++row)
	{
		auto* idItem = m_tableWidget->item(row, 0);
		auto* valueItem = m_tableWidget->item(row, 1);
		if (idItem)
			idItem->setText("");
		if (valueItem)
			valueItem->setText("");
	}

	auto it = std::find_if(m_signalDefs.begin(),
						   m_signalDefs.end(),
						   [this](const SignalDef& def)
						   {
							   return def.type == m_valueType;
						   });
	const SignalDef& def = *it;

	// Auto-fill the table with random values
	for (int row = 0; row < m_tableWidget->rowCount() && row < def.end; ++row)
	{
		auto* idItem = m_tableWidget->item(row, 0);
		auto* valueItem = m_tableWidget->item(row, 1);

		if (!idItem || !valueItem)
			continue;

		int channel = (row) % MAX_ROW_COUNT + 1; // 01-MAX_ROW_COUNT
		QChar ab = (gen() % 2 == 0) ? 'A' : 'B';


		QString signalId = def.pattern;

		if (m_valueType == SignalType::Discrete)
		{
			if (signalId.contains('|'))
			{
				QStringList ids;
				ids = signalId.split('|', Qt::SkipEmptyParts);
				if (!ids.isEmpty())
				{
					int idx = gen() % ids.size();
					signalId = ids[idx];
				}
			}
			if (signalId.contains("%1"))
			{
				signalId = signalId.arg(channel, 2, 10, QChar('0'));
			}
			if (signalId.contains("%2"))
			{
				signalId = signalId.arg(ab);
			}
			idItem->setText(signalId);
			valueItem->setText((gen() % 2 == 0) ? "true" : "false");
		}
		else if (m_valueType == SignalType::AnalogInt32)
		{
			if (signalId.contains('|'))
			{
				QStringList ids = signalId.split('|', Qt::SkipEmptyParts);
				if (!ids.isEmpty())
				{
					int idx = gen() % ids.size();
					signalId = ids[idx];
				}
			}
			if (signalId.contains("%1"))
			{
				signalId = signalId.arg(channel, 2, 10, QChar('0'));
			}
			if (signalId.contains("%2"))
			{
				signalId = signalId.arg(ab);
			}
			idItem->setText(signalId);
			valueItem->setText(QString::number(distInt(gen)));
		}
		else if (m_valueType == SignalType::AnalogFloat)
		{
			if (signalId.contains('|'))
			{
				QStringList ids;
				ids = signalId.split('|', Qt::SkipEmptyParts);
				if (!ids.isEmpty())
				{
					int idx = gen() % ids.size();
					signalId = ids[idx];
				}
			}
			if (signalId.contains("%1"))
			{
				signalId = signalId.arg(channel, 2, 10, QChar('0'));
			}
			if (signalId.contains("%2"))
			{
				signalId = signalId.arg(ab);
			}
			idItem->setText(signalId);
			valueItem->setText(QString::number(distFloat(gen), 'f', 2));
		}
	}
	return;
}