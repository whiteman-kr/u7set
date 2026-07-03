#include "ChooseActuatorDialog.h"

#include <VFrame30/ActuatorHeader.h>

#include <QDialogButtonBox>

namespace
{
	constexpr int ActuatorItemRole = Qt::UserRole;
	QString s_lastSelectedActuatorTypeId;
} // namespace

ChooseActuatorDialog::ChooseActuatorDialog(const std::vector<std::shared_ptr<VFrame30::ActuatorHeader>>& actuators, QWidget* parent) :
	QDialog{parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint},
	m_actuators{actuators}
{
	QStringList headers;
	headers.push_back(tr("Caption"));
	headers.push_back(tr("ID"));

	// Quick search
	//
	m_quickSearchLabel = new QLabel{tr("Quick search:"), this};
	m_quickSearchLineEdit = new QLineEdit{this};

	// --
	//
	m_actuatorsTreeWidget = new QTreeWidget{this};
	m_actuatorsTreeWidget->setHeaderLabels(headers);

	m_actuatorsTreeWidget->setSortingEnabled(true);
	m_actuatorsTreeWidget->sortItems(0, Qt::AscendingOrder);

	// Caption
	//
	m_captionLabel = new QLabel{tr("Caption:"), this};
	m_captionLineEdit = new QLineEdit{this};
	m_captionLineEdit->setReadOnly(true);

	// Description
	m_descriptionLabel = new QLabel{tr("Description:"), this};
	m_descriptionTextEdit = new QPlainTextEdit{this};
	m_descriptionTextEdit->setReadOnly(true);

	//
	// --
	//
	QGridLayout* layout = new QGridLayout{this};
	layout->addWidget(m_quickSearchLabel, 0, 0);
	layout->addWidget(m_quickSearchLineEdit, 0, 1);

	layout->addWidget(m_actuatorsTreeWidget, 1, 0, 1, 2);

	layout->addWidget(m_captionLabel, 0, 2);
	layout->addWidget(m_captionLineEdit, 0, 3);
	layout->addWidget(m_descriptionLabel, 1, 2, Qt::AlignTop);
	layout->addWidget(m_descriptionTextEdit, 1, 3);

	m_buttonBox = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	layout->addWidget(m_buttonBox, 2, 2, 1, 2);

	connect(m_quickSearchLineEdit, &QLineEdit::textChanged, this, &ChooseActuatorDialog::fillTree);
	connect(m_actuatorsTreeWidget, &QTreeWidget::itemSelectionChanged, this, &ChooseActuatorDialog::itemSelectionChanged);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ChooseActuatorDialog::acceptSelection);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_actuatorsTreeWidget, &QTreeWidget::doubleClicked, this, &ChooseActuatorDialog::itemDoubleClicked);

	fillTree();

	return;
}

ChooseActuatorDialog::~ChooseActuatorDialog()
{
	QSettings{}.setValue("ChooseActuatorDialog/Geometry", this->saveGeometry());
	QSettings{}.setValue("ChooseActuatorDialog/TreeState", this->m_actuatorsTreeWidget->header()->saveState());
}

void ChooseActuatorDialog::showEvent(QShowEvent*)
{
	if (QByteArray geometry = QSettings{}.value("ChooseActuatorDialog/Geometry").toByteArray(); geometry.isNull() == false)
	{
		this->restoreGeometry(geometry);
	}
	else
	{
		// Resize depends on monitor size, DPI, resolution
		//
		QRect screen = parentWidget()->screen()->availableGeometry();

		resize(static_cast<int>(screen.width() * 0.35), static_cast<int>(screen.height() * 0.40));

		move(screen.center() - rect().center());
	}

	if (QByteArray treeState = QSettings{}.value("ChooseActuatorDialog/TreeState").toByteArray(); treeState.isNull() == false)
	{
		m_actuatorsTreeWidget->header()->restoreState(treeState);
	}
	else
	{
		m_actuatorsTreeWidget->header()->adjustSize();
	}

	return;
}

void ChooseActuatorDialog::fillTree()
{
	m_selectedActuator.reset();
	m_captionLineEdit->clear();
	m_descriptionTextEdit->clear();
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	m_actuatorsTreeWidget->clear();

	QString searchMask = m_quickSearchLineEdit->text();

	QList<QTreeWidgetItem*> items;
	for (qsizetype i = 0; i < static_cast<qsizetype>(m_actuators.size()); ++i)
	{
		const auto& ah = m_actuators[static_cast<size_t>(i)];
		bool add = (searchMask.isEmpty() == true) ||
				   (searchMask.isEmpty() == false && (ah->caption().contains(searchMask, Qt::CaseInsensitive) == true ||
													  ah->actuatorTypeId().contains(searchMask, Qt::CaseInsensitive) == true));
		if (add == false)
		{
			continue;
		}

		QStringList sl;
		sl.push_back(ah->caption());
		sl.push_back(ah->actuatorTypeId());

		auto* item = new QTreeWidgetItem(sl);
		item->setData(0, ActuatorItemRole, static_cast<qulonglong>(i));
		items.append(item);
	}

	m_actuatorsTreeWidget->insertTopLevelItems(0, items);
	if (s_lastSelectedActuatorTypeId.isEmpty() == false)
	{
		const QList<QTreeWidgetItem*> matches = m_actuatorsTreeWidget->findItems(s_lastSelectedActuatorTypeId, Qt::MatchExactly, 1);
		if (matches.isEmpty() == false)
		{
			m_actuatorsTreeWidget->setCurrentItem(matches.front());
			itemSelectionChanged();
			return;
		}
	}

	return;
}

void ChooseActuatorDialog::itemSelectionChanged()
{
	QTreeWidgetItem* item = m_actuatorsTreeWidget->currentItem();
	if (item == nullptr)
	{
		m_selectedActuator.reset();
		m_captionLineEdit->clear();
		m_descriptionTextEdit->clear();
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	bool ok = false;
	qulonglong index = item->data(0, ActuatorItemRole).toULongLong(&ok);
	if (ok == false || index >= m_actuators.size())
	{
		m_selectedActuator.reset();
		m_captionLineEdit->clear();
		m_descriptionTextEdit->clear();
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	m_selectedActuator = m_actuators[static_cast<size_t>(index)];
	m_captionLineEdit->setText(m_selectedActuator->caption());
	m_descriptionTextEdit->setPlainText(m_selectedActuator->description());
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	return;
}

void ChooseActuatorDialog::acceptSelection()
{
	if (m_selectedActuator == nullptr)
	{
		itemSelectionChanged();
	}

	if (m_selectedActuator != nullptr)
	{
		s_lastSelectedActuatorTypeId = m_selectedActuator->actuatorTypeId();
		accept();
	}

	return;
}

void ChooseActuatorDialog::itemDoubleClicked(QModelIndex index)
{
	if (index.isValid() == false)
	{
		return;
	}

	m_actuatorsTreeWidget->setCurrentIndex(index);
	itemSelectionChanged();
	acceptSelection();
	return;
}

std::shared_ptr<VFrame30::ActuatorHeader> ChooseActuatorDialog::result()
{
	return m_selectedActuator;
}
