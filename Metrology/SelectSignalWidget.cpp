#include "SelectSignalWidget.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SelectSignalItem::SelectSignalItem()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

SelectSignalItem::SelectSignalItem(const SelectSignalItem& signal)
{
	*this = signal;
}

// -------------------------------------------------------------------------------------------------------------------

SelectSignalItem::SelectSignalItem(int index, int signalConnectionType, const MeasureSignal& measureSignal)
{
	clear();

	set(index, signalConnectionType, measureSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalItem::clear()
{
	m_index = -1;
	m_connectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;

	for(int ioType = 0; ioType < MEASURE_IO_SIGNAL_TYPE_COUNT; ioType++)
	{
		m_signalId[ioType].clear();
		m_caption[ioType].clear();
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool SelectSignalItem::isValid() const
{
	if (m_index == -1)
	{
		return false;
	}

	if (m_connectionType < 0 || m_connectionType > SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return false;
	}

	if (m_signalId[MEASURE_IO_SIGNAL_TYPE_INPUT].isEmpty() == true)
	{
		return false;
	}

	if (m_connectionType != SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		if (m_signalId[MEASURE_IO_SIGNAL_TYPE_OUTPUT].isEmpty() == true)
		{
			return false;
		}
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SelectSignalItem::set(int index, int signalConnectionType, const MeasureSignal& measureSignal)
{
	setIndex(index);
	setConnectionType(signalConnectionType);

	const MultiChannelSignal& inSignal = measureSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inSignal.isEmpty() == true)
	{
		return false;
	}

	if (inSignal.signalID().isEmpty() == true)
	{
		return false;
	}

	setSignalId(MEASURE_IO_SIGNAL_TYPE_INPUT,inSignal.signalID());
	setCaption(MEASURE_IO_SIGNAL_TYPE_INPUT, inSignal.caption());

	if (signalConnectionType != SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		const MultiChannelSignal& outSignal = measureSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
		if (outSignal.isEmpty() == true)
		{
			return false;
		}

		if (outSignal.signalID().isEmpty() == true)
		{
			return false;
		}

		setSignalId(MEASURE_IO_SIGNAL_TYPE_OUTPUT, outSignal.signalID());
		setCaption(MEASURE_IO_SIGNAL_TYPE_OUTPUT, outSignal.caption());
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString SelectSignalItem::signalId(int ioType) const
{
	if (ioType < 0 || ioType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	return m_signalId[ioType];
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalItem::setSignalId(int ioType, const QString& signalId)
{
	if (ioType < 0 || ioType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	m_signalId[ioType] = signalId;
}

// -------------------------------------------------------------------------------------------------------------------

QString SelectSignalItem::caption(int ioType) const
{
	if (ioType < 0 || ioType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	return m_caption[ioType];
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalItem::setCaption(int ioType, const QString& caption)
{
	if (ioType < 0 || ioType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	m_caption[ioType] = caption;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SelectSignalWidget::SelectSignalWidget(QWidget* parent) :
	QWidget(parent)
{
	m_signalList.reserve(32);

	// --
	//
	m_button = new QPushButton{this};
	m_button->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
	m_button->setStyleSheet("Text-align:left");

	// --
	//
	setLayout(new QHBoxLayout{this});
	layout()->setMargin(layout()->margin() / 4);

	layout()->addWidget(m_button);

	// --
	//
	connect(m_button, &QPushButton::clicked, this, &SelectSignalWidget::slot_buttonClicked);

	// It will make this button the size to 2 rows, it is important to keep toolbar always the same size, from the very start of app
	//
	m_button->setText(QString("\n"));

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalWidget::clear()
{
	m_signalList.clear();
	setCurrentSignalIndex(QString());
	return;
}

// -------------------------------------------------------------------------------------------------------------------

bool SelectSignalWidget::setCurrentSignalIndex(const QString& signalId)
{
	m_currentSignalIndex = -1;

	SelectSignalItem signal;

	for (const SelectSignalItem& s : m_signalList)
	{
		if (s.isValid() == false)
		{
			continue;
		}

		if (s.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT) == signalId)
		{
			signal = s;
			break;
		}
	}

	if (signal.isValid() == false)
	{
		// It will make this button the size to 2 rows, it is important to keep toolbar always the same size, from the very start of app
		//
		m_button->setText(QString("\n"));
		return false;
	}

	m_currentSignalIndex = signal.index();

	QString buttonTitle;

	if (signal.connectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		buttonTitle = QString(" %1\n %2").arg(signal.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT)).arg(signal.caption(MEASURE_IO_SIGNAL_TYPE_INPUT));
	}
	else
	{
		buttonTitle = " " + signal.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT);

		if (signal.caption(MEASURE_IO_SIGNAL_TYPE_INPUT).isEmpty() == false)
		{
			buttonTitle += " - " + signal.caption(MEASURE_IO_SIGNAL_TYPE_INPUT);
		}

		buttonTitle += "\n " + signal.signalId(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

		if (signal.caption(MEASURE_IO_SIGNAL_TYPE_OUTPUT).isEmpty() == false)
		{
			buttonTitle += " - " + signal.caption(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
		}
	}

	m_button->setText(buttonTitle);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int SelectSignalWidget::currentSignalIndex() const
{
	return m_currentSignalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

bool SelectSignalWidget::addSignal(const SelectSignalItem& signal)
{
	if (signal.isValid() == false)
	{
		return false;
	}

	m_signalList.push_back(signal);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int SelectSignalWidget::setSignalList(const std::vector<SelectSignalItem>& signalList, const QString& defaultSignalId)
{
	clear();

	m_signalList = signalList;

	if (signalList.size() == 0)
	{
		return 0;
	}

	if (setCurrentSignalIndex(defaultSignalId) == false)
	{
		return 0;
	}

	return m_currentSignalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalWidget::updateActiveOutputSignal(const MeasureSignal& activeSignal)
{
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	if (m_currentSignalIndex < 0 || m_currentSignalIndex >= m_signalList.size())
	{
		return;
	}

	SelectSignalItem& signal = m_signalList[m_currentSignalIndex];

	if (signal.isValid() == false)
	{
		return;
	}

	signal.set(signal.index(), signal.connectionType(), activeSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalWidget::slot_buttonClicked()
{
	SelectSignalPopup w(m_currentSignalIndex, m_signalList, this);

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

	if (w.selectedSignalIndex() != -1 &&
		w.selectedSignalIndex() != m_currentSignalIndex)
	{
		emit selectionChanged(w.selectedSignalIndex());
	}

	m_lastPopupHeight = w.height();
	m_lastPopupWidth = w.width();

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalWidget::activeSignalChanged(const MeasureSignal& activeSignal)
{
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	const MultiChannelSignal& signal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (signal.isEmpty() == true)
	{
		return;
	}

	if (signal.signalID().isEmpty() == true)
	{
		return;
	}

	setCurrentSignalIndex(signal.signalID());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SelectSignalPopup::SelectSignalPopup(int defaultSignalIndex, const std::vector<SelectSignalItem>& signalList, QWidget* parent) :
	QDialog(parent)
{
	m_signalList.reserve(512);
	m_signalList.insert(m_signalList.begin(), signalList.begin(), signalList.end());

	setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

	setBackgroundRole(QPalette::Light);
	setAutoFillBackground(true);
	setSizeGripEnabled(true);

	// --
	//
	m_edit = new QLineEdit{this};
	m_edit->setPlaceholderText(tr("Filter"));
	//m_edit->setClearButtonEnabled(true);

	// List
	//
	m_tableWidget = new SelectSignalTable{m_signalList, defaultSignalIndex, this};

	// --
	//
	setLayout(new QVBoxLayout{this});

	layout()->addWidget(m_edit);
	layout()->addWidget(m_tableWidget);

	fillList(defaultSignalIndex);

	m_edit->setFocus();

	// --
	//
	connect(m_edit, &QLineEdit::textEdited, this, &SelectSignalPopup::filterTextChanged);
	connect(m_tableWidget, &QAbstractItemView::activated, this, &SelectSignalPopup::listCellClicked);
	connect(m_tableWidget, &QAbstractItemView::clicked, this, &SelectSignalPopup::listCellClicked);

	return;
}

// -------------------------------------------------------------------------------------------------------------------

int SelectSignalPopup::selectedSignalIndex() const
{
	return m_selectedSignalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalPopup::showEvent(QShowEvent* /*event*/)
{
	QPropertyAnimation* animation = new QPropertyAnimation{this, "windowOpacity"};
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setDuration(200);

	animation->start(QAbstractAnimation::DeleteWhenStopped);
	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalPopup::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
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

	if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
	{
		m_tableWidget->setFocus();
	}
	else
	{
		QDialog::keyPressEvent(event);
	}

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalPopup::fillList(int selectSignalIndex)
{
	QString filterText = m_edit->text().trimmed();
	m_tableWidget->applyFilter(filterText, selectSignalIndex);

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalPopup::filterTextChanged()
{
	fillList(-1);
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalPopup::listCellClicked(const QModelIndex& index)
{
	if (index.isValid() == true)
	{
		m_selectedSignalIndex = m_tableWidget->model()->data(index, Qt::UserRole).toInt();

		if (m_selectedSignalIndex != -1)
		{
			close();
		}
	}

	return;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SelectSignalModel::SelectSignalModel(const std::vector<SelectSignalItem>& signalList, QObject* parent) :
	QAbstractTableModel(parent),
	m_signalList(signalList)
{
	m_filteredItems.reserve(m_signalList.size());

	// Use applyFilter to fill m_filteredItems
	//

	return;
}

// -------------------------------------------------------------------------------------------------------------------

int SelectSignalModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return static_cast<int>(m_filteredItems.size());
}

int SelectSignalModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 1;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SelectSignalModel::data(const QModelIndex& modelIndex, int role) const
{
	int row = modelIndex.row();

	if (role == Qt::DisplayRole)
	{
		Q_ASSERT(static_cast<size_t>(row) < m_filteredItems.size());

		int index = m_filteredItems[row];
		Q_ASSERT(static_cast<size_t>(index) < m_signalList.size());

		const auto& signal = m_signalList[index];

		QString str = QString("\n");

		if (signal.isValid() == true)
		{
			if (signal.connectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
			{
				str = QString(" %1\n %2").arg(signal.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT)).arg(signal.caption(MEASURE_IO_SIGNAL_TYPE_INPUT));
			}
			else
			{
				str = " " + signal.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT);

				if (signal.caption(MEASURE_IO_SIGNAL_TYPE_INPUT).isEmpty() == false)
				{
					str += " - " + signal.caption(MEASURE_IO_SIGNAL_TYPE_INPUT);
				}

				str += "\n " + signal.signalId(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

				if (signal.caption(MEASURE_IO_SIGNAL_TYPE_OUTPUT).isEmpty() == false)
				{
					str += " - " + signal.caption(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
				}
			}
		}

		return QVariant(str);
	}

	if (role == Qt::UserRole)
	{
		Q_ASSERT(static_cast<size_t>(row) < m_filteredItems.size());

		int index = m_filteredItems[row];
		Q_ASSERT(static_cast<size_t>(index) < m_signalList.size());

		const auto& signal = m_signalList[index];

		return QVariant(signal.index());
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

int SelectSignalModel::applyFilter(QString filterText, int defaultSignalIndex)
{
	beginResetModel();

		m_filteredItems.clear();

		int selectSignalRow = -1;

		for (int i = 0; i < static_cast<int>(m_signalList.size()); i++)
		{
			const auto& s = m_signalList[i];

			if (s.isValid() == true)
			{
				if (s.connectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
				{
					if(	filterText.isEmpty() == true ||
							s.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT).contains(filterText, Qt::CaseInsensitive) ||
							s.caption(MEASURE_IO_SIGNAL_TYPE_INPUT).contains(filterText, Qt::CaseInsensitive))
					{
						m_filteredItems.push_back(i);

						if (s.index() == defaultSignalIndex)
						{
							selectSignalRow = i;
						}
					}
				}
				else
				{
					if(	filterText.isEmpty() == true ||
							s.signalId(MEASURE_IO_SIGNAL_TYPE_INPUT).contains(filterText, Qt::CaseInsensitive) ||
							s.caption(MEASURE_IO_SIGNAL_TYPE_INPUT).contains(filterText, Qt::CaseInsensitive) ||
							s.signalId(MEASURE_IO_SIGNAL_TYPE_OUTPUT).contains(filterText, Qt::CaseInsensitive) ||
							s.caption(MEASURE_IO_SIGNAL_TYPE_OUTPUT).contains(filterText, Qt::CaseInsensitive) )
					{
						m_filteredItems.push_back(i);

						if (s.index() == defaultSignalIndex)
						{
							selectSignalRow = i;
						}
					}
				}

			}
		}

	endResetModel();

	if (m_filteredItems.size() == 1)		// If just only one item in the list, then select it
	{
		selectSignalRow = 0;
	}

	return selectSignalRow;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SelectSignalTable::SelectSignalTable(const std::vector<SelectSignalItem>& signalList, int defaultSignalIndex, QWidget* parent) :
	QTableView(parent),
	m_signalList(signalList)
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
	m_model = new SelectSignalModel{signalList, this};
	setModel(m_model);

	applyFilter(QString(), defaultSignalIndex);

	// --
	//
	connect(this, &QAbstractItemView::entered, this, &SelectSignalTable::mouseOverItem);

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalTable::applyFilter(QString filter, int defaultSignalIndex)
{
	int rowToSelect = m_model->applyFilter(filter, defaultSignalIndex);

	if (rowToSelect != -1)
	{
		selectRow(rowToSelect);
		setCurrentIndex(model()->index(rowToSelect, 0));
	}

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalTable::mouseOverItem(const QModelIndex& index)
{
	if (index.isValid() == true)
	{
		selectRow(index.row());
		setCurrentIndex(index);
	}

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SelectSignalTable::currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
	if (current.isValid() == true)
	{
		selectRow(current.row());
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
