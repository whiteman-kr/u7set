#include "SelectSchemaWidget.h"
#include <QHBoxLayout>
#include <QDialog>
#include <QHeaderView>
#include <QPropertyAnimation>

SelectSchemaWidget::SelectSchemaWidget(MonitorConfigController* configController, MonitorCentralWidget* centralWidget) :
	m_configController(configController),
	m_centraWidget(centralWidget)
{
	assert(m_configController);
	assert(m_centraWidget);

	m_schemas.reserve(32);

	// --
	//
	m_button = new QPushButton;
	m_button->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
	m_button->setStyleSheet("Text-align:left");

	// --
	//
	setLayout(new QHBoxLayout);
	layout()->setMargin(layout()->margin() / 4);

	layout()->addWidget(m_button);

	// --
	//
	connect(m_button, &QPushButton::clicked, this, &SelectSchemaWidget::slot_buttonClicked);

	connect(configController, &MonitorConfigController::configurationArrived, this, &SelectSchemaWidget::slot_configurationArrived);
	connect(m_centraWidget, &MonitorCentralWidget::signal_schemaChanged, this, &SelectSchemaWidget::slot_schemaChanged);

	return;
}

void SelectSchemaWidget::clear()
{
	m_schemas.clear();
	setCurrentSchema(QString());

	return;
}

void SelectSchemaWidget::addSchema(QString schemaId, QString caption)
{
	m_schemas.push_back(SelectSchemaItem{schemaId, caption});
}

bool SelectSchemaWidget::setCurrentSchema(QString schemaId)
{
	m_currentSchemaId.clear();

	for (const SelectSchemaItem& s : m_schemas)
	{
		if (s.schemaId == schemaId)
		{
			m_currentSchemaId = s.schemaId;
			m_button->setText(QString(" %1\n %2").arg(s.schemaId).arg(s.caption));
			return true;
		}
	}

	m_button->setText(QString());

	return false;
}

QString SelectSchemaWidget::currentSchemaId() const
{
	return m_currentSchemaId;
}

void SelectSchemaWidget::slot_configurationArrived(ConfigSettings)
{
	assert(m_button);
	assert(m_configController);
	assert(m_centraWidget);

	// Save state
	//
	QString selectedSchemaId;

	MonitorSchemaWidget* tab = m_centraWidget->currentTab();
	if (tab != nullptr)
	{
		selectedSchemaId = tab->schemaId();
	}

	// Clear all and fill with new data;
	//
	clear();

	// Get arrived schamas
	//
	std::vector<VFrame30::SchemaDetails> schemas = m_configController->schemasDetails();

	std::sort(schemas.begin(), schemas.end(),
		[](const VFrame30::SchemaDetails& s1, const VFrame30::SchemaDetails& s2) -> bool
		{
			return s1.m_schemaId < s2.m_schemaId;
		});


	// Assign schemas to this contol
	//
	for (const VFrame30::SchemaDetails& s : schemas)
	{
		m_schemas.push_back({s.m_schemaId, s.m_caption});
	}

	// Restore selected
	//
	setCurrentSchema(selectedSchemaId);

	return;
}

void SelectSchemaWidget::slot_schemaChanged(QString schemaId)
{
	setCurrentSchema(schemaId);
}

void SelectSchemaWidget::slot_buttonClicked()
{
	SelectSchemaPopup w(m_currentSchemaId, m_schemas, this);

	QPoint buttonPos = mapToGlobal(m_button->pos());
	buttonPos.setY(buttonPos.y() + m_button->rect().height());

	w.move(buttonPos);

	w.setMinimumWidth(m_button->rect().width());		// Lock the width, it will not be possible to change it from UI
	w.setMaximumWidth(m_button->rect().width() * 2);

	if (m_lastPopupHeight == 0)
	{
		w.resize(m_button->rect().width(), w.sizeHint().height() * 2);
	}
	else
	{
		w.resize(m_lastPopupWidth, m_lastPopupHeight);
	}

	w.exec();

	if (w.selectedSchemaId().isEmpty() == false &&
		w.selectedSchemaId() != m_currentSchemaId)
	{
		emit selectionChanged(w.selectedSchemaId());
	}

	m_lastPopupHeight = w.height();
	m_lastPopupWidth = w.width();

	return;
}


SelectSchemaPopup::SelectSchemaPopup(QString defaultSchemaId, const std::vector<SelectSchemaItem>& schemas, QWidget* parent) :
	QDialog(parent)
{
	m_schemas.reserve(512);
	m_schemas.insert(m_schemas.begin(), schemas.begin(), schemas.end());

	setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

	setBackgroundRole(QPalette::Light);
	setAutoFillBackground(true);
	setSizeGripEnabled(true);

	// --
	//
	m_edit = new QLineEdit;
	m_edit->setPlaceholderText(tr("Filter"));
	m_edit->setClearButtonEnabled(true);

	// List
	//
	m_tableWidget = new SelectSchemaTable(m_schemas, defaultSchemaId, this);

	// --
	//
	setLayout(new QVBoxLayout);

	layout()->addWidget(m_edit);
	layout()->addWidget(m_tableWidget);

	fillList(defaultSchemaId);

	m_edit->setFocus();

	// --
	//
	connect(m_edit, &QLineEdit::textEdited, this, &SelectSchemaPopup::filterTextChanged);
	connect(m_tableWidget, &QAbstractItemView::activated, this, &SelectSchemaPopup::listCellClicked);
	connect(m_tableWidget, &QAbstractItemView::clicked, this, &SelectSchemaPopup::listCellClicked);

	return;
}

QString SelectSchemaPopup::selectedSchemaId() const
{
	return m_selectedSchemaId;
}

void SelectSchemaPopup::showEvent(QShowEvent* /*event*/)
{
	QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity");
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setDuration(200);

	animation->start(QAbstractAnimation::DeleteWhenStopped);
	return;
}

void SelectSchemaPopup::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return ||
		event->key() == Qt::Key_Enter)
	{
		if (m_tableWidget->selectionModel()->hasSelection() == true)
		{
			auto selectedCell = m_tableWidget->selectionModel()->selectedIndexes().front();
			listCellClicked(selectedCell);

			accept();
		}
		else
		{
			QDialog::keyPressEvent(event);
		}

		return;
	}

	if (event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Down)
	{
		m_tableWidget->setFocus();
	}
	else
	{
		QDialog::keyPressEvent(event);
	}

	return;
}

void SelectSchemaPopup::fillList(QString selectSchemaId)
{
	QString filterText = m_edit->text().trimmed();
	m_tableWidget->applyFilter(filterText, selectSchemaId);

	return;
}

void SelectSchemaPopup::filterTextChanged()
{
	fillList(QString());
}

void SelectSchemaPopup::listCellClicked(const QModelIndex& index)
{
	if (index.isValid() == true)
	{
		m_selectedSchemaId = m_tableWidget->model()->data(index, Qt::UserRole).toString();

		if (m_selectedSchemaId.isNull() == false)
		{
			close();
		}
	}

	return;
}

SelectSchemaModel::SelectSchemaModel(const std::vector<SelectSchemaItem>& schemas, QObject* parent) :
	QAbstractTableModel(parent),
	m_schemas(schemas)
{
	m_filteredItems.reserve(m_schemas.size());

	// Use applyFilter to fill m_filteredItems
	//

	return;
}

int SelectSchemaModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_filteredItems.size());
}

int SelectSchemaModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 1;
}

QVariant SelectSchemaModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();

	if (role == Qt::DisplayRole)
	{
		assert(static_cast<size_t>(row) < m_filteredItems.size());

		int index = m_filteredItems[row];
		assert(static_cast<size_t>(index) < m_schemas.size());

		const auto& schema = m_schemas[index];
		QString str = QString("%1\n%2").arg(schema.schemaId).arg(schema.caption);

		return QVariant(str);
	}

	if (role == Qt::UserRole)
	{
		assert(static_cast<size_t>(row) < m_filteredItems.size());

		int index = m_filteredItems[row];
		assert(static_cast<size_t>(index) < m_schemas.size());

		const auto& schema = m_schemas[index];

		return QVariant(schema.schemaId);
	}

	return QVariant();
}

int SelectSchemaModel::applyFilter(QString filterText, QString defaultSchemaId)
{
	beginResetModel();

	m_filteredItems.clear();

	int selectSchemaRow = -1;

	for (int i = 0; i < static_cast<int>(m_schemas.size()); i++)
	{
		const auto& s = m_schemas[i];

		if (filterText.isEmpty() == true ||
			s.schemaId.contains(filterText, Qt::CaseInsensitive) ||
			s.caption.contains(filterText, Qt::CaseInsensitive))
		{
			m_filteredItems.push_back(i);

			if (s.schemaId == defaultSchemaId)
			{
				selectSchemaRow = i;
			}
		}
	}

	endResetModel();

	if (m_filteredItems.size() == 1)		// If just only one item in the list, then select it
	{
		selectSchemaRow = 0;
	}

	return selectSchemaRow;
}


SelectSchemaTable::SelectSchemaTable(const std::vector<SelectSchemaItem>& schemas, QString defaultSchemaId, QWidget* parent) :
	QTableView(parent),
	m_schemas(schemas)
{
	setMouseTracking(true);

	setEditTriggers(QAbstractItemView::NoEditTriggers);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	verticalHeader()->setHidden(true);

	horizontalHeader()->setHidden(true);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);			// ResizeToContents is too slow
	verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize() * 2);

	setTabKeyNavigation(false);

	// Set the same color with and withoud focus
	//
	QPalette p = palette();

	p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
	p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));

	setPalette(p);

	// Model
	//
	m_model = new SelectSchemaModel(schemas, this);
	setModel(m_model);

	applyFilter(QString(), defaultSchemaId);

	// --
	//
	connect(this, &QAbstractItemView::entered, this, &SelectSchemaTable::mouseOverItem);

	return;
}

void SelectSchemaTable::applyFilter(QString filter, QString defaultSchemaId)
{
	int rowToSelect = m_model->applyFilter(filter, defaultSchemaId);

	if (rowToSelect != -1)
	{
		selectRow(rowToSelect);
		setCurrentIndex(model()->index(rowToSelect, 0));
	}

	return;
}

void SelectSchemaTable::mouseOverItem(const QModelIndex& index)
{
	if (index.isValid() == true)
	{
		selectRow(index.row());
		setCurrentIndex(index);
	}

	return;
}

void SelectSchemaTable::currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
	if (current.isValid() == true)
	{
		selectRow(current.row());
	}
}
