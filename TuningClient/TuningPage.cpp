#include "Settings.h"
#include "MainWindow.h"
#include "TuningPage.h"
#include <QKeyEvent>
#include "../VFrame30/DrawParam.h"

using namespace std;

//
// TuningItemModelMain
//


TuningModelClient::TuningModelClient(TuningSignalManager* tuningSignalManager, int tuningPageIndex, QWidget* parent):
	TuningModel(parent),
	m_tuningSignalManager(tuningSignalManager)
{

	assert(m_tuningSignalManager);

	TuningPageSettings* pageSettings = theSettings.tuningPageSettings(tuningPageIndex);
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (pageSettings->m_columnCount == 0)
	{
		addColumn(Columns::CustomAppSignalID);
		addColumn(Columns::EquipmentID);
		addColumn(Columns::Caption);
		addColumn(Columns::Units);
		addColumn(Columns::Type);

		addColumn(Columns::Value);
		addColumn(Columns::LowLimit);
		addColumn(Columns::HighLimit);
		addColumn(Columns::Default);
		//addColumn(Columns::Valid);
		addColumn(Columns::Underflow);
		addColumn(Columns::Overflow);
	}
	else
	{
		m_columnsIndexes  = pageSettings->m_columnsIndexes;
		removeColumn(Columns::Valid);
		removeColumn(Columns::Underflow);
		removeColumn(Columns::Overflow);
	}
}

void TuningModelClient::setValue(const std::vector<int>& selectedRows)
{
	bool first = true;
	bool analog = false;
	float value = 0.0;
	float defaultValue = 0.0;
	bool sameValue = true;
	int precision = 0;
	float lowLimit = 0;
	float highLimit = 0;

	for (int i : selectedRows)
	{
		const TuningModelRecord& o = m_items[i];

		if (o.state.valid() == false)
		{
			return;
		}

		if (o.param.isAnalog() == true)
		{
			if (o.param.precision() > precision)
			{
				precision = o.param.precision();
			}
		}

		if (first == true)
		{
			analog = o.param.isAnalog();
			value = o.state.value();
			defaultValue = o.param.tuningDefaultValue();
			lowLimit = o.param.lowEngineeringUnits();
			highLimit = o.param.highEngineeringUnits();
			first = false;
		}
		else
		{
			if (analog != o.param.isAnalog())
			{
				QMessageBox::warning(m_parent, tr("Set Value"), tr("Please select one type of objects: analog or discrete."));
				return;
			}

			if (analog == true)
			{
				if (lowLimit != o.param.lowEngineeringUnits() || highLimit != o.param.highEngineeringUnits())
				{
					QMessageBox::warning(m_parent, tr("Set Value"), tr("Selected objects have different input range."));
					return;
				}
			}

			if (defaultValue != o.param.tuningDefaultValue())
			{
				QMessageBox::warning(m_parent, tr("Set Value"), tr("Selected objects have different default values."));
				return;
			}

			if (value != o.state.value())
			{
				sameValue = false;
			}
		}
	}

	DialogInputTuningValue d(analog, value, defaultValue, sameValue, lowLimit, highLimit, precision, m_parent);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	float newValue = d.value();

	for (int i : selectedRows)
	{
		TuningModelRecord& o = m_items[i];
		o.state.onEditValue(newValue);
	}
}

void TuningModelClient::invertValue(const std::vector<int>& selectedRows)
{
	for (int i : selectedRows)
	{
		TuningModelRecord& o = m_items[i];

		if (o.state.valid() == false)
		{
			return;
		}

		if (o.param.isAnalog() == false)
		{
			if (o.state.editValue() == 0)
			{
				o.state.onEditValue(1);
			}
			else
			{
				o.state.onEditValue(0);
			}
		}
	}
}


void TuningModelClient::updateStates()
{
	static int counter = 0;

	if (counter++ >= 2)
	{
		counter = 0;
		m_blink = !m_blink;
	}

	m_tuningSignalManager->updateStates(m_items);

	return;
}

QBrush TuningModelClient::backColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_items.size())
	{
		assert(false);
		return QBrush();
	}

	if (displayIndex == static_cast<int>(Columns::Value))
	{
		const TuningModelRecord& o = m_items[row];

		if (m_blink == true && o.state.userModified() == true)
		{
			QColor color = QColor(Qt::yellow);
			return QBrush(color);
		}

		if (o.state.valid() == false || o.limitsUnbalance() == true)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Valid))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.valid() == false)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::LowLimit) || displayIndex == static_cast<int>(Columns::HighLimit))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.limitsUnbalance() == true)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Underflow))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.underflow() == true)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Overflow))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.overflow() == true)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Default))
	{
		QColor color = QColor(Qt::gray);
		return QBrush(color);
	}

	return QBrush();
}

QBrush TuningModelClient::foregroundColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_items.size())
	{
		assert(false);
		return QBrush();
	}

	if (displayIndex == static_cast<int>(Columns::Valid))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.valid() == false)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Value))
	{
		const TuningModelRecord& o = m_items[row];

		if (m_blink == true && o.state.userModified() == true)
		{
			QColor color = QColor(Qt::black);
			return QBrush(color);
		}

		if (o.state.valid() == false || o.limitsUnbalance() == true)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::LowLimit) || displayIndex == static_cast<int>(Columns::HighLimit))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.limitsUnbalance() == true)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Underflow))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.underflow() == true)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns:: Overflow))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.overflow() == true)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Default))
	{
		//int displayIndex = m_columnsIndexes[col];
		QColor color = QColor(Qt::white);
		return QBrush(color);
	}

	return QBrush();

}

Qt::ItemFlags TuningModelClient::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = TuningModel::flags(index);

	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_items.size())
	{
		assert(false);
		return f;
	}


	if (displayIndex == static_cast<int>(Columns::Value))
	{
		const TuningModelRecord& o = m_items[row];

		if (o.state.valid() == true)
		{
			if (o.param.isAnalog() == false)
			{
				f |= Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
			}
			else
			{
				f |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
			}
		}

	}

	return f;
}

QVariant TuningModelClient::data(const QModelIndex& index, int role) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_items.size())
	{
		assert(false);
		return QVariant();
	}

	const TuningModelRecord& o = m_items[row];

	if (role == Qt::CheckStateRole && displayIndex == static_cast<int>(Columns::Value) && o.param.isAnalog() == false && o.state.valid() == true)
	{
		return (o.state.editValue() == 0 ? Qt::Unchecked : Qt::Checked);
	}

	return TuningModel::data(index, role);
}

bool TuningModelClient::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
	{
		return false;
	}

	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_items.size())
	{
		assert(false);
		return false;
	}

	if (role == Qt::EditRole && displayIndex == static_cast<int>(TuningModel::Columns::Value))
	{
		TuningModelRecord& o = m_items[row];

		bool ok = false;
		float v = value.toFloat(&ok);
		if (ok == false)
		{
			return false;
		}

		o.state.onEditValue(v);
		return true;
	}

	if (role == Qt::CheckStateRole && displayIndex == static_cast<int>(TuningModel::Columns::Value))
	{
		TuningModelRecord& o = m_items[row];

		if ((Qt::CheckState)value.toInt() == Qt::Checked)
		{
			o.state.onEditValue(1.0);
			return true;
		}
		else
		{
			o.state.onEditValue(0.0);
			return true;
		}
	}
	return false;
}

void TuningModelClient::slot_setAll()
{
	QMenu menu(m_parent);

	// Set All To On
	QAction* actionAllToOn = new QAction(tr("Set All Discretes To On"), &menu);

	auto fAllToOn = [this]() -> void
	{
			for (TuningModelRecord& o : m_items)
	{
			if (o.state.valid() == false)
	{
			continue;
}

			if (o.param.isAnalog() == false)
	{
			o.state.onEditValue(1);
}
}
};
	connect(actionAllToOn, &QAction::triggered, this, fAllToOn);

	// Set All To Onff
	QAction* actionAllToOff = new QAction(tr("Set All Discretes To Off"), &menu);

	auto fAllToOff = [this]() -> void
	{
			for (TuningModelRecord& o : m_items)
	{
			if (o.state.valid() == false)
	{
			continue;
}

			if (o.param.isAnalog() == false)
	{
			o.state.onEditValue(0);
}
}
};

	connect(actionAllToOff, &QAction::triggered, this, fAllToOff);

	// Set All To Defaults
	QAction* actionAllToDefault = new QAction(tr("Set All To Defaults"), &menu);

	auto fAllToDefault = [this]() -> void
	{
			for (TuningModelRecord& o : m_items)
	{
			if (o.state.valid() == false)
	{
			continue;
}

			//float scalePercent = std::fabs(o.param.lowEngineeringUnits() - o.param.highEngineeringUnits()) / 100.0;

			double epsilon = std::numeric_limits<double>::epsilon();

			if (std::fabs(o.param.tuningDefaultValue() - o.state.editValue()) > epsilon)
	{
			o.state.onEditValue(o.param.tuningDefaultValue());
}
}
};

	connect(actionAllToDefault, &QAction::triggered, this, fAllToDefault);

	// Run the menu

	menu.addAction(actionAllToOn);
	menu.addAction(actionAllToOff);
	menu.addSeparator();
	menu.addAction(actionAllToDefault);

	menu.exec(QCursor::pos());
}


void TuningModelClient::slot_undo()
{
	for (TuningModelRecord& o : m_items)
	{
		o.state.onEditValue(o.state.value());
	}
}

void TuningModelClient::slot_Write()
{
	if (theUserManager.requestPassword(m_parent, false) == false)
	{
		return;
	}

	QString str = tr("New values will be written:") + QString("\r\n\r\n");
	QString strValue;

	bool modifiedFound = false;
	int modifiedCount = 0;

	for (TuningModelRecord& o : m_items)
	{
		if (o.state.userModified() == false)
		{
			continue;
		}

		modifiedFound = true;
		modifiedCount++;
	}

	if (modifiedFound == false)
	{
		return;
	}

	int listCount = 0;

	std::vector<std::pair<Hash, float>> writeData;

	for (TuningModelRecord& o : m_items)
	{
		if (o.state.userModified() == false)
		{
			continue;
		}

		if (listCount >= 10)
		{
			str += tr("and %1 more values.").arg(modifiedCount - listCount);
			break;
		}

		if (o.param.isAnalog() == true)
		{
			strValue = QString::number(o.state.editValue(), 'f', o.param.precision());
		}
		else
		{
			strValue = o.state.editValue() == 0 ? tr("0") : tr("1");
		}

		str += tr("%1 (%2) = %3\r\n").arg(o.param.appSignalId()).arg(o.param.caption()).arg(strValue);

		listCount++;
	}

	str += QString("\r\n") + tr("Are you sure you want to continue?");

	if (QMessageBox::warning(m_parent, tr("Write Changes"),
							 str,
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	for (TuningModelRecord& o : m_items)
	{
		if (o.state.userModified() == false)
		{
			continue;
		}

		o.state.clearUserModified();

		writeData.push_back(std::pair<Hash, float>(o.param.hash(), o.state.editValue()));
	}

	m_tuningSignalManager->writeTuningSignals(writeData);
}

void TuningModelClient::slot_Apply()
{

}


//
// TuningPage
//
FilterButton::FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filter = filter;
	m_caption = caption;

	setCheckable(true);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<TuningFilter> FilterButton::filter()
{
	return m_filter;
}

void FilterButton::slot_toggled(bool checked)
{
	if (checked == true)
	{
		emit filterButtonClicked(m_filter);
	}

}

//
// TuningPage
//

bool TuningTableView::editorActive()
{
	return m_editorActive;
}

bool TuningTableView::edit(const QModelIndex&  index, EditTrigger trigger, QEvent* event)
{

	if (trigger == QAbstractItemView::EditKeyPressed)
	{
		TuningModel* m_model = dynamic_cast<TuningModel*>(model());
		if (m_model == nullptr)
		{
			assert(m_model);
			return false;
		}

		int row = index.row();

		if (row >= 0)
		{
			AppSignalParam* param = m_model->param(row);
			if (param == nullptr)
			{
				assert(param);
				return false;
			}

			if (param->isAnalog() == true)
			{
				m_editorActive = true;
			}
		}
	}

	return QTableView::edit(index, trigger, event);
}

void TuningTableView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{

	//qDebug() << "closeEditor";

	m_editorActive = false;

	QTableView::closeEditor(editor, hint);
}

//
// TuningPage
//


TuningPage::TuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> tabFilter, TuningSignalManager* tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects, QWidget* parent) :
	QWidget(parent),
	m_objects(objects),
	m_tuningSignalManager(tuningSignalManager),
	m_filterStorage(filterStorage),
	m_tabFilter(tabFilter),
	m_tuningPageIndex(tuningPageIndex)
{

	assert(m_tuningSignalManager);
	assert(m_filterStorage);
	assert(m_objects);

	std::vector<FilterButton*> buttons;

	// Top buttons
	//
	int count = m_filterStorage->m_root->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> f = m_filterStorage->m_root->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isButton() == false)
		{
			continue;
		}

		FilterButton* button = new FilterButton(f, f->caption());
		buttons.push_back(button);

		connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);
	}

	// Child buttons
	//
	if (tabFilter != nullptr)
	{
		for (int i = 0; i < tabFilter->childFiltersCount(); i++)
		{
			std::shared_ptr<TuningFilter> f = tabFilter->childFilter(i);
			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			if (f->isButton() == false)
			{
				continue;
			}

			FilterButton* button = new FilterButton(f, f->caption());
			buttons.push_back(button);

			connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);

		}
	}

	if (buttons.empty() == false)
	{
		m_filterButtonGroup = new QButtonGroup(this);

		m_filterButtonGroup->setExclusive(true);

		m_buttonsLayout = new QHBoxLayout();

		for (auto b: buttons)
		{
			m_filterButtonGroup->addButton(b);
			m_buttonsLayout->addWidget(b);
		}

		m_buttonsLayout->addStretch();

		// Set the first button checked
		//
		buttons[0]->blockSignals(true);
		buttons[0]->setChecked(true);
		m_buttonFilter = buttons[0]->filter();
		buttons[0]->blockSignals(false);

	}

	// Object List
	//
	m_objectList = new TuningTableView();

	QFont f = m_objectList->font();

	// Models and data
	//
	m_model = new TuningModelClient(m_tuningSignalManager, m_tuningPageIndex, this);
	m_model->setFont(f.family(), f.pointSize(), false);
	m_model->setImportantFont(f.family(), f.pointSize(), true);

	// Filter controls
	//
	m_filterTypeCombo = new QComboBox();
	m_filterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterType::All));
	m_filterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterType::AppSignalID));
	m_filterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterType::CustomAppSignalID));
	m_filterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterType::EquipmentID));
	m_filterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterType::Caption));

	connect(m_filterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterTypeIndexChanged(int)));

	m_filterTypeCombo->setCurrentIndex(0);

	m_filterEdit = new QLineEdit();
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &TuningPage::slot_ApplyFilter);

	m_filterButton = new QPushButton(tr("Filter"));
	connect(m_filterButton, &QPushButton::clicked, this, &TuningPage::slot_ApplyFilter);

	// Button controls
	//

	m_setValueButton = new QPushButton(tr("Set Value"));
	connect(m_setValueButton, &QPushButton::clicked, this, &TuningPage::slot_setValue);

	m_setAllButton = new QPushButton(tr("Set All"));
	connect(m_setAllButton, &QPushButton::clicked, m_model, &TuningModelClient::slot_setAll);

	m_writeButton = new QPushButton(tr("Write"));
	connect(m_writeButton, &QPushButton::clicked, m_model, &TuningModelClient::slot_Write);

	m_undoButton = new QPushButton(tr("Undo"));
	connect(m_undoButton, &QPushButton::clicked, m_model, &TuningModelClient::slot_undo);


	m_bottomLayout = new QHBoxLayout();

	m_bottomLayout->addWidget(m_filterTypeCombo);
	m_bottomLayout->addWidget(m_filterEdit);
	m_bottomLayout->addWidget(m_filterButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_setValueButton);
	m_bottomLayout->addWidget(m_setAllButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_writeButton);
	m_bottomLayout->addWidget(m_undoButton);

	if (theConfigSettings.autoApply == false)
	{
		m_applyButton = new QPushButton(tr("Apply"));
		connect(m_applyButton, &QPushButton::clicked, m_model, &TuningModelClient::slot_Apply);
		m_bottomLayout->addWidget(m_applyButton);
	}

	m_mainLayout = new QVBoxLayout(this);

	if (m_buttonsLayout != nullptr)
	{
		m_mainLayout->addLayout(m_buttonsLayout);
	}

	m_mainLayout->addWidget(m_objectList);
	m_mainLayout->addLayout(m_bottomLayout);


	m_objectList->setModel(m_model);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_objectList->setSortingEnabled(true);
	m_objectList->setEditTriggers(QAbstractItemView::EditKeyPressed);

	connect(m_objectList->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &TuningPage::sortIndicatorChanged);

	connect(m_objectList, &QTableView::doubleClicked, this, &TuningPage::slot_tableDoubleClicked);

	m_objectList->installEventFilter(this);


	fillObjectsList();

	TuningPageSettings* pageSettings = theSettings.tuningPageSettings(tuningPageIndex);
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (pageSettings->m_columnCount == 0)
	{
		m_objectList->resizeColumnsToContents();
	}
	else
	{
		for (int i = 0; i < pageSettings->m_columnCount; i++)
		{
			int width = pageSettings->m_columnsWidth[i];
			if (width < 20)
			{
				width = 20;
			}

			m_objectList->setColumnWidth(i, width);
		}
	}

	m_updateStateTimerId = startTimer(250);

}

TuningPage::~TuningPage()
{
	TuningPageSettings* pageSettings = theSettings.tuningPageSettings(m_tuningPageIndex);
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (m_model != nullptr && m_objectList != nullptr)
	{
		pageSettings->m_columnsIndexes = m_model->columnsIndexes();

		pageSettings->m_columnCount = static_cast<int>(pageSettings->m_columnsIndexes.size());

		pageSettings->m_columnsWidth.resize(pageSettings->m_columnCount);

		for (int i = 0; i < pageSettings->m_columnCount; i++)
		{
			pageSettings->m_columnsWidth[i] = m_objectList->columnWidth(i);
		}
	}
}

void TuningPage::fillObjectsList()
{
	std::vector<TuningModelRecord> filteredObjects;

	QString filter = m_filterEdit->text();

	FilterType filterType = FilterType::All;
	QVariant data = m_filterTypeCombo->currentData();
	if (data.isValid() == true)
	{
		filterType = static_cast<FilterType>(data.toInt());
	}

	for (int i = 0; i < m_objects->signalsCount(); i++)
	{
		const AppSignalParam* signal = m_objects->signalPtrByIndex(i);
		if (signal == nullptr)
		{
			assert(signal);
			continue;
		}

		bool modifyDefaultValue = false;
		float modifiedDefaultValue = 0;

		// Tree Filter
		//

		// If currently selected filter is root - all signals are displayed
		//

		if (m_treeFilter != nullptr && m_treeFilter->isRoot() == false)
		{
			bool result = true;

			TuningFilter* treeFilter = m_treeFilter.get();

			// If currently selected filter is empty - no signals are displayed (a "Folder" filter)

			if (treeFilter->isEmpty() == true)
			{
				continue;
			}

			// Otherwise, check parent filters. Values are checked ONLY for selected filter, not child filters!!!

			bool checkValues = true;

			while (treeFilter != nullptr)
			{
				if (treeFilter->match(*signal, checkValues) == false)
				{
					result = false;
					break;
				}

				checkValues = false;

				treeFilter = treeFilter->parentFilter();
			}
			if (result == false)
			{
				continue;
			}

			// Modify the default value from selected filter
			//

			TuningFilterValue filterValue;

			bool hasValue = m_treeFilter->value(signal->hash(), filterValue);

			if (hasValue == true)
			{
				modifyDefaultValue = true;
				modifiedDefaultValue = filterValue.value();
			}
		}

		// Tab Filter
		//

		if (m_tabFilter != nullptr)
		{
			if (m_tabFilter->match(*signal, false) == false)
			{
				continue;
			}
		}

		// Button Filter
		//

		if (m_buttonFilter != nullptr)
		{
			if (m_buttonFilter->match(*signal, false) == false)
			{
				continue;
			}
		}

		// Text filter
		//

		if (filter.length() != 0)
		{
			bool filterMatch = false;

			switch (filterType)
			{
			case FilterType::All:
				if (signal->appSignalId().contains(filter, Qt::CaseInsensitive) == true
						|| signal->customSignalId().contains(filter, Qt::CaseInsensitive) == true
						|| signal->equipmentId().contains(filter, Qt::CaseInsensitive) == true
						|| signal->caption().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::AppSignalID:
				if (signal->appSignalId().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::CustomAppSignalID:
				if (signal->customSignalId().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::EquipmentID:
				if (signal->equipmentId().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::Caption:
				if (signal->caption().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			}

			if (filterMatch == false)
			{
				continue;
			}
		}


		TuningModelRecord item;
		item.param =* signal;

		filteredObjects.push_back(item);

		if (modifyDefaultValue == true)
		{
			filteredObjects[filteredObjects.size() - 1].param.setTuningDefaultValue(modifiedDefaultValue);
		}
	}

	m_tuningSignalManager->updateStates(filteredObjects);

	m_model->setSignals(filteredObjects);

	m_objectList->sortByColumn(m_sortColumn, m_sortOrder);
}

QColor TuningPage::backColor()
{
	if (m_tabFilter != nullptr)
	{
		return m_tabFilter->backColor();
	}

	return QColor();

}

QColor TuningPage::textColor()
{
	if (m_tabFilter != nullptr)
	{
		return m_tabFilter->textColor();
	}

	return QColor();

}


void TuningPage::slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	m_buttonFilter = filter;

	fillObjectsList();

}

void TuningPage::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_model->sort(column, order);
}

void TuningPage::slot_setValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == false)
	{
		m_model->setValue(selectedRows);
	}
}

void TuningPage::invertValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == false)
	{
		m_model->invertValue(selectedRows);
	}
}

void TuningPage::slot_tableDoubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	slot_setValue();
}

void TuningPage::slot_FilterTypeIndexChanged(int index)
{
	Q_UNUSED(index);
	fillObjectsList();
}

void TuningPage::slot_ApplyFilter()
{
	fillObjectsList();
}

void TuningPage::slot_filterTreeChanged(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		//qDebug() << "Filter tree removed.";
	}
	else
	{
		//qDebug() << "Filter tree clicked: " << filter->caption();
	}

	m_treeFilter = filter;

	fillObjectsList();
}

void TuningPage::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId && m_model->rowCount() > 0 && isVisible() == true)
	{
		// Update only visible dynamic items
		//
		int from = m_objectList->rowAt(0);
		int to = m_objectList->rowAt(m_objectList->height() - m_objectList->horizontalHeader()->height());

		if (from == -1)
		{
			from = 0;
		}

		if (to == -1)
		{
			to = m_model->rowCount() - 1;
		}

		// Update signal states
		//
		m_model->updateStates();

		// Redraw visible table items
		//
		for (int row = from; row <= to; row++)
		{
			TuningSignalState* state = m_model->state(row);

			if (state == nullptr)
			{
				assert(state);
				continue;
			}

			if (state->needRedraw() == true || state->userModified() == true)
			{
				for (int col = 0; col < m_model->columnCount(); col++)
				{
					int displayIndex = m_model->columnIndex(col);

					if (displayIndex >= static_cast<int>(TuningModel::Columns::Value))
					{
						//QString str = QString("%1:%2").arg(row).arg(col);
						//qDebug() << str;

						m_objectList->update(m_model->index(row, col));
					}
				}
			}
		}
	}
}

bool TuningPage::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_objectList && event->type()==QEvent::KeyPress)
	{
		QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
		if(pKeyEvent->key() == Qt::Key_Return)
		{
			if (m_objectList->editorActive() == false)
			{
				m_model->slot_Write();
				return true;
			}
			return true;
		}

		if(pKeyEvent->key() == Qt::Key_Space)
		{
			invertValue();
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}
