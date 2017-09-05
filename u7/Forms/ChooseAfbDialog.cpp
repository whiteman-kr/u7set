#include "Stable.h"
#include "ChooseAfbDialog.h"
#include "ui_ChooseAfbDialog.h"
#include "../VFrame30/SchemaItemAfb.h"

int ChooseAfbDialog::m_lastSelectedIndex = -1;
QString ChooseAfbDialog::AllCategoryName = "All";
QString ChooseAfbDialog::m_lastSelectedCategory = AllCategoryName;
Qt::SortOrder ChooseAfbDialog::m_lastSortOrder = Qt::SortOrder::AscendingOrder;

ChooseAfbDialog::ChooseAfbDialog(const std::vector<std::shared_ptr<Afb::AfbElement>>& elements, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::ChooseAfbDialog)
{
	ui->setupUi(this);

	for (std::shared_ptr<Afb::AfbElement> e : elements)
	{
        m_elements.push_back(e);
	}

	QStringList columns;
	columns << "Caption";
	ui->m_afbTree->setHeaderLabels(columns);

	ui->m_afbTree->setColumnWidth(0, 150);

    fillTree();

	return;
}

void ChooseAfbDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(screen.width() * 0.35, screen.height() * 0.40);

	move(screen.center() - rect().center());

	return;
}

void ChooseAfbDialog::fillTree()
{
    ui->m_afbTree->clear();

    QString mask = ui->editQuickSearch->text();

    // create categories list
    //
    QStringList categories;
    categories.append(AllCategoryName);

    for (std::shared_ptr<Afb::AfbElement> e : m_elements)
    {
        if (e->category().isEmpty())
        {
            continue;
        }

        if (categories.contains(e->category()) == false)
        {
            categories.append(e->category());
        }
    }

    if (mask.isEmpty() == true)
    {
        // add categories
        //
        for (QString cat : categories)
        {
            QTreeWidgetItem* catItem = new QTreeWidgetItem();
            catItem->setText(0, cat);
            catItem->setData(0, Qt::UserRole, -1);
            ui->m_afbTree->addTopLevelItem(catItem);

            int index = 0;

            for (std::shared_ptr<Afb::AfbElement> e : m_elements)
            {
                if (e->internalUse() == false &&
                        (e->category() == cat || cat == AllCategoryName))
                {
                    QTreeWidgetItem* item = new QTreeWidgetItem(catItem);
                    item->setText(0, e->caption());
                    item->setData(0, Qt::UserRole, index);

                    if (m_lastSelectedCategory == cat && index == m_lastSelectedIndex)
                    {
                        ui->m_afbTree->setCurrentItem(item);
                    }
                }

                index++;
            }

            if (m_lastSelectedCategory == cat)
            {
                ui->m_afbTree->expandItem(catItem);
            }
        }
    }
    else
    {
        // mask is entered, create a simple list
        //
        int index = 0;

        for (std::shared_ptr<Afb::AfbElement> e : m_elements)
        {
            if (e->internalUse() == false && e->caption().contains(mask, Qt::CaseInsensitive))
            {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, e->caption());
                item->setData(0, Qt::UserRole, index);
                ui->m_afbTree->addTopLevelItem(item);
            }

            index++;
        }
    }


    ui->m_afbTree->sortItems(0, m_lastSortOrder);
    ui->m_afbTree->setSortingEnabled(true);

	return;
}

ChooseAfbDialog::~ChooseAfbDialog()
{
	delete ui;
}

int ChooseAfbDialog::index()
{
	return m_lastSelectedIndex;
}

int ChooseAfbDialog::getSelectedIndex()
{
	QList<QTreeWidgetItem*> selectedItems = ui->m_afbTree->selectedItems();
	if (selectedItems.size() == 0 || selectedItems.size() > 1)
	{
		return -1;
    }

    int result = selectedItems[0]->data(0, Qt::UserRole).toInt();
    return result;

}

QString ChooseAfbDialog::getSelectedCategory()
{
    QList<QTreeWidgetItem*> selectedItems = ui->m_afbTree->selectedItems();
    if (selectedItems.size() == 0 || selectedItems.size() > 1)
    {
		return "";
    }

    QTreeWidgetItem* item = selectedItems[0];

    QString result;

    int index = item->data(0, Qt::UserRole).toInt();
    if (index != -1)
    {
        // this is afb item, get parent
        //
        item = item->parent();
    }

    if (item == nullptr)
    {
        //root, the item is an AFB item with entered mask
        //
        result = AllCategoryName;
    }
    else
    {
        result = item->text(0);
    }

    return result;
}

void ChooseAfbDialog::on_btnOk_clicked()
{
	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();

	m_lastSelectedIndex = getSelectedIndex();
    m_lastSelectedCategory = getSelectedCategory();

	if (m_lastSelectedIndex == -1)
	{
		return;
	}

	if (m_lastSelectedIndex < 0 || static_cast<size_t>(m_lastSelectedIndex) >= m_elements.size())
	{
		assert(false);
		return;
	}

	accept();

	return;
}

void ChooseAfbDialog::reject()
{
	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();

	done(QDialog::Rejected);
}

void ChooseAfbDialog::closeEvent(QCloseEvent* e)
{
	Q_UNUSED(e);

	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();
}

void ChooseAfbDialog::on_btnCancel_clicked()
{
	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();

	done(QDialog::Rejected);
}

void ChooseAfbDialog::on_m_afbTree_itemSelectionChanged()
{
	int selectedIndex = getSelectedIndex();

	if (selectedIndex == -1)
	{
		ui->labelCaption->setText("");
		ui->labelDescription->setPlainText("");

		ui->btnOk->setEnabled(false);

		ui->afbHelpWidget->setAfb(nullptr);
	}
	else
	{
		if (selectedIndex < 0 || static_cast<size_t>(selectedIndex) >= m_elements.size())
		{
			assert(false);
			return;
		}

		std::shared_ptr<Afb::AfbElement> e = m_elements[selectedIndex];

		ui->afbHelpWidget->setAfb(e);

        ui->labelCaption->setText(e->caption() + tr(", version ") + e->version());
		ui->labelDescription->setPlainText(e->description());

		ui->btnOk->setEnabled(true);
	}
}

void ChooseAfbDialog::on_m_afbTree_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	on_btnOk_clicked();
}

void ChooseAfbDialog::on_editQuickSearch_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1);

    fillTree();
}


AfbHelpWidget::AfbHelpWidget(QWidget* parent) :
	QWidget(parent)
{
}


void AfbHelpWidget::paintEvent(QPaintEvent* e)
{
	QPainter p(this);
	p.fillRect(e->rect(), Qt::white);

	if (m_afb == nullptr)
	{
		return;
	}

	m_afb->drawAfbHelp(&p, rect());

	return;
}

void AfbHelpWidget::setAfb(std::shared_ptr<Afb::AfbElement> afb)
{
	if (afb == nullptr)
	{
		m_afb.reset();
	}
	else
	{
		QString errorMessage;
		m_afb = std::make_shared<VFrame30::SchemaItemAfb>(VFrame30::SchemaUnit::Display, *afb, &errorMessage);
	}

	update();

	return;
}
