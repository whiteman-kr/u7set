#include "ChooseUfbDialog.h"
#include "ui_ChooseUfbDialog.h"

ChooseUfbDialog::ChooseUfbDialog(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::ChooseUfbDialog),
	m_ufbs(ufbs)
{
	ui->setupUi(this);

	QStringList headers;
	headers.push_back(tr("Caption"));
	headers.push_back(tr("ID"));

	ui->ufbElements->setHeaderLabels(headers);

	ui->ufbElements->setSortingEnabled(true);
	ui->ufbElements->sortItems(0, Qt::AscendingOrder);

	ui->caption->setReadOnly(true);
	ui->description->setReadOnly(true);

	ui->ok->setEnabled(false);

	connect(ui->quickSearch, &QLineEdit::textChanged, this, &ChooseUfbDialog::fillTree);
	connect(ui->ufbElements, &QTreeWidget::itemSelectionChanged, this, &ChooseUfbDialog::itemSelectionChanged);

	connect(ui->ok, &QPushButton::clicked, this, &ChooseUfbDialog::accept);
	connect(ui->cancel, &QPushButton::clicked, this, &ChooseUfbDialog::reject);
	connect(ui->ufbElements, &QTreeWidget::doubleClicked, this, &ChooseUfbDialog::itemDoubleClicked);

	fillTree();

	return;
}

ChooseUfbDialog::~ChooseUfbDialog()
{
	QSettings{}.setValue("ChooseUfbDialog/Geometry", this->saveGeometry());
	QSettings{}.setValue("ChooseUfbDialog/TreeState", this->ui->ufbElements->header()->saveState());

	delete ui;
}

void ChooseUfbDialog::showEvent(QShowEvent*)
{
	if (QByteArray geometry = QSettings{}.value("ChooseUfbDialog/Geometry").toByteArray();
		geometry.isNull() == false)
	{
		this->restoreGeometry(geometry);
	}
	else
	{
		// Resize depends on monitor size, DPI, resolution
		//
		QRect screen = QDesktopWidget().availableGeometry(parentWidget());

		resize(static_cast<int>(screen.width() * 0.35),
			   static_cast<int>(screen.height() * 0.40));

		move(screen.center() - rect().center());
	}

	if (QByteArray treeState = QSettings{}.value("ChooseUfbDialog/TreeState").toByteArray();
		treeState.isNull() == false)
	{
		this->ui->ufbElements->header()->restoreState(treeState);
	}
	else
	{
		this->ui->ufbElements->header()->adjustSize();
	}

	return;
}

void ChooseUfbDialog::fillTree()
{
	ui->ok->setEnabled(false);
	ui->caption->clear();
	ui->description->clear();
	ui->ufbElements->clear();

	QString searchMask = ui->quickSearch->text();

	QList<QTreeWidgetItem*> items;
	for (std::shared_ptr<VFrame30::UfbSchema>& ufb : m_ufbs)
	{
		bool add = (searchMask.isEmpty() == true) || (searchMask.isEmpty() == false && ufb->caption().contains(searchMask, Qt::CaseInsensitive) == true);
		if (add == false)
		{
			continue;
		}

		QStringList sl;
		sl.push_back(ufb->caption());
		sl.push_back(ufb->schemaId());

		items.append(new QTreeWidgetItem(sl));
	}

	ui->ufbElements->insertTopLevelItems(0, items);

	return;
}

void ChooseUfbDialog::itemSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = ui->ufbElements->selectedItems();
	if (selectedItems.size() == 1)
	{
		QTreeWidgetItem* item = selectedItems[0];
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
		{
			if (ufb->caption() == item->text(0))
			{
				// UFB was found
				//
				m_selectedUfb = ufb;

				ui->caption->setText(ufb->caption());
				ui->description->setPlainText(ufb->description());
				ui->ok->setEnabled(true);

				return;
			}
		}
	}

	// No UFB was found

	m_selectedUfb = nullptr;

	ui->caption->setText(QString());
	ui->description->setPlainText(QString());
	ui->ok->setEnabled(false);

	return;
}

void ChooseUfbDialog::itemDoubleClicked(QModelIndex index)
{
	QString itemId = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();

	for (std::shared_ptr<VFrame30::UfbSchema>& ufb : m_ufbs)
	{
		if (ufb->schemaId() == itemId)
		{
			m_selectedUfb = ufb;

			ui->caption->setText(ufb->caption());
			ui->description->setPlainText(ufb->description());
			ui->ok->setEnabled(true);

			accept();

			break;
		}
	}

	return;
}

std::shared_ptr<VFrame30::UfbSchema> ChooseUfbDialog::result()
{
	return m_selectedUfb;
}
