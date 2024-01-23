#include "DialogWriteValues.h"
#include <QTableWidget>
#include <utility>

QSize TuningValuesTreeWidget::sizeHint() const
{
	QTreeWidgetItemIterator allIterator(const_cast<TuningValuesTreeWidget*>(this), QTreeWidgetItemIterator::All);

	// Height of all item and header.
	//
	const int frameWidth = style()->styleHint( QStyle::StyleHint::SH_WindowFrame_Mask);
	int tableHeight = header()->height() + 2 * frameWidth;

	// While iterator is valid.
	//
	while( *allIterator )
	{
		// Update the height.
		//
		QTreeWidgetItem* item = *allIterator;
		tableHeight += visualItemRect(item).height();

		// Next iterator
		//
		++allIterator;
	}

	return QSize(400, tableHeight);
}

int DialogWriteValues::askConfirmation(const AppSignalParam& param,
									   const TuningValue& oldValue,
									   const TuningValue& newValue,
									   E::AnalogFormat analogFormat,
									   QWidget* parent)
{
	DialogWriteValues d({param}, {oldValue}, {newValue}, analogFormat, parent);
	return d.exec();
}

int DialogWriteValues::askConfirmation(std::vector<AppSignalParam>& params,
									   std::vector<TuningValue>& oldValues,
									   std::vector<TuningValue>& newValues,
									   E::AnalogFormat analogFormat,
									   QWidget* parent)
{
	DialogWriteValues d(std::move(params), std::move(oldValues), std::move(newValues), analogFormat, parent);
	return d.exec();
}

DialogWriteValues::DialogWriteValues(const std::vector<AppSignalParam>& params,
									 const std::vector<TuningValue>& oldValues,
									 const std::vector<TuningValue>& newValues,
									 E::AnalogFormat analogFormat,
									 QWidget* parent):
	QDialog(parent),
	m_params(params),
	m_oldValues(oldValues),
	m_newValues(newValues),
	m_analogFormat(analogFormat)
{
	if (m_params.size() != m_oldValues.size() || m_params.size() != m_newValues.size())
	{
		Q_ASSERT(false);
		return;
	}

	setMinimumWidth(700);

	QVBoxLayout* rightLayout = new QVBoxLayout();

	rightLayout->addWidget(new QLabel(tr("The following tuning values will be written. Are you sure you want to continue?")));

	// Table
	//
	QStringList headerLabels;
	headerLabels.push_back(tr("Signal ID"));
	headerLabels.push_back(tr("Caption"));
	headerLabels.push_back(tr("Default"));
	headerLabels.push_back(tr("Current"));
	headerLabels.push_back(tr("Writing"));

	m_table = new TuningValuesTreeWidget();
	m_table->setHeaderLabels(headerLabels);
	m_table->setRootIsDecorated(false);
	rightLayout->addWidget(m_table);

	m_table->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
	m_table->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
	m_table->header()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
	m_table->header()->setSectionResizeMode(3, QHeaderView::ResizeMode::ResizeToContents);
	m_table->header()->setSectionResizeMode(4, QHeaderView::ResizeMode::ResizeToContents);
	m_table->header()->setStretchLastSection(false);

	// Fill table data
	//
	fillTable(m_defaultSignalsCount);

	// Adjust table height
	//
	m_table->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

	// Buttons
	//
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	rightLayout->addLayout(buttonLayout);

	if (m_params.size() > m_defaultSignalsCount)
	{
		m_showAll = new QPushButton(tr("Show All"));
		buttonLayout->addWidget(m_showAll);
		connect(m_showAll, &QPushButton::clicked, this, &DialogWriteValues::onShowAll);
	}

	buttonLayout->addStretch();

	QPushButton* yesBtn = new QPushButton(tr("Yes"));
	buttonLayout->addWidget(yesBtn);

	QPushButton* noBtn = new QPushButton(tr("No"));
	buttonLayout->addWidget(noBtn);
	noBtn->setDefault(true);

	connect(yesBtn, &QPushButton::clicked, this, &DialogWriteValues::accept);
	connect(noBtn, &QPushButton::clicked, this, &DialogWriteValues::reject);

	// Icon
	//
	QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxWarning); //or
	int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
	QLabel* logo = new QLabel("a");
	logo->setPixmap(QPixmap(icon.pixmap(iconSize, iconSize)));

	// Main layout
	//
	QHBoxLayout* mainLayout = new QHBoxLayout();
	mainLayout->addWidget(logo);
	mainLayout->addLayout(rightLayout);
	setLayout(mainLayout);

	noBtn->setFocus();

	return;
}

void DialogWriteValues::fillTable(int maxCount)
{
	Q_ASSERT(m_table);
	m_table->clear();

	int rowCount = static_cast<int>(m_params.size());
	for (int row = 0; row < rowCount; row++)
	{
		const auto& asp = m_params[row];

		QStringList strings;

		strings << asp.customSignalId();
		strings << asp.caption();

		if (asp.isAnalog() == true)
		{
			strings << asp.tuningDefaultValue().toString(m_analogFormat, asp.precision());
			strings << m_oldValues[row].toString(m_analogFormat, asp.precision());
			strings << m_newValues[row].toString(m_analogFormat, asp.precision());
		}
		else
		{
			strings << asp.tuningDefaultValue().toString();
			strings << m_oldValues[row].toString();
			strings << m_newValues[row].toString();
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(strings);
		m_table->addTopLevelItem(item);

		if (row == maxCount - 1 && row != rowCount - 1)
		{
			QTreeWidgetItem* breakItem = new QTreeWidgetItem(QStringList() <<
															 QString() <<  tr("and %1 more signals. Press \"Show all\" to view them.")
															 .arg(rowCount - row - 1));
			m_table->addTopLevelItem(breakItem);
			break;
		}
	}
}

void DialogWriteValues::onShowAll()
{
	fillTable(static_cast<int>(m_params.size()));

	Q_ASSERT(m_showAll);
	m_showAll->setEnabled(false);
}
