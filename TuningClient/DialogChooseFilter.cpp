#include "DialogChooseFilter.h"
#include <AppSignalLists/SignalList.h>

DialogChooseFilter::DialogChooseFilter(const AppSignalLists::AppSignalListSet& appSignalLists, const QStringList& systemTags, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();

	QLabel* l = new QLabel(tr("Choose a List:"));
	mainLayout->addWidget(l);

	// Listbox

	m_listBox = new QListWidget();
	m_listBox->setSortingEnabled(false);
	m_listBox->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_listBox, &QListWidget::doubleClicked, this, &DialogChooseFilter::accept);
	mainLayout->addWidget(m_listBox);

	for (int i = 0; i < appSignalLists.count(); i++)
	{
		auto list = appSignalLists.get(i);
		if (list == nullptr) 
		{
			Q_ASSERT(list);
			continue;
		}

		if (list->hasAnySystemTag(systemTags) == true)
		{
			QListWidgetItem* newItem = new QListWidgetItem;
			newItem->setData(Qt::UserRole, list->uuid());
			newItem->setText(list->caption());
			m_listBox->addItem(newItem);
		}
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

QUuid DialogChooseFilter::chosenFilterUuid() const
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

	m_chosenFilter = item->data(Qt::UserRole).toUuid();

	QDialog::accept();
	return;
}
