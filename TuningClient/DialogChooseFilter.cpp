#include "DialogChooseFilter.h"

#include <QListWidget>

Q_DECLARE_METATYPE (TuningFilter*)

DialogChooseFilter::DialogChooseFilter(QWidget* parent, TuningFilter* parentFilter, TuningFilter::InterfaceType interfaceType, TuningFilter::Source source)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	if (parentFilter == nullptr)
	{
		assert(parentFilter);
		return;
	}

	QVBoxLayout* mainLayout = new QVBoxLayout();

	QLabel* l = new QLabel(tr("Choose a Filter:"));
	mainLayout->addWidget(l);

	// Listbox

	m_listBox = new QListWidget();
	m_listBox->setSortingEnabled(false);
	m_listBox->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_listBox, &QListWidget::doubleClicked, this, &DialogChooseFilter::accept);

	mainLayout->addWidget(m_listBox);

	for (int i = 0; i < parentFilter->childFiltersCount(); i++)
	{
		TuningFilter* cf = parentFilter->childFilter(i).get();
		if (cf == nullptr)
		{
			assert(cf);
			return;
		}

		if (cf->interfaceType() != interfaceType)
		{
			continue;
		}
		if (cf->source() != source)
		{
			continue;
		}

		QListWidgetItem* newItem = new QListWidgetItem;
		newItem->setData(Qt::UserRole, QVariant::fromValue<TuningFilter*>(cf));
		newItem->setText(cf->caption());
		m_listBox->addItem(newItem);
	}

	// Buttons

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	buttonLayout->addStretch();

	QPushButton* b = new QPushButton(tr("OK"));
	connect(b, &QPushButton::clicked, this, &DialogChooseFilter::accept);
	buttonLayout->addWidget(b);

	b = new QPushButton(tr("Cancel"));
	connect(b, &QPushButton::clicked, this, &DialogChooseFilter::reject);
	buttonLayout->addWidget(b);

	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
}

TuningFilter* DialogChooseFilter::chosenFilter() const
{
	return m_chosenFilter;
}

void DialogChooseFilter::accept()
{
	QList<QListWidgetItem*> selected = m_listBox->selectedItems();

	if (selected.empty() == true)
	{
		return;
	}

	QListWidgetItem* item = selected.front();
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	TuningFilter* cf = item->data(Qt::UserRole).value<TuningFilter*>();
	if (cf == nullptr)
	{
		Q_ASSERT(cf);
		return;
	}

	m_chosenFilter = cf;

	QDialog::accept();
	return;
}
